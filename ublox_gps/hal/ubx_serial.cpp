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
 * $Id: ubx_serial.cpp 96416 2015-05-28 10:39:25Z marcus.picasso $
 * $HeadURL: http://svn.u-blox.ch/GPS/SOFTWARE/hmw/android/release/release_v3.10/gps/hal/ubx_serial.cpp $
 *****************************************************************************/
/*!
  \file
  \brief  Serial utility functions

  Utility for accessing the serial port in a easier way
*/

#include "ubx_serial.h"

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <ctype.h>
#include <sys/ioctl.h>

#if defined (ANDROID_BUILD)
#include <termios.h>
#else
#include <sys/termios.h>
#endif

#if defined serial_icounter_struct
#include <linux/serial.h>
#endif
#include <linux/i2c.h>

#include "std_types.h"
#include "std_lang_def.h"
#include "std_macros.h"
#include "ubx_log.h"

const int CSerialPort::s_baudrateTable[BAUDRATE_TABLE_SIZE] =
{
    4800,
    9600,
    19200,
    38400,
    57600,
    115200,
    230400,
};

CSerialPort::CSerialPort()
{
	m_fd = -1;
	m_i2c = false;
}

CSerialPort::~CSerialPort()
{
}

bool CSerialPort::openSerial(const char * pTty, int ttybaud, int blocksize)
{
    //UBXSERLOG("CSerialPort::openSerial: pTty = %s, ttybaud = %d, blocksize = %d", pTty, ttybaud, blocksize);

	closeSerial(); // Make sure the serial interface is closed
    m_i2c = ( strncmp(pTty, "/dev/i2c", 8) == 0 );
	m_fd = open(pTty, (m_i2c ? 0 : O_NOCTTY)
              | O_RDWR
#ifdef O_EXLOCK                 /* for Linux */
              | O_EXLOCK
#endif
        );
    if (m_fd < 0)
    {
        UBX_LOG(LCAT_ERROR, "Cannot open %s port %s, return %d: '%s'", m_i2c?"i2c":"serial", pTty, m_fd, strerror(errno));
        return false;
    }

	if (m_i2c)
	{
		int io = ioctl(m_fd, I2C_SLAVE, 0x42);
		char adr = 0xFF;
		if (io < 0)
		{
			UBX_LOG(LCAT_ERROR, "no i2c device");
	        close (m_fd);
			m_fd = -1;
			return false;
		}
		UBX_LOG(LCAT_VERBOSE, "GPS detected on I2C %s", pTty);
    	if (write(m_fd, &adr, 1) != 1)
		{
			UBX_LOG(LCAT_ERROR, "set register failed");
	        close (m_fd);
			m_fd = -1;
			return false;
		}
	}
	else
	{
		if (settermios(ttybaud, blocksize) == -1)
		{
			UBX_LOG(LCAT_ERROR, "Cannot invoke settermios");
			close (m_fd);
			m_fd = -1;
			return false;
		}
#ifdef BIDIRECTIONAL
		tcflush(m_fd, TCIOFLUSH);
#else
	    tcflush(m_fd, TCIFLUSH);
#endif
		fcntl(m_fd, F_SETFL, 0);

#if defined serial_icounter_struct
		/* Initialize the error counters */
		if (!ioctl(m_fd,TIOCGICOUNT, &einfo))
		{
			/* Nothing to do... */
		}
		else
		{
			/* error, cannot read the error counters */
			UBX_LOG(LCAT_WARNING, "unable to read error counters!!!");
		}
#endif
	}

    UBX_LOG(LCAT_VERBOSE, "Serial port opened, fd = %d", m_fd);

    return true;
}

int CSerialPort::setbaudrate(int ttybaud)
{
	return settermios(ttybaud, -1);
}

int CSerialPort::changeBaudrate(char * pTty, int * pBaudrate, const unsigned char * pBuf, int length)
{
    unsigned long newbaudrate = 0;

    //UBXSERLOG("CSerialPort::changeBaudrate");

    if (m_i2c)
	{
		*pBaudrate = (int)newbaudrate;
        return 1;
	}
	if (length != 28)	  return 0;
    if (pBuf[0] != 0xb5)  return 0;
    if (pBuf[1] != 0x62)  return 0;
    if (pBuf[2] != 0x06)  return 0;
    if (pBuf[3] != 0x00)  return 0;
    if (pBuf[4] != 0x14)  return 0;
    if (pBuf[5] != 0x00)  return 0;
    if ((pBuf[6] != 0x01) && (pBuf[6] != 0xff))  return 0;

    memcpy(&newbaudrate, pBuf+8+6,4);

    if (newbaudrate != (unsigned long) *pBaudrate)
    {
        int tmp = *pBaudrate;

        close (m_fd);

        UBX_LOG(LCAT_WARNING, "Attempting to change baudrate from %d to %lu on %s)",tmp,newbaudrate, pTty);

        /* Wait a while... */
        usleep(100000);

        if (!openSerial(pTty, (int) newbaudrate, 2))
        {
            // failed ?!?
            UBX_LOG(LCAT_ERROR, "%s: %s", pTty, strerror(errno));
            openSerial(pTty, *pBaudrate, 2);
            return 0;
        } else {

            *pBaudrate = (int)newbaudrate;
            UBX_LOG(LCAT_WARNING, "Changed baudrate from to %i on %s)",*pBaudrate, pTty);
            return 1;
        }
    }

    return 0;
}

void CSerialPort::baudrateIncrease(int *pBaudrate) const
{
    int i;
    int oldBaudrate = *pBaudrate;

    /* check next baudrate */
    for (i = 0; i< BAUDRATE_TABLE_SIZE; i++)
    {
        if ( *pBaudrate < s_baudrateTable[i])
        {
            *pBaudrate = s_baudrateTable[i];
            break;
        }
    }
    if (i == BAUDRATE_TABLE_SIZE)
    {
        *pBaudrate = s_baudrateTable[0];
    }

    UBX_LOG(LCAT_WARNING, "Attempting to change baudrate from %d to %d", oldBaudrate, *pBaudrate);

    return;
}

int CSerialPort::retrieveErrors() const
{
    int res = 0;
	if (m_i2c)
		return res;
#if defined serial_icounter_struct
    struct serial_icounter_struct einfo_tmp;

    /* read the counters */
    if (!ioctl(m_fd,TIOCGICOUNT, &einfo_tmp))
    {
        /* check if something has changed */
        if (einfo_tmp.frame != einfo.frame)
        {
            UBX_LOG(LCAT_WARNING, "Frame error occurred!!!! (%d)",einfo_tmp.frame - einfo.frame);

            /* return the number of frame errors */
            res = einfo_tmp.frame - einfo.frame;
        }

        else if (einfo_tmp.brk != einfo.brk)
        {
            UBX_LOG(LCAT_WARNING, "Breaks occurred!!!! (%d)",einfo_tmp.brk - einfo.brk);

            /* return the number of frame errors */
            res = einfo_tmp.brk - einfo.brk;
        }

        /* update the stored counters */
        memcpy(&einfo, &einfo_tmp, sizeof(einfo));
    }
    else
    {
        /* cannot read the counters! ignore, as we might be talking to a device without error counters */
    }
#endif
    return res;
}

void CSerialPort::closeSerial()
{
	if (m_fd > 0)
		close(m_fd);
	m_fd = -1;
	m_i2c = false;
}

bool CSerialPort::fdSet(fd_set &rfds, int &rMaxFd) const
{
	if (m_fd <= 0)
		return false;
	if ((m_fd + 1) > rMaxFd)
		rMaxFd = m_fd + 1;
	FD_SET(m_fd, &rfds);
	return true;
}

bool CSerialPort::fdIsSet(fd_set &rfds) const
{
	if ((m_fd > 0)  && FD_ISSET(m_fd, &rfds))
		return true;
	return false;
}

#ifdef UBX_SERIAL_EXTENSIVE_LOGGING
void CSerialPort::_LogBufferAsHex(const void *pBuffer, unsigned int size, bool bIsWrite) const
{
	char *sHex, *sPrintable;
	const unsigned char *pBufPtr = (unsigned char const *) pBuffer;

	if (!size) return;

	sHex = (char *) malloc(size * 3 + 1);
	sPrintable = (char *) malloc(size + 1);
	if (!sHex || !sPrintable)
		return;

	for(unsigned int i = 0; i < size; ++i)
	{
		sprintf(sHex+(i*3), "%02X ", pBufPtr[i]);
		sprintf(sPrintable+i, "%c", isprint(pBufPtr[i])?pBufPtr[i]:'?');
	}

	sHex[size*3-1] = '\0';
	sPrintable[size] = '\0';

	UBXSERLOG("serial hex: %u %c '%s'", size, bIsWrite ? '>' : '<', sHex);
	UBXSERLOG("serial str: %u %c '%s'", size, bIsWrite ? '>' : '<', sPrintable);

	free(sHex);
	free(sPrintable);
}
#endif // UBX_SERIAL_EXTENSIVE_LOGGING

int CSerialPort::readSerial(void *pBuffer, unsigned int size)
{
	int r;

	//UBXSERLOG("readSerial: m_fd = %d", m_fd);

	if (m_fd <= 0)
		return -1;

	r = read(m_fd, pBuffer, size);

#ifdef UBX_SERIAL_EXTENSIVE_LOGGING
	_LogBufferAsHex(pBuffer, r, false);
#endif // UBX_SERIAL_EXTENSIVE_LOGGING

	return r;
}

ssize_t CSerialPort::writeSerial(const void *pBuffer, size_t size)
{
	if(size > SSIZE_MAX)
		return -1;

	unsigned char* p;

	//UBXSERLOG("writeSerial");
#ifdef UBX_SERIAL_EXTENSIVE_LOGGING
	_LogBufferAsHex(pBuffer, size, true);
#endif // UBX_SERIAL_EXTENSIVE_LOGGING

	if (m_fd <= 0)
		return -1;
	if (!m_i2c)
		return write(m_fd, pBuffer, size);

	p = (unsigned char*)malloc(size+1);
	if (!p)
		return 0;
	p[0] = 0xFF; // allways address the stream register
	memcpy(p+1, pBuffer, size);
	ssize_t written = write(m_fd, p, size+1);

	// Clean up
	// The caller does not know about the additional byte
	// sent. Thus, make sure the caller gets the expected
	// return value.
	if (written > 0)
		written --;
	free(p);

	return size;
}

bool CSerialPort::isFdOpen(void) const
{
	return m_fd > 0;
}

int CSerialPort::settermios(int ttybaud, int blocksize)
{
    struct termios  termIos;

    //UBXSERLOG("settermios");

    if (m_i2c)	return 0;

	if (tcgetattr(m_fd, &termIos) < 0)
    {
        return (-1);
    }
    cfmakeraw (&termIos);
    termIos.c_cflag |= CLOCAL;
/*    {
        int cnt;

        for (cnt = 0; cnt < NCCS; cnt++)
        {
            termIos.c_cc[cnt] = -1;
        }
    } */

	if (blocksize >= 0)
		termIos.c_cc[VMIN] = (unsigned char) blocksize;
    termIos.c_cc[VTIME] = 0;

    //UBXSERLOG("settermios: ttybaud = %d", ttybaud);

#if (B4800 != 4800)
    /*
     * Only paleolithic systems need this.
     */

    switch (ttybaud)
    {
    case 300:
        ttybaud = B300;
        break;
    case 1200:
        ttybaud = B1200;
        break;
    case 2400:
        ttybaud = B2400;
        break;
    case 4800:
        ttybaud = B4800;
        break;
    case 9600:
        ttybaud = B9600;
        break;
    case 19200:
        ttybaud = B19200;
        break;
    case 38400:
        ttybaud = B38400;
        break;
    case 57600:
        ttybaud = B57600;
        break;
    case 115200:
        ttybaud = B115200;
        break;
    default:
        ttybaud = B4800;
        break;
    }
#endif

    if (cfsetispeed(&termIos, (unsigned int) ttybaud) != 0)
    {
        return (-1);
    }
#ifdef BIDIRECTIONAL
    if (cfsetospeed(&termIos, ttybaud) != 0)
    {
        return (-1);
    }
#endif
    if (tcsetattr(m_fd, TCSANOW, &termIos) < 0)
    {
        return (-1);
	}

    //UBXSERLOG("settermios ok");

    return 0;
}
