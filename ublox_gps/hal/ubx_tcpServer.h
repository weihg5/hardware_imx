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
 * $Id: ubx_tcpServer.h 96363 2015-05-27 18:38:02Z fabio.robbiani $
 * $HeadURL: http://svn.u-blox.ch/GPS/SOFTWARE/hmw/android/release/release_v3.10/gps/hal/ubx_tcpServer.h $
 *****************************************************************************/
/*!
  \file
  \brief  Local TCP Port Handling

  This file provides the implementation of local ports. A master (i.e. what is
  implemented here) broadcasts data it has received to local clients which wait
  for TCP traffic on a defined port.

  If clients talk, the traffic is received in here and delivered to the
  upper layers.

  Connection handling, keep-alive  timeouting is also being done in here
*/

#ifdef TCP_SERVER_PORT
#ifndef __UBX_TCPSERVERL_H__
#define __UBX_TCPSERVERL_H__

#include <arpa/inet.h>
#include <fcntl.h>
#include "../agnss/com/ThreadedPacketIf.h"

class CTcpServer : private CThreadedPacketIf
{
public: // Functions
    CTcpServer();
    ~CTcpServer();

    int startServer(uint16_t port, uint8_t maxConn, bool echo=false, bool localhostOnly = true);
    void stopServer();

    ssize_t onNewMsg(const unsigned char * pBuf, size_t len) const;
	ssize_t sizeNextMsg() const;
	ssize_t getNextMsg(unsigned char *buf, size_t count) const;

    bool fdSet(fd_set &rfds, int &rMaxFd) const;
    bool fdIsSet(fd_set &rfds) const;

protected: // Functions
	/////////////////////////////////////////////////////////////////
	// Start function declarations as required to implemet by base //
	/////////////////////////////////////////////////////////////////
	virtual int impl_open(int **fds, int *timeout_ms);
	virtual int impl_close();
	virtual size_t impl_process(size_t source, unsigned char * buf, size_t size);
	virtual bool impl_notify(size_t source, CComThread::NOTIF_t type);
	virtual void impl_timeout();
	virtual void impl_errorOccurred(int operation, int fdindex, int correrrno);
	////////////////////////////////////////////////////////////////
	// Stop function declarations as required to implemet by base //
	////////////////////////////////////////////////////////////////

private: // Functions
	size_t findAndProcessUbx(size_t dest, unsigned char * buf, size_t size);

	//! Accept new connections
	void acceptConnection();

	//! Check if the source is authenticated
	bool isSourceAuth(size_t source);

	ssize_t authSource(size_t source, unsigned char *buf, size_t size, bool *auth);

private: // Definitions
	typedef struct
	{
		size_t source;
		bool authSucc;
	} AUTH_SOURCE_t;

private: // Variables
	uint8_t _numConn;                //! Number of currently open connections
	uint8_t _maxConn;                //! Maximum number of connections allowed
    int _fdUsrIf;                    //! File descriptor for user side of pipe
	int _fdSocket;                   //! File descriptor TCP socket
	uint16_t _port;                  //! The TCP port
	bool _localhostOnly;             //! Only allow connections from the localhost?
	bool _echo;                      //! Send requests from clients to all clients as well?
	static const
	   	UBX_MSG_TYPE _allowed[];     //! Allowed UBX messages for clients to send to receiver
	static const
		unsigned char _passPhrase[]; //! Passphrase for authentication
	CList<AUTH_SOURCE_t> _authList;  //! Is a source authenticated?
	pthread_mutex_t _authMutex;      //! Mutex to protect _authList
};

#endif /* __UBX_TCPSERVERL_H__ */
#endif //ifdef TCP_SERVER_PORT

