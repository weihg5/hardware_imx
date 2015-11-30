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
 * $Id: ubx_serial.h 96416 2015-05-28 10:39:25Z marcus.picasso $
 * $HeadURL: http://svn.u-blox.ch/GPS/SOFTWARE/hmw/android/release/release_v3.10/gps/hal/ubx_serial.h $
 *****************************************************************************/

#ifndef __UBX_SERIAL_H__
#define __UBX_SERIAL_H__

#include <fcntl.h>
#include <malloc.h>
#include <string.h>

#define BAUDRATE_TABLE_SIZE 7

//#define UBX_SERIAL_EXTENSIVE_LOGGING

#ifdef UBX_SERIAL_EXTENSIVE_LOGGING
#define UBXSERLOG LOGD
#else
#define UBXSERLOG(...) do { } while(0)
#endif

class CSerialPort
{
public:
    CSerialPort();
    ~CSerialPort();

    bool openSerial(const char * pTty, int ttybaud, int blocksize);

    int setbaudrate(int ttybaud);

    int changeBaudrate(char * ptty, int * pBaudrate, const unsigned char * pBuf, int length);

    void baudrateIncrease(int *baudrate) const;

    int retrieveErrors() const;

    void closeSerial();

    bool fdSet(fd_set &rfds, int &rMaxFd) const;

    bool fdIsSet(fd_set &rfds) const;

#ifdef UBX_SERIAL_EXTENSIVE_LOGGING
    void _LogBufferAsHex(const void *pBuffer, unsigned int size, bool bIsWrite) const;
#endif // UBX_SERIAL_EXTENSIVE_LOGGING

    int readSerial(void *pBuffer, unsigned int size);

    ssize_t writeSerial(const void *pBuffer, size_t size);

	bool isFdOpen(void) const;

private:
    int m_fd;
	bool m_i2c;

    int settermios(int ttybaud, int blocksize);

    static const int s_baudrateTable[BAUDRATE_TABLE_SIZE];

	//! container of error counters
#if defined serial_icounter_struct
    static struct serial_icounter_struct s_einfo;
#endif

};


#endif /* __UBX_SERIAL_H__ */
