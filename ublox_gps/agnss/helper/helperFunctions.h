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
 * Project: Android GNSS Driver
 *
 ******************************************************************************
 * $Id: helperFunctions.h 96357 2015-05-27 17:25:28Z fabio.robbiani $
 * $HeadURL: http://svn.u-blox.ch/GPS/SOFTWARE/hmw/android/release/release_v3.10/gps/agnss/helper/helperFunctions.h $
 *****************************************************************************/
 
#ifndef __HELPER_FUNCTIONS
#define __HELPER_FUNCTIONS
#include "helperTypes.h"

//! Return string to corresponding id
char const * gnssIdToString(unsigned int gnssId);

//! Return string to corresponding data type to string
char const * aidingDataTypeToString(unsigned int dataType);

//! Return string to corresponding MGA ini type to string
char const * mgaIniDataTypeToString(unsigned int iniType);

//! Return string to corresponding service type
char const * agnssServiceTypeToString(unsigned int iniType);

//! Return string to corresponding action type
char const * agnssActionToString(unsigned int iniType);

//! Will generate a string matching to the known UBX message class and id
char const * ubxIdsToString(unsigned int classId, unsigned int msgId);

char const *const STR_UNKNOWN = "UNKNOWN";

//! Verifies if the passed data contains a valid UBX Message data
int isUbxMessage(const unsigned char* pBuffer, size_t iSize, bool ignTrailCh = false);

//! Verifies the passed data contains only whitelisted UBX messages
ssize_t verifyUbxMsgsBlock(unsigned char const * buf, size_t size, UBX_MSG_TYPE const * allowed, size_t allowed_num, BUFC_t **msgs);

//! Checks if the provided UBX message is part of the whitelisted messages
bool isAllowedUbxMsg(unsigned char const * buf, int size, UBX_MSG_TYPE const * allowed, size_t allowed_num, bool allowWildCards = false);

//! Checks if the data contains a valid ALP file and returns the header
int verifyAlpData(const unsigned char* pData, unsigned int iData, ALP_FILE_HEADER_t *header);

//! Read all available bytes on the file descriptor
ssize_t availRead( int fd, unsigned char ** buf, size_t maxSize = 0 );

//! Read data from a file descriptor with timeout end end sequence
ssize_t extRead( int fd
               , unsigned char * buf
               , size_t bufSize
               , unsigned char const * stopSeq
               , size_t stopSeqSize
               , int timeoutMs );

//! Calculate 8-bit Fletcher checksum
uint16_t calculateChecksum(unsigned char const * const data, size_t size, uint16_t const *chk = NULL);

//! Encode an array of buffers into an Agnssf00 format
ssize_t encodeAgnssf00(unsigned char **raw, BUF_t const * bufs, int8_t numBufs);

//! Decode an Agnssf00 formatted memory location into an array of buffers
int8_t decodeAgnssf00(unsigned char const *raw, size_t rawSize, BUF_t * bufs, int8_t numBufs);

//! Replace the content of buf with pMsg
bool replaceBuf(BUF_t* buf, const unsigned char* pMsg, unsigned int iMsg);

//! Free a buffer
bool freeBuf(BUF_t* pBuf);

//! Free an array of buffers
bool freeBufs(BUF_t* pBufs, size_t iBufs);

//! Write a UBX message to the receiver
ssize_t createUbx( unsigned char **buf
                 , unsigned char classID, unsigned char msgID
                 , const void* pData0 = NULL, size_t iData0 = 0
                 , const void* pData1 = NULL, size_t iData1 = 0);

//! Make sure the nanosecond part is >=0 and <= 999999999
void normalizeTime(struct timespec * unTime);

//! Find the earlier time of the two provided times
int timecmp(struct timespec const * time1, struct timespec const * time2);

//! Get the earliest sane time that could represent the system clock
time_t getEarliestSaneTime(struct timespec * earliestTime = NULL);

//! Substract the leap seconds since the Unix epoch, if not done already
void includeLeapSeconds(ACCTIME_t * accTime);

//! Get a value from the monotonic time counter of the OS
bool getMonotonicCounter(struct timespec * monTime);

//! Check if the parameter is a valid \ref ACCTIME_t
bool isValidAccTime(ACCTIME_t const * accTime);

//! Check if the parameter is a valid \ref POS_t
bool isValidPos(POS_t const * pos);

//! Extract the time from the message payload
bool extractMgaIniTimeUtc(MGA_INI_TIME_UTC_t const * data, ACCTIME_t * accTime);

//! Create the payload for an MGA-INI-TIME_UTC message from the given time
bool createMgaIniTimeUtc(MGA_INI_TIME_UTC_t * data, ACCTIME_t const * accTime);

//! Create the payload for an MGA-INI-TIME_LLH message from the given position
bool createMgaIniPos(MGA_INI_POS_t * data, POS_t const * pos);

//! Extract a UBX-AID-INI message
bool extractUbxAidIni( GPS_UBX_AID_INI_U5__t const *pay
                     , ACCTIME_t * accTime
                     , POS_t * pos = NULL );

//! Create a UBX-AID-INI message
bool createUbxAidIni( GPS_UBX_AID_INI_U5__t * pay
                    , ACCTIME_t const * accTime
                    , POS_t const * pos = NULL );

#endif //HELPER_FUNCTIONS

