/******************************************************************************
 *
 * Copyright (C) u-blox AG
 * u-blox AG, Thalwil, Switzerland
 *
 * All rights reserved.
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose without fee is hereby granted, provided that this entire notice is
 * included in all copies of any software which is or includes a copy or
 * modification of this software and in all copies of the supporting
 * documentation for such software.
 *
 * THIS SOFTWARE IS BEING PROVIDED "AS IS", WITHOUT ANY EXPRESS OR IMPLIED
 * WARRANTY. IN PARTICULAR, NEITHER THE AUTHOR NOR U-BLOX MAKES ANY
 * REPRESENTATION OR WARRANTY OF ANY KIND CONCERNING THE MERCHANTABILITY OF
 * THIS SOFTWARE OR ITS FITNESS FOR ANY PARTICULAR PURPOSE.
 *
 ******************************************************************************
 *
 * Project: libParser
 * Purpose: Library providing functions to parse u-blox GNSS receiver messages.
 *
 ******************************************************************************
 * $Id: protocolubx.cpp 86496 2014-10-24 17:37:48Z fabio.robbiani $
 * $HeadURL: http://svn.u-blox.ch/GPS/SOFTWARE/PRODUCTS/libParser/tags/libParser_v1.03/protocolubx.cpp $
 *****************************************************************************/

#include <new>

#include "gpsconst.h"
#include "protocolubx.h"
#include "parserbuffer.h"

#ifdef SUPL_ENABLED
#include "ubx_log.h"
#endif

int CProtocolUBX::Parse(const unsigned char* pBuffer, int iSize)
{
	return ParseUbx(pBuffer, iSize);
}

int CProtocolUBX::ParseUbx(const unsigned char* pBuffer, int iSize)
{
	if (iSize == 0)
		return CParserBuffer::WAIT;
	if (pBuffer[0] != UBX_CHAR_SYNC0)
		return CParserBuffer::NOT_FOUND;
	if (iSize == 1)
		return CParserBuffer::WAIT;
	if (pBuffer[1] != UBX_CHAR_SYNC1)
		return CParserBuffer::NOT_FOUND;
	if (iSize < 6)
		return CParserBuffer::WAIT;
	U2 iLength = (U2)(  ((U2)pBuffer[4]) +
						(((U2)pBuffer[5]) << 8));
	// filter out all large messages (with the exception of the tunneling class messages)
	if ((iLength > UBX_MAX_SIZE) &&
		(pBuffer[2] != 0x08/*tunneling class*/))
		return CParserBuffer::NOT_FOUND;
	if (iSize < iLength + 6)
		return CParserBuffer::WAIT;
	// calculate the cksum
	U1 ckA = 0;
	U1 ckB = 0;
	for (int i = 2; i < iLength + 6; i++)
	{
		ckA += pBuffer[i];
		ckB += ckA;
	}
	// check the cksum
	if (iSize < iLength + UBX_FRM_SIZE-1)
		return CParserBuffer::WAIT;
	if (pBuffer[iLength+6] != ckA)
		return CParserBuffer::NOT_FOUND;
	if (iSize < iLength + UBX_FRM_SIZE)
		return CParserBuffer::WAIT;
	if (pBuffer[iLength+7] != ckB)
		return CParserBuffer::NOT_FOUND;
	return iLength + UBX_FRM_SIZE;
}

void CProtocolUBX::Process(unsigned char* pBuffer, int iSize, CDatabase* pDatabase)
{
	// enable next line if you like noise
	if (pBuffer[2] == 0x01/*NAV*/ && pBuffer[3] == 0x03/*STATUS*/)
	{
		pDatabase->Process(CDatabase::MSG_UBX_NAV_STATUS, pBuffer, iSize, &ProcessNavStatus);
	}
#ifdef SUPL_ENABLED
	else if (pBuffer[2] == 0x02/*RXM*/ && pBuffer[3] == 0x12/*MEAS*/)
	{
		pDatabase->Process(CDatabase::MSG_UBX_RXM_MEAS, pBuffer, iSize, &ProcessRxmMeas);
	}
#endif	
}

unsigned int CProtocolUBX::NewMsg(U1 classId, U1 msgId, const void* pPayload, unsigned int iPayloadSize, unsigned char **ppMsg)
{
    unsigned int nRetVal = iPayloadSize + UBX_FRM_SIZE;

    if ( ppMsg )
    {
        U1* pBuffer = new(std::nothrow) U1[nRetVal];

	    if (pBuffer)
	    {
           *ppMsg = pBuffer;

		    pBuffer[0] = UBX_CHAR_SYNC0;
		    pBuffer[1] = UBX_CHAR_SYNC1;
		    pBuffer[2] = classId;
		    pBuffer[3] = msgId;
		    pBuffer[4] = (U1) iPayloadSize;
		    pBuffer[5] = (U1)(iPayloadSize >> 8);
		    memcpy(&pBuffer[6], pPayload, iPayloadSize);

		    // calculate the cksum
		    U1 ckA = 0;
		    U1 ckB = 0;
		    for (unsigned int i = 2; i < iPayloadSize + 6; i++)
		    {
			    ckA += pBuffer[i];
			    ckB += ckA;
		    }
		    pBuffer[6+iPayloadSize] = ckA;
		    pBuffer[7+iPayloadSize] = ckB;
	    }
        else
        {
            nRetVal = 0;
            *ppMsg = NULL;
        }
    }
    else
    {
        nRetVal = 0;
    }

	return nRetVal;
}

void CProtocolUBX::ProcessNavStatus(unsigned char* pBuffer, int iSize, CDatabase* pDatabase)
{
    // NAV-STATUS message structure
	#pragma pack (push, 1)
	typedef struct {
		U4 tow;
        U1 gpsFix;
        U1 flags;
        U1 fixStat;
        U1 flags2;
        U4 ttff;
        U4 msss;
	} STRUCT;
	#pragma pack (pop)

	if (iSize == (int)(sizeof(STRUCT) + UBX_FRM_SIZE))
	{
		STRUCT s, *p=&s;
		memcpy(p, &pBuffer[6], sizeof(s));
		if(p->ttff) // A TTFF of 0ms implies that no TTFF exists yet -> no fix
		{
			pDatabase->Set(CDatabase::DATA_UBX_TTFF, p->ttff);
		}
	}
}

#ifdef SUPL_ENABLED	
void CProtocolUBX::ProcessRxmMeas(unsigned char* pBuffer, int iSize, CDatabase* pDatabase)
{
	if (iSize < (int)(sizeof(GPS_UBX_RXM_MEAS_t) + UBX_FRM_SIZE))
	{
		// No enough data received
		return;
	}
	
	pBuffer += 6;
	GPS_UBX_RXM_MEAS_t rmxMeasHeader;
	memcpy(&rmxMeasHeader, pBuffer, sizeof(rmxMeasHeader));
	pBuffer += sizeof(rmxMeasHeader);
		
	I4 svCount = rmxMeasHeader.info & 0xFF;
	I4 dopCenter = ((I4)rmxMeasHeader.info) >> 8;
	
	if (iSize < (int) (((int)sizeof(GPS_UBX_RXM_MEAS_t) + UBX_FRM_SIZE + (svCount * (int)sizeof(GPS_UBX_RXM_MEAS_SVID_t)))))
	{
		// Not enough data received
		return;
	}

	pDatabase->Set(CDatabase::DATA_UBX_GNSS_TOW, rmxMeasHeader.gnssTow);
	pDatabase->Set(CDatabase::DATA_UBX_GNSS_DOP_CENTER, dopCenter);
	
	I4 gpsSvCount = 0;
	GPS_UBX_RXM_MEAS_SVID_t svData;
	for(I4 i = 0; i < svCount; i++)
	{
		memcpy(&svData, pBuffer, sizeof(GPS_UBX_RXM_MEAS_SVID_t));
		pBuffer += sizeof(GPS_UBX_RXM_MEAS_SVID_t);

		if ((svData.svid > 0) && (svData.svid <= 32))
		{
			pDatabase->Set(DATA_UBX_SATELLITES_IN_MEAS_(gpsSvCount), svData.svid);
			pDatabase->Set(DATA_UBX_SATELLITES_IN_MEAS_CNO_(gpsSvCount), svData.cno);
			
			R8 prRms = svData.prRms * 0.5;
			pDatabase->Set(DATA_UBX_SATELLITES_IN_MEAS_PRRMS_(gpsSvCount), prRms);
			pDatabase->Set(DATA_UBX_SATELLITES_IN_MEAS_MULTIPATH_IND_(gpsSvCount), svData.mpInd);
			pDatabase->Set(DATA_UBX_SATELLITES_IN_MEAS_REL_CODEPHASE_(gpsSvCount), svData.redSigtow);
			pDatabase->Set(DATA_UBX_SATELLITES_IN_MEAS_DOPPLER_(gpsSvCount), svData.doppler);
			gpsSvCount++;
			
//			LOGV("%s: satId %3d - cno %2d - doppler %8.2f chips %10.5f",
//				 __FUNCTION__, svData.svid, svData.cno,
//				 (double) svData.doppler / (1<<12),
//				 ((double)(svData.redSigtow & 0x1FFFFF) * 1023.0) / 0x200000);
		}
	}
	
	pDatabase->Set(CDatabase::DATA_UBX_SATELLITES_IN_MEAS_COUNT, gpsSvCount);
//	LOGV("%s: gpsSvCount %d", __FUNCTION__, gpsSvCount);
}
#endif

