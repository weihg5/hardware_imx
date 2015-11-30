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
 * $Id: AssistNowLeg.cpp 95458 2015-05-07 13:01:29Z fabio.robbiani $
 * $HeadURL: http://svn.u-blox.ch/GPS/SOFTWARE/hmw/android/release/release_v3.10/gps/agnss/AssistNowLeg.cpp $
 *****************************************************************************/
 
/*! \file
    \ref CAssistNowLeg (AssistNow Legacy Online/Offline) implementation.

    \brief

    Implementation of the functions defined by \ref CAssistNowLeg, a class
    derived from \ref CAgnss
*/

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <new>
#include <limits.h>
#include <errno.h>
#include <assert.h>
#include "AssistNowLeg.h"
#include "func/MemberFunc.h"
#include "helper/helperFunctions.h"

const struct UBX_MSG_TYPE CAssistNowLeg::_aid_allowed[]=
{
	{ 0x0B, 0x01 }, //INI
	{ 0x0B, 0x02 }, // Health / UTC / Iono
	{ 0x0B, 0x30 }, //ALM
	{ 0x0B, 0x31 }, //EPH
	{ 0x0B, 0x33 }  //AOP
};

CAssistNowLeg *CAssistNowLeg::_singleton = NULL;
pthread_mutex_t CAssistNowLeg::_singleton_mutex=PTHREAD_MUTEX_INITIALIZER;

/*! Will create an object of this class type.

    \param online_primary_server   : The zero-terminated string defines
                                     the host name of the server from which the
                                     AssistNow Legacy Online data will be
                                     downloaded. The contents of this string
                                     are copied, the string must thus only
                                     exist during the call to this function.
                                     Must not be NULL.
    \param offline_primary_server  : The zero-terminated string defines
                                     the host name of the server from which the
                                     AssistNow Legacy Offline data will be
                                     downloaded. The contents of this string
                                     are copied, the string must thus only
                                     exist during the call to this function.
                                     Must not be NULL.
    \param server_token            : Token to authenticate against the server
    \param cb                      : Structure of \ref CAgnss::CONF_t type
                                     containing the configuration for the base
                                     class. 
*/
CAssistNowLeg::CAssistNowLeg(const char *online_primary_server,
                             const char *offline_primary_server,
                             const char *server_token,
                             CONF_t *cb)
	: CAgnss(cb, "LEG: ")
	, _ubx_aid_eph_alm((void*)this, CAgnss::writeToRcv, 30)
	, _ubx_poll_aid_eph_alm((void*)this, CAgnss::writeToRcv, 30)
	, _ubx_aid_ini((void*)this, CAgnss::writeToRcv, 30)
	, _ubx_cfg_navx5((void*)this, CAgnss::writeToRcv, 30)
    , _action_state(SUCCESS)
	, _action_data_id(0)
	, _transfer_buf(NULL)
	, _transfer_size(0)
	, _msgs(NULL)
	, _msgs_num(0)
	, _sent_online_msg(false)
	, _online_msg_counter(0)
	, _num_client_answers(-1)
	, _num_aiding_send_tries(0)
	, _num_aiding_recv_tries(0)
	, _num_config_ack_send_tries(0)
	, _alpsrv_data_save_req(false)
	, _num_config_alpsrv_recv_tries(-1)
	, _num_config_alpsrv_send_tries(-1)
	, _online_primary_server(NULL)
	, _offline_primary_server(NULL)
	, _strServerToken(NULL)
{
	_online_primary_server   = online_primary_server ?
	                            strdup(online_primary_server)
	                            : strdup("");
	_offline_primary_server  = offline_primary_server ?
	                            strdup(offline_primary_server)
	                            : strdup("");
	_strServerToken          = server_token ?
	                            strdup(server_token)
                                : strdup("");

	_fun_lookup[TIME]            [TRANSFER ].push(FREF(CAssistNowLeg, this, configureAidAck));
	_fun_lookup[TIME]            [TRANSFER ].push(FREF(CAssistNowLeg, this, timeTransfer));
	_fun_lookup[TIME]            [TRANSFER ].push(FREF(CAssistNowLeg, this, finishTimeTransfer));

	_fun_lookup[POSITION]        [TRANSFER ].push(FREF(CAssistNowLeg, this, configureAidAck));
	_fun_lookup[POSITION]        [TRANSFER ].push(FREF(CAssistNowLeg, this, posTransfer));

	_fun_lookup[ONLINE]          [DOWNLOAD ].push(FREF(CAssistNowLeg, this, onlineDownload));

	_fun_lookup[ONLINE]          [TRANSFER ].push(FREF(CAssistNowLeg, this, configureAidAck));
	_fun_lookup[ONLINE]          [TRANSFER ].push(FREF(CAssistNowLeg, this, onlineTransfer));

	_fun_lookup[OFFLINE]         [DOWNLOAD ].push(FREF(CAssistNowLeg, this, offlineDownload));

	_fun_lookup[OFFLINE]         [TRANSFER ].push(FREF(CAssistNowLeg, this, configurePeriodicAlpSrv));
	_fun_lookup[OFFLINE]         [TRANSFER ].push(FREF(CAssistNowLeg, this, timeTransfer));
	_fun_lookup[OFFLINE]         [TRANSFER ].push(FREF(CAssistNowLeg, this, offlineTransfer));

	_fun_lookup[RECV_AID_STATE]  [POLL_RECV].push(FREF(CAssistNowLeg, this, recvStatePoll));

	_fun_lookup[RECV_AID_STATE]  [TRANSFER ].push(FREF(CAssistNowLeg, this, configureAidAck));
	_fun_lookup[RECV_AID_STATE]  [TRANSFER ].push(FREF(CAssistNowLeg, this, timeTransfer));
	_fun_lookup[RECV_AID_STATE]  [TRANSFER ].push(FREF(CAssistNowLeg, this, recvStateTransfer));
}

/* Destructor freeing the ressources allocated and
   calling \ref teardown as required
*/
CAssistNowLeg::~CAssistNowLeg()
{
	pthread_mutex_lock(&_singleton_mutex);
	teardown();
	free(_msgs);
	free(_online_primary_server);
	free(_offline_primary_server);
	free(_strServerToken);
	free(_transfer_buf);
	_transfer_buf = NULL;
	_singleton = NULL;
	pthread_mutex_unlock(&_singleton_mutex);
}

/*! Creates a singleton of this class or returns the address to the singleton
    if it is already existing. For information on the parameters passed,
    please refer to the constructor documentation
    \ref CAssistNowLeg::CAssistNowLeg

    \param online_primary_server   : Passed on to the constructor if required
                                     otherwise ignored.
    \param offline_primary_server  : Passed on to the constructor if required
                                     otherwise ignored.
    \param server_token            : Passed on to the constructor if required
                                     otherwise ignored.
    \param cb                      : Passed on to the constructor if required
                                     otherwise ignored. 
    \return                          The address of the singleton if newly
	                                 created, NULL otherwise
*/
CAssistNowLeg *CAssistNowLeg::createInstance(const char *online_primary_server,
                                             const char *offline_primary_server,
                                             const char *server_token,
                                             CONF_t *cb)
{
	CAssistNowLeg *tmpRef=NULL;
	pthread_mutex_lock(&_singleton_mutex);
	if(!_singleton)
	{
		_singleton = new (std::nothrow) CAssistNowLeg(online_primary_server, offline_primary_server, server_token, cb);
		tmpRef=_singleton;
	}
	pthread_mutex_unlock(&_singleton_mutex);

	return tmpRef;
}

/*! Returns the address to the singleton - if it is already existing

    \return                          The address of the singleton if
                                     existing, NULL otherwise
*/
CAssistNowLeg *CAssistNowLeg::getInstance()
{
	CAssistNowLeg *tmpRef=NULL;
	pthread_mutex_lock(&_singleton_mutex);
	if(_singleton)
	{
		tmpRef=_singleton;
	}
	pthread_mutex_unlock(&_singleton_mutex);

	return tmpRef;
}

/*! Implements impl_init as required by base. Nothing has to be done during 
    init. Will check if the data initialised in the constructor is correct.

    \return                          true on success, false otherwise
*/
bool CAssistNowLeg::impl_init()
{
	bool result=false;
	if(_online_primary_server
	   && _offline_primary_server
	   && _strServerToken)
	{
		result=true;
	}
	return result;
}

/*! Implements impl_deinit as required by base. Nothing has to be done during 
    deinit. Will always return true.
    \return                          true
*/
bool CAssistNowLeg::impl_deinit()
{
	return true;
}

/*! Implements impl_initAction as required by base.
*/
CList<CFuncMngr> CAssistNowLeg::impl_initAction( SERVICE_t service
                                                , ACTION_t action )
{
	CList<CFuncMngr> result;
	if(isValidServiceAction(service, action))
	{
		result=_fun_lookup[service][action];
		// Clear local data
		_ubx_poll_aid_eph_alm.clearData();
		_ubx_aid_eph_alm.clearData();
	    _ubx_aid_ini.clearData();
		_ubx_cfg_navx5.clearData();
		_action_data_id=0;
		_alpsrv_data_save_req=false;
		_num_client_answers=-1;
		_num_config_ack_send_tries=0;
		free(_transfer_buf);
		_transfer_buf=NULL;
		_transfer_size=0;
		free(_msgs);
		_msgs=NULL;
		_msgs_num=0;

		// A transfer requires data to be stored already and needs valid
		// time information
		if(action==TRANSFER && (!hasData(service) || !hasValidTime()))
		{
			result.clear();
		}
	}
	return result;
}

bool CAssistNowLeg::impl_isValidServiceAction( SERVICE_t service
                                             , ACTION_t action)
{
	bool result=false;
	if(service < _NUM_SERVICES_ && action < _NUM_ACTIONS_)
	{
		result=(_fun_lookup[service][action].getSize()!=0);
	}
	return result;
}

/*! Implements impl_isValidData as required by base.
*/
bool CAssistNowLeg::impl_isValidData(SERVICE_t service, unsigned char const *buf, size_t size)
{
	bool result=false;
	switch(service)
	{
		case RECV_AID_STATE:
		{
			result=(verifyUbxMsgsBlock(buf, size, _aid_allowed, sizeof(_aid_allowed), NULL)>0);
			break;
		}
		case ONLINE:
		{
			result=(verifyUbxMsgsBlock(buf, size, _aid_allowed, sizeof(_aid_allowed), NULL)>0);
			break;
		}
		case OFFLINE:
		{
			result=verifyOfflineData(buf, size);
			break;
		}
		case TIME:
		case _NUM_SERVICES_:
		default:
		{
			break;
		}
	}
	return result;
}

uint16_t CAssistNowLeg::impl_getDataId()
{
	return _action_data_id;
}

TRI_STATE_FUNC_RESULT_t CAssistNowLeg::onlineDownload( unsigned char const *buf
                                                     , size_t size )
{
	(void)(buf);
	(void)(size);
	return performDownload(ONLINE);
}

TRI_STATE_FUNC_RESULT_t CAssistNowLeg::offlineDownload( unsigned char const *buf
                                                      , size_t size )
{
	(void)(buf);
	(void)(size);
	return performDownload(OFFLINE);
}

/*! This function will will download assistance data to the CAgnss super
    with \ref saveToDb.
    
    \param service    : The service from which data should be downloaded
    \return             SUCCESS if the download completed successfully,
                        FAIL if the download failed,
                        CALL_AGAIN if the download is not complete yet and
                        the function has to be called again at least once
                        to complete
*/
TRI_STATE_FUNC_RESULT_t CAssistNowLeg::performDownload(SERVICE_t service)
{
	if(service!=ONLINE && service!=OFFLINE)
		return FAIL;

	TRI_STATE_FUNC_RESULT_t result=FAIL;
	//The initialisation of tmpBuf is a safety measure
	//lint -e(438)
	unsigned char *tmpBuf=NULL;
	ssize_t size=getDataFromServer(service, &tmpBuf);
	if(size>0)
	{
		bool readyForSave=true;
		// For the online services, remove the time from the messages
		// It might be incorrect at a future transfer time and can be
		// used to inform the system of a new time
		if(service==ONLINE)
		{
			readyForSave=false;
			BUFC_t *tmpMsg=NULL;
			if(verifyUbxMsgsBlock(tmpBuf, size, NULL, 0, &tmpMsg)>0)
			{
				if( tmpMsg[0].p             // Valid pointer
				 && tmpMsg[0].i    == sizeof(GPS_UBX_AID_INI_U5__t)+8  // size correct
				 && tmpMsg[0].p[2] == 0x0B  // AID
				 && tmpMsg[0].p[3] == 0x01) // INI
				{
					// Get the current time from the data
					GPS_UBX_AID_INI_U5__t payload;
					ACCTIME_t extractedTime;
					memset(&extractedTime, 0, sizeof(extractedTime));
					memcpy(&payload, tmpBuf+6, sizeof(GPS_UBX_AID_INI_U5__t));
					if(extractUbxAidIni(&payload, &extractedTime) && extractedTime.valid)
					{
						// Use extracted time to set the current time
						setCurrentTime(&extractedTime);

						// Blank out the time for storage
						ACCTIME_t epochStart;
						memset(&epochStart, 0, sizeof(epochStart));
						getEarliestSaneTime(&epochStart.time);
						epochStart.acc.tv_sec=10; // No uncertaintiy - no UBX-AID-INI
						epochStart.valid=true;
						epochStart.leapSeconds=false; // don't introduce leapseconds
						memset(&payload, 0, sizeof(GPS_UBX_AID_INI_U5__t));
						// Create "empty" time payload
						if(createUbxAidIni(&payload, &epochStart))
						{
							unsigned char * ubxmsg=NULL;
							ssize_t ubxsize=createUbx(&ubxmsg, 0x0B, 0x01, &payload, sizeof(payload));
							if(ubxsize>0)
							{
								memcpy(tmpBuf, ubxmsg, (size_t) ubxsize);
								readyForSave=true;
							}
							free(ubxmsg);
						}
					}
					else
					{
						print_err("Could not extract timing or positioning information from the downloaded data");
					}
				}
			}
			free(tmpMsg);
		}
		if(readyForSave)
		{
			if(saveToDb(service, tmpBuf, (size_t) size, &_action_data_id)) 
			{
				print_std("Successfully downloaded and stored LEG %s data from the server", agnssServiceTypeToString(service));
				result=SUCCESS;
			}
			else
			{
				print_err("Soring the downloaded data for LEG %s failed! Abort!", agnssServiceTypeToString(service));
			}
		}
		else
		{
			print_err("Preparing the download LEG %s data for saving failed! Abort!", agnssServiceTypeToString(service));
		}
	}
	else
	{
		print_err("Downloading aiding data from the LEG %s server failed",agnssServiceTypeToString(service));
	}

	free(tmpBuf);
	return result;
}
 
bool CAssistNowLeg::insertTransferTimeMsg( unsigned char *msg)
{
	ACCTIME_t accTimeNow;
	GPS_UBX_AID_INI_U5__t Payload;
	if( !msg
	 || !getCurrentTime(&accTimeNow)
	 || !createUbxAidIni(&Payload, &accTimeNow) )
	{
		return false;
	}
	bool result=false;

	char nowS[20];
	char accS[20];
	struct tm tmpTmp;
	strftime(nowS, 20, "%Y.%m.%d %H:%M:%S", gmtime_r(&accTimeNow.time.tv_sec, &tmpTmp));
	strftime(accS, 20, "%H:%M:%S", gmtime_r(&accTimeNow.acc.tv_sec, &tmpTmp));

	unsigned char *ubxmsg=NULL;
	ssize_t ubxsize=createUbx(&ubxmsg, 0x0B, 0x01, &Payload, sizeof(Payload));
	if(ubxsize > 0)
	{
		if(ubxsize == 48+8)
		{
			memcpy(msg, ubxmsg, (size_t) ubxsize);
			print_std("Using the follwing UTC time for aiding the receiver:"
					  " %s.%.9d (GPS: %uwn:%ums:%uns) Accuracy: %s.%.9d"
			          , nowS, accTimeNow.time.tv_nsec
			          , Payload.wn, Payload.tow, Payload.towNs
			          , accS, accTimeNow.acc.tv_nsec);
			result=true;
		}
		free(ubxmsg);
	}
	return result;
}

//! Initiates the transfer to the receiver for the specified service
TRI_STATE_FUNC_RESULT_t CAssistNowLeg::timeTransfer( unsigned char const *buf
                                                   , size_t size )
{
	return timePosTransfer(buf, size, false);
}

TRI_STATE_FUNC_RESULT_t CAssistNowLeg::posTransfer( unsigned char const *buf
                                                   , size_t size )
{
	return timePosTransfer(buf, size, true);
}

TRI_STATE_FUNC_RESULT_t CAssistNowLeg::timePosTransfer( unsigned char const *buf
                                                      , size_t size
                                                      , bool forcePosition )
{
	TRI_STATE_FUNC_RESULT_t result=FAIL;

	// Send data if none is stored in this class yet
	if(!_ubx_aid_ini.isReadyToSend())
	{
		ACCTIME_t accTimeNow;
		POS_t pos;
		GPS_UBX_AID_INI_U5__t Payload;

		memset(&accTimeNow, 0, sizeof(accTimeNow));
		memset(&pos, 0, sizeof(pos));
		memset(&Payload, 0, sizeof(Payload));
		bool validPos=getCurrentPosition(&pos);

		// It is required to have information about
		// the current. Having valid Position information
		// is optional as long as forcePosition is false
		if( getCurrentTime(&accTimeNow)
		 && ( validPos || !forcePosition )
		 && createUbxAidIni(&Payload, &accTimeNow) )
		{
			char nowS[20];
			char accS[20];
			struct tm tmpTmp;
			strftime(nowS, 20, "%Y.%m.%d %H:%M:%S", gmtime_r(&accTimeNow.time.tv_sec, &tmpTmp));
			strftime(accS, 20, "%H:%M:%S", gmtime_r(&accTimeNow.acc.tv_sec, &tmpTmp));
			if( _ubx_aid_ini.setPosTime(&accTimeNow, validPos?&pos:NULL)
			 && _ubx_aid_ini.sendData())
			{
				print_std("Sent the follwing UTC time for aiding the receiver:"
						  " %s.%09u (GPS: %uwn:%09ums:%09lins) Accuracy: %s.%.9u"
						  , nowS, accTimeNow.time.tv_nsec
						  , Payload.wn, Payload.tow, Payload.towNs
						  , accS, accTimeNow.acc.tv_nsec);

				if(validPos)
				{
					print_std("Sent the follwing position for aiding the receiver:"
							  " latitude: %f longitude: %f Accuracy: %ucm"
							  , ((double) pos.latDeg) *1e-7
							  , ((double) pos.lonDeg) *1e-7
					          , pos.posAccCm );
				}
				result=CALL_AGAIN;
			}
		}
	}
	else
	{
		result=_ubx_aid_ini.onNewMsg(buf, size);
		if(result==SUCCESS)
		{
			print_std("Successfully aided the receiver with the current time!");
			_action_data_id=2;
		}
	}
	return result;
}

//! Sets the _action_data_id to a valid value
TRI_STATE_FUNC_RESULT_t CAssistNowLeg::finishTimeTransfer( unsigned char const *buf
                                                         , size_t size )
{
	((void)(buf));
	((void)(size));
	_action_data_id=1;
	return SUCCESS;  // This is always successful
}

TRI_STATE_FUNC_RESULT_t CAssistNowLeg::recvStateTransfer( unsigned char const *buf
                                                        , size_t size)
{
	TRI_STATE_FUNC_RESULT_t result=FAIL;
	if(!_ubx_aid_eph_alm.hasSentData())
	{
		unsigned char *tmpBuf=NULL;
		ssize_t tmpSize=loadFromDb(RECV_AID_STATE, &tmpBuf, &_action_data_id);
		if(tmpSize>=0)
		{
			if( _ubx_aid_eph_alm.setData(tmpBuf, tmpSize)
			 && _ubx_aid_eph_alm.sendData() )
			{
				result=CALL_AGAIN;
			}
			free(tmpBuf);
		}
	}
	else
	{
		result=_ubx_aid_eph_alm.onNewMsg(buf, size);
	}
	return result;
}

/*! This helper function will initiate the polling data from the receiver
    for the specified service 

    \param buf        : A message that will be parsed for answers
                        from the receiver (May be NULL)
    \param size       : The size of buf (must be 0 if buf is NULL)
    \return             SUCCESS on success,
                        FAIL of failure
                        CALL_AGAIN if additional calls to this
                        function are required
*/
TRI_STATE_FUNC_RESULT_t CAssistNowLeg::recvStatePoll( unsigned char const *buf
                                                    , size_t size)
{
	TRI_STATE_FUNC_RESULT_t result=FAIL;
	if(!_ubx_poll_aid_eph_alm.hasSentData())
	{
		print_std("Sending the Poll request for %s", agnssServiceTypeToString(RECV_AID_STATE));
		if(_ubx_poll_aid_eph_alm.sendData())
		{
			print_std("Sent the Poll request for %s", agnssServiceTypeToString(RECV_AID_STATE));
			result=CALL_AGAIN;
		}
		else
		{
			print_err("Failed to send the Poll request for %s", agnssServiceTypeToString(RECV_AID_STATE));
			result=FAIL;
		}
	}
	else
	{
		result=_ubx_poll_aid_eph_alm.onNewMsg(buf, size);
		switch(result)
		{
			case SUCCESS:
			{
				print_std("Received all required data for the poll request for %s", agnssServiceTypeToString(RECV_AID_STATE));
				result=FAIL;
				unsigned char * tmpBuf=NULL;
				ssize_t tmpSize=_ubx_poll_aid_eph_alm.getData(&tmpBuf);
				if(tmpSize>0)
				{
					if(saveToDb(RECV_AID_STATE, tmpBuf, tmpSize, &_action_data_id))
					{
						print_std("Successfully save the data extracted from the poll request for %s", agnssServiceTypeToString(RECV_AID_STATE));
						result=SUCCESS;
					}
					else
					{
						print_err("Could not save the extracted data from the poll request for %s", agnssServiceTypeToString(RECV_AID_STATE));
					}
					free(tmpBuf);
				}
				else
				{
					print_err("Failed to extract the received data from the poll request for %s", agnssServiceTypeToString(RECV_AID_STATE));
				}
				break;
			}
			case FAIL:
			{
				print_err("Failed to receive all required data for the poll request for %s", (agnssServiceTypeToString(RECV_AID_STATE)));
				break;
			}
			case CALL_AGAIN:
			default:
			{
				break;
			}
		}
	}
	return result;
}

/*! This helper function will create a connection the the u-blox server
    of the specified service and download the required data. 

    \param service    : The service for which data should be downloaded
    \param ppData     : On success the pointer which is passed will
                        be pointing to the downloaded data. This pointer
                        must then be freed on success. Must not be NULL.
    \return             true on success, false of failure
*/
ssize_t CAssistNowLeg::getDataFromServer(SERVICE_t service, unsigned char ** ppData)
{
	if(!ppData)
		return -1;

	int result=-1;
	// get the ip address
	struct sockaddr_in server;
	memset( &server, 0, sizeof (server));
	unsigned long addr=0;
	//lint -esym(438,tmpBuf)
	unsigned char *tmpBuf=NULL;
   	if(service==ONLINE)
	{
		addr= inet_addr(_online_primary_server);
	}
	else //service==OFFLINE
	{
		addr= inet_addr(_offline_primary_server);
	}

	if (addr != INADDR_NONE)  // numeric IP address
	{
		memcpy((char *)&server.sin_addr, &addr, sizeof(addr));
	}
	else
	{
		// get by host name
		struct hostent* host_info=NULL;
   		if(service==ONLINE)
		{
			host_info= gethostbyname(_online_primary_server);
		}
		else //service==OFFLINE
		{
			host_info= gethostbyname(_offline_primary_server);
		}
		if (host_info != NULL)
			memcpy((char *)&server.sin_addr, host_info->h_addr, (unsigned int) host_info->h_length);
		else
		{
			print_err("Cannot resolve LEG %s server %s", agnssServiceTypeToString(service), service==ONLINE?_online_primary_server:_offline_primary_server);
			return -1;
		}
	}
	server.sin_family = AF_INET;
#if defined (_lint)
	server.sin_port	  = 0;
#else
	server.sin_port	  = htons(80);
#endif
	// create the socket
	int sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	fcntl(sock, F_SETFL, SOCK_NONBLOCK);
	if (sock == -1)
	{
		print_err("Cannot create LEG %s socket: %s", agnssServiceTypeToString(service), strerror(errno));
		return -1;
	}
	// establish connection to the server. EINPROGRESS does not indicate
	// an error but an ongoing process
	if ( !connect( sock, (struct sockaddr*) (void *) &server, sizeof(server)) )
	{
		bool fail=true;
		if(errno == EINPROGRESS)
		{
			// Wait for connection (for 15 seconds)
			fd_set fdset;
			FD_ZERO(&fdset);
			FD_SET(sock, &fdset);
			struct timeval selTimeout;
			selTimeout.tv_sec=15; 
			selTimeout.tv_usec=0; 
			int soerr;
			socklen_t solen= sizeof(soerr);
			// Is the socket writeable (ready) now?
			if( select(sock + 1, NULL, &fdset, NULL, &selTimeout) == 1
			 && !getsockopt(sock, SOL_SOCKET, SO_ERROR, &soerr, &solen)
			 && soerr == 0)
			{
				fail=false;
			}
		}

		// Was it possible to connect to the server?
		if(fail)
		{
			print_err("Cannot connect to LEG %s server: %s", agnssServiceTypeToString(service), strerror(errno));
			::close(sock);
			return -1;
		}
	}



    // send the http get request

	char request_str[1024];
	if(service==ONLINE)
	{
		snprintf(request_str, 1024, "GET /GetOnlineData.ashx?"
									"token=%s;format=aid;gnss=gps;datatype=eph,alm,aux,pos HTTP/1.1\r\n"
									"Host: %s\r\n"
									"Accept: */*\r\n"
									"Connection: Keep-Alive\r\n\r\n"
									, _strServerToken
									, _online_primary_server);
	}
	else
	{
		snprintf(request_str, 1024, "GET /GetOfflineData.ashx?"
									"token=%s;format=aid;gnss=gps;days=14 HTTP/1.1\r\n"
									"Host: %s\r\n"
									"Accept: */*\r\n"
									"Connection: Keep-Alive\r\n\r\n"
									, _strServerToken
									, _offline_primary_server);
	}
	print_std( "Sent request to LEG %s server %s"
	         , agnssServiceTypeToString(service)
	         , service==ONLINE?_online_primary_server:_offline_primary_server);

	bool contSend=true;
	const int SLEEP_BETWEEN_TRIES=USEC_PER_SEC/10; // 0.1s
	int tries=LEG_HTTP_TIMEOUT_S*(USEC_PER_SEC/SLEEP_BETWEEN_TRIES);
	do
	{
		if(send(sock,  request_str, strlen(request_str), 0) > 0)
		{
			contSend=false;
		}
		else // An error occured
		{
			bool errAgain=false;
			if(errno==EAGAIN || errno==EWOULDBLOCK) // This is needs special handling
			{
				// Only 'tries' amount of tries are allowed
				//to be answered with 'EAGAIN' or 'EWOULDBLOCK'
				--tries;
				errno=ETIMEDOUT;
				errAgain=true;
			}

			// If the maximum number of EAGAIN are up or another error 
			// occurred, abort at this point. Otherwise retry after
			// a short sleep
			if( tries < 0 || !errAgain )
			{

				print_err("Cannot send request to LEG %s server: %s", agnssServiceTypeToString(service), strerror(errno));
				::close(sock);
				return -1;
			}
			else
			{
				usleep(SLEEP_BETWEEN_TRIES); // Sleep between the sends
			}
		}

	} while(contSend);

	// Get the HTTP Header
	char header[1024];
	char const stopSeq[] = "\r\n\r\n";
	ssize_t iSize=extRead( sock
	                     , (unsigned char *) header
	                     , sizeof(header)-1 // space for '\0'
	                     , (unsigned char const *) stopSeq
	                     , strlen(stopSeq)
	                     , LEG_HTTP_TIMEOUT_S*1000 );

	if(iSize <=0)
	{
		print_err("An error occured during downloading the header from the LEG %s server: %s", agnssServiceTypeToString(service), strerror(errno));
	}
	else if (iSize < (int)(sizeof(header)-1))
	{
		header[iSize] = '\0'; // Make it a string
		print_std("Got header from LEG %s server", agnssServiceTypeToString(service));
		// check the two mandatory fields length and type
		char* pLength = strstr(header, "Content-Length: ");
		char* pType   = strstr(header, "Content-Type: ");
		if (pLength && pType)
		{
			pType += 14;
			pLength += 16;
			ssize_t length= atoi(pLength);
			print_std("LEG %s server reports Content-Length: %i", agnssServiceTypeToString(service), length);
			// check the content type, can be either txt (an error message) or ubx protocol (aiding data)
			bool bUBX  = strncasecmp(pType, "application/ubx", 15)==0;
			bool bTxt  = strncasecmp(pType, "text/plain",      10)==0;
			if(bUBX)
			{
				print_std("LEG %s server reports Content-Type: application/ubx", agnssServiceTypeToString(service));
			}
			else if(bTxt)
			{
				print_std("LEG %s server reports Content-Type: text/plain", agnssServiceTypeToString(service));
			}
			if ((bUBX || bTxt) && (length > 0))
			{
				tmpBuf = (unsigned char*)malloc(length);
				if ( tmpBuf )
				{
                    if (length > 4)
                    {
						ssize_t iRecv=extRead( sock
						                     , tmpBuf
						                     , length
						                     , NULL
						                     , 0
						                     , LEG_HTTP_TIMEOUT_S*1000 );
						if(iRecv < 0)
						{
							print_err("An error occured while downloading data from the LEG %s server", agnssServiceTypeToString(service));
						}
						else if (iRecv==length)
                        {
                            if (bUBX)
                            {
								result=length;
                                print_std("Got AGNSS data of size %d from LEG %s server", length, agnssServiceTypeToString(service));
                            }
                            else if (bTxt)
							{
                                print_err("Got the following message from the LEG %s server: '%s'", agnssServiceTypeToString(service));
							}
                        }
                        else
						{
                            print_err("AGNSS data size does not match Content-Length specified by LEG %s server %d/%d", agnssServiceTypeToString(service), iRecv, length);
						}
                    }
					else
					{
						print_err("Size of data received from LEG %s server too small %d", agnssServiceTypeToString(service), length);
					}
				}
				else
				{
					print_err("Unable to allocate memory for data of size %d received from LEG %s server", length, agnssServiceTypeToString(service));
				}
			}
			else
			{
				print_err("The data received from the LEG %s server is of invalid Content Type / Length", agnssServiceTypeToString(service));
			}
		}
		else
		{
			print_err("Invalid Header received from the LEG %s server", agnssServiceTypeToString(service));
		}
	}
	else
	{
		print_err("Header Size received from the LEG %s server too big", agnssServiceTypeToString(service));
	}
	// we are done close socket
	print_std("Terminating connection to LEG %s server", agnssServiceTypeToString(service));
	::close(sock);
	// Clean up / or provide data to caller
	if(result <= 0)
	{
		free(tmpBuf);
		tmpBuf=NULL;
	}
	else
	{
		*ppData=tmpBuf;
	}
	return result;
}

/*! This function will configure the receiver to send out acknowledgments
    for received legacy aiding data. 

    \param buf        : A pointer to a message from the receiver which
                        is checked for containing the acknowledgment
                        of the receiver that acknowledgments have been
                        enabled for for aiding messages. May be NULL.
    \param size       : The size of the data in bytes pointed to by buf
                        Must be 0 if buf is NULL.
    \return             SUCCESS if the configuration completed successfully,
                        FAIL if the configuration failed,
                        CALL_AGAIN if the configuration is not complete yet
                        and the function has to be called again at least once
                        to complete
*/
TRI_STATE_FUNC_RESULT_t CAssistNowLeg::configureAidAck( unsigned char const *buf
                                                      , size_t size)
{
	TRI_STATE_FUNC_RESULT_t result=FAIL;
	bool sendDataIfRequired=true;
	// If message sent out, check if there has been an answer
	if(_ubx_cfg_navx5.isReadyToSend())
	{
		result=_ubx_cfg_navx5.onNewMsg(buf, size);
		if(result==SUCCESS)
		{
			print_std("Acknowledgment for aiding messages enabled (Try %u/%u)",  _num_config_ack_send_tries, LEG_MAX_CONFIG_ACK_SEND_TRIES);
			_num_config_ack_send_tries=0;
			sendDataIfRequired=false;
			result=SUCCESS;
		}
		else if( result==FAIL )
		{
			if( _num_config_ack_send_tries >= LEG_MAX_CONFIG_ACK_SEND_TRIES )
			{
				print_err("Did not receive acknowledgement from receiver that enabling acknowledgment for aiding messages worked in %u tries. Abort.", LEG_MAX_CONFIG_ACK_SEND_TRIES);
				sendDataIfRequired=false;
				_num_config_ack_send_tries=0;
			}
			else
			{
				print_std("received no acknowledgment for enabling acknowledgments for aiding in time (Try %u/%u)",  _num_config_ack_send_tries, LEG_MAX_CONFIG_ACK_SEND_TRIES);
			}
		}
	}

	// Check if it is required to send the messsage
	if(!_ubx_cfg_navx5.isReadyToSend() && sendDataIfRequired)
	{
		print_std("Configure receiver to acknowledge aiding (Try %u/%u)", _num_config_ack_send_tries+1, LEG_MAX_CONFIG_ACK_SEND_TRIES);
		if( _ubx_cfg_navx5.enableAidingAck(true)
		 && _ubx_cfg_navx5.sendData())
		{
			++_num_config_ack_send_tries;
			result=CALL_AGAIN;
		}
		else
		{
			print_err("Could not enable acknowledgement for aiding!");
			_num_config_ack_send_tries=0;
			result=FAIL;
		}
	}

	return result;

}

/*! This function will configure the receiver to send out periodic 
    ALPSRV requests. 

    \param buf        : A pointer to a message from the receiver which
                        is checked for containing the acknowledgment
                        of the receiver that periodic ALPSRV messages
                        have been enabled. May be NULL.
    \param size       : The size of the data in bytes pointed to by buf
                        Must be 0 if buf is NULL.
    \return             SUCCESS if the configuration completed successfully,
                        FAIL if the configuration failed,
                        CALL_AGAIN if the configuration is not complete yet
                        and the function has to be called again at least once
                        to complete
*/
TRI_STATE_FUNC_RESULT_t CAssistNowLeg::configurePeriodicAlpSrv(unsigned char const *buf, size_t size)
{
	U1 const ubx_cfg_per_alpsrv_en[]= { 0xB5, 0x62, 0x06, 0x01, 0x03, 0x00, 0x0B, 0x32, 0x01, 0x48, 0xC5 };
	U1 const ubx_ack_per_alpsrv_en[]= { 0xB5, 0x62, 0x05, 0x01, 0x02, 0x00, 0x06, 0x01, 0x0F, 0x38 }; 
	U1 const ubx_nak_per_alpsrv_en[]= { 0xb5, 0x62, 0x05, 0x00, 0x02, 0x00, 0x06, 0x01, 0x0e, 0x33 }; 
	TRI_STATE_FUNC_RESULT_t result=CALL_AGAIN;
	// Make sure acknowledgments are
	// enabled for aiding messages
	if(_num_config_alpsrv_send_tries < 0 || _num_config_alpsrv_recv_tries >= LEG_MAX_CONFIG_ALPSRV_RECV_TRIES)
	{
		if(_num_config_alpsrv_send_tries > LEG_MAX_CONFIG_ALPSRV_SEND_TRIES)
		{
			print_std("Tried to activate periodic ALPSRV requests %i times. Receiver not responding", LEG_MAX_CONFIG_ALPSRV_SEND_TRIES);
			_num_config_alpsrv_send_tries=-1;
			result=FAIL;
		}
		else
		{
			if(_num_config_alpsrv_recv_tries >= LEG_MAX_CONFIG_ALPSRV_RECV_TRIES)
			{
				print_err("Did not receive acknowledgement from receiver that enabling periodic ALPSRV requests worked. Retrying...");
			}
			++_num_config_alpsrv_send_tries;
			// This must be the first call to this function
			print_std("Configure receiver to send periodic ALPSRV requests");
		
			_num_config_alpsrv_recv_tries = 0;
			writeToRcv(ubx_cfg_per_alpsrv_en, sizeof(ubx_cfg_per_alpsrv_en));
		}
	}
	else
	{
		++_num_config_alpsrv_recv_tries;

		if(buf && size)
		{

			// Is it a valid UBX message length or is it non-UBX?
			// Check for a UBX header. (Could still be NMEA/invalid)
			if ( size == sizeof(ubx_ack_per_alpsrv_en)           // expected length of our message
				 && !memcmp( buf, ubx_ack_per_alpsrv_en, size))
			{
				//The checksum must be right, thanks to CRcv
				print_std("Receiver will send periodiuc ALPSRV requests");
				result = SUCCESS;
				_num_config_alpsrv_send_tries=-1;
				_num_config_alpsrv_recv_tries=0;
			}
			else if ( size == sizeof(ubx_nak_per_alpsrv_en)           // expected length of our message
				 && !memcmp( buf, ubx_nak_per_alpsrv_en, size))
			{
				print_std("The receiver denied the request do send periodic ALPSRV requests");
				result=FAIL;
				_num_config_alpsrv_send_tries=-1;
				_num_config_alpsrv_recv_tries=0;
			}
		}
	}

	return result;

}

/*! This function will try to send all stored AssistNow Legacy Online messages
    stored in \ref _msgs to the receiver and check for acknowledgements from
    the receiver.

    \param buf        : A pointer to a message from the receiver which
                        is checked for containing the acknowledgment
                        of a previously sent message by this function.
                        May be NULL.
    \param size       : The size of the data in bytes pointed to by buf
                        Must be 0 if buf is NULL.
    \return             SUCCESS if the transfer of all data completed
                        successfully,
                        FAIL if transferring all data failed,
                        CALL_AGAIN if not all messages could be transferred
                        yet and the function has to be called again at least
                        once to complete
*/
TRI_STATE_FUNC_RESULT_t CAssistNowLeg::onlineTransfer( unsigned char const *buf
                                                     , size_t size)
{

	TRI_STATE_FUNC_RESULT_t result = FAIL;
	bool init=false;
	// Initialise the data if not done yet
	if(!_transfer_buf)
	{
		ssize_t tmpTransferSize=loadFromDb(ONLINE, &_transfer_buf, &_action_data_id);
		if(tmpTransferSize<=0)
		{
			if(tmpTransferSize==LOCKED_STORAGE_ERROR)
			{
				print_err("An error occured while trying to load data "
						  "for transferring it to the receiver. Aborting.");
			}
			else if(tmpTransferSize==LOCKED_STORAGE_NO_DATA)
			{
				print_err("There is no data ready for transfer. Aborting.");
			}
		}
		else
		{
			_transfer_size=(size_t)tmpTransferSize;
			ssize_t tmpMsgsNum=verifyUbxMsgsBlock(_transfer_buf, _transfer_size, _aid_allowed, sizeof(_aid_allowed), &_msgs);
			if(tmpMsgsNum<=0)
			{
				print_err("An error occurred while verifying the stored data.");
			}
			else
			{
				_msgs_num=(size_t)tmpMsgsNum;
				if( _msgs[0].i == 48+8
				 && _msgs[0].p
				 && _msgs[0].p[2] == 0x0B // UBX-AID
				 && _msgs[0].p[3] == 0x01 // UBX-AID-INI
				 &&  insertTransferTimeMsg(_transfer_buf) )
				{
					_online_msg_counter=0;
					_sent_online_msg=false;
					_num_aiding_send_tries=0;
					_num_aiding_recv_tries=0;
					init=true;
				}
				else
				{
					print_err("An error occurred while trying to determine the transfer time!");
				}
			}
		}
	}
	else // Initialisation was done before
	{
		init=true;
	}

	// Has the initialisation been successful?
	if(init)
	{
		assert(_msgs);
		if(_sent_online_msg)
		{
			if(_num_aiding_recv_tries > LEG_MAX_RECV_TRIES)
			{
				_num_aiding_recv_tries=0;
				_sent_online_msg=false;
				print_err("Could not find acknowledgment from the receiver for aiding message in %i messages .", LEG_MAX_RECV_TRIES);
			}
			else
			{
				++_num_aiding_recv_tries;
				// At some point we have sent an unacknowledged
				// message to the receiver
				if(buf && size) // We got a message
				{
					ssize_t msg_length=isUbxMessage(buf, size);
					if( msg_length==10 // Has the size of an ACK
					 && buf[2]==0x05)   // Is this the ACK class ID?
					{
						// Does this message acknowledge the message sent before?
						if( _msgs[_online_msg_counter].p
						 && _msgs[_online_msg_counter].p[2]==buf[6]
						 && _msgs[_online_msg_counter].p[3]==buf[7])
						{
							if(buf[3]==0x01)  // Is this the ACK message ID?
							{
								// This acknowledges the type of message
								// we sent out earlier. Send out the next
								// message if there are any left.
								if( _msgs_num < SSIZE_MAX
								 && (_online_msg_counter + 1 >= _msgs_num
								 || !(_msgs[_online_msg_counter+1].p)))
								{
									// The end of the saved data or
									// the size of saveable data has
									// been reached. No need to call
									// this function again right now
									// in this session.
									print_std("Transfer of Message [%i] completed successfully!", _online_msg_counter+1);
									print_std("All messages from the server sent to the receiver successfully!");
									_online_msg_counter=0;
									result=SUCCESS;
								}
								else
								{
									print_std("Transfer of Message [%i] completed successfully!", _online_msg_counter+1);
									++_online_msg_counter;
									_sent_online_msg=false;
									_num_aiding_send_tries=0;
									_num_aiding_recv_tries=0;
									result=CALL_AGAIN;
								}
							}
							else //NACK
							{
								print_err("Received a NACK message from the receiver! Trying to send again!");
								_num_aiding_recv_tries=0;
								_sent_online_msg=false;
							}
						}
					}
					else // Not UBX or irrelevant UBX
					{
						result=CALL_AGAIN;
					}
				}
				else // Timeout
				{
					result=CALL_AGAIN;
				}
			}
		}
		if(!_sent_online_msg)  //_sent_online_msg could have been changed in the meanwhile
		{
			if(_num_aiding_send_tries> LEG_MAX_SEND_TRIES)
			{
				print_err("After %i retires the receiver did acknowledge the aiding message %i. Abort.", LEG_MAX_SEND_TRIES, _online_msg_counter);
			}
			else
			{
				writeToRcv((unsigned char const *)_msgs[_online_msg_counter].p, _msgs[_online_msg_counter].i);
				if( _msgs[_online_msg_counter].p
				 && _msgs[_online_msg_counter].p[2]==0x0B
				 && (_msgs[_online_msg_counter].p[3]==0x30
				 ||  _msgs[_online_msg_counter].p[3]==0x31))
				{
					unsigned int svid=_msgs[_online_msg_counter].p[6]
									 |_msgs[_online_msg_counter].p[7] << 8
									 |_msgs[_online_msg_counter].p[8] << 16
									 |_msgs[_online_msg_counter].p[9] << 24;
					print_std("Message sent [%i/%i]: GPS-%s for %u (size [%u] retries [%i])",_online_msg_counter+1 ,_msgs_num, ubxIdsToString(_msgs[_online_msg_counter].p[2],_msgs[_online_msg_counter].p[3]), svid, _msgs[_online_msg_counter].i, _num_aiding_send_tries);
				}
				else
				{
					print_std("Message sent [%i/%i]: GPS-%s (size [%u] retries [%i])",_online_msg_counter+1 ,_msgs_num, ubxIdsToString(_msgs[_online_msg_counter].p[2],_msgs[_online_msg_counter].p[3]), _msgs[_online_msg_counter].i, _num_aiding_send_tries);
				}
				++_num_aiding_send_tries;
				_sent_online_msg=true;
				_num_aiding_recv_tries=0;
				result=CALL_AGAIN;
			}
		}
	}

	return result;
}

/*! This function will initially send information on the stored Offline data
    to the receiver and will handle the following request of the receiver
    to update this data, respectively receive certain parts of it.

    \param buf        : A pointer to a message from the receiver which
                        is checked for containing the AssistNow Legacy
                        Offline requests. May be NULL.
    \param size       : The size of the data in bytes pointed to by buf
                        Must be 0 if buf is NULL.
    \return             SUCCESS if the transfer of the header completed
                        successfully,
                        FAIL if transferring header data failed,
                        CALL_AGAIN if there could be an incoming message
                        for more data from the receiver
*/
TRI_STATE_FUNC_RESULT_t CAssistNowLeg::offlineTransfer( unsigned char const *buf
                                                      , size_t size)
{
	TRI_STATE_FUNC_RESULT_t result=FAIL;
	if(!_transfer_buf) //Header with data not yet sent
	{
		ssize_t tmp_size=loadFromDb(OFFLINE, &_transfer_buf, &_action_data_id);
		if(tmp_size==LOCKED_STORAGE_ERROR)
		{
			print_err("An error occured while trying to load data "
					  "for transferring it to the receiver. Aborting.");
		}
		else if(tmp_size==LOCKED_STORAGE_NO_DATA)
		{
			print_err("There is no data ready for transfer. Aborting.");
		}
		else
		{
			if(verifyOfflineData(_transfer_buf, tmp_size))
			{
				_transfer_size=tmp_size;
				if(writeUbxAlpHeader())
				{
					result=CALL_AGAIN;
					_alpsrv_data_save_req=false;
					_num_client_answers=-1;
				}
				else
				{
					print_err("Could not send contents of the file Alp file to the receiver! Abort!");
				}
			}
			else
			{
				print_err("An error occured while verifying the stored data.");
				free(_transfer_buf);
				_transfer_buf=NULL;
			}
		}
	}
	else
	{
		result=checkForAlpAnswer(buf, size);
	}
	return result;
}

/*! This helper function will check if the incoming receiver message
    needs any AssistNow Legacy Offline activity which could be sending
    a certain part of the data to it or actually updating the data of
    stored.

    \param pMsg       : A pointer to a message from the receiver which
                        is checked for containing the AssistNow Legacy
                        Offline requests. May be NULL.
    \param iMsg       : The size of the data in bytes pointed to by buf
                        Must be 0 if buf is NULL.
    \return             SUCCESS If no error occured,
                        FAIL if the action that was requested failed,
                        CALL_AGAIN if there could be an incoming message
                        for more data from the receiver
*/
TRI_STATE_FUNC_RESULT_t CAssistNowLeg::checkForAlpAnswer(unsigned char const * pMsg, unsigned int iMsg)
{
	if (!_transfer_buf || !_transfer_size)
	{
		print_err("There is no offline data in the internal storage! Abort!");
		return FAIL;
	}
	TRI_STATE_FUNC_RESULT_t result=FAIL;
	++_num_client_answers;

	// Were too many messages ALPSRV unrelated?
	if(_num_client_answers >= LEG_MAX_ALPSRV_RECV_TRIES)
	{
		if(_alpsrv_data_save_req)
		{
			_alpsrv_data_save_req=false;
			if(saveToDb(OFFLINE, _transfer_buf, _transfer_size, &_action_data_id))
			{
				print_std("Saved the updated ALP data from the receiver");
				result=SUCCESS;
			}
			else
			{
				print_err("Saving the updated ALP data failed!");
			}
		}
		else
		{
			result=SUCCESS; //No answer received
		}
		print_std("No (more?) matching ALPSRV requests received from receiver. Stopping transfer.");
	}
	else if(pMsg && iMsg>8+sizeof(GPS_UBX_AID_ALPSRV_CLI_t))
	{
		// Could it be an ALPSRV request?
		// Is it an ALPSRV request?
		if(pMsg[0]==0xb5 && pMsg[1]==0x62 && pMsg[2]==0x0b && pMsg[3]==0x32)
		{
			// This is a request let's process it,
			// but make sure to check if there is
			// a next one
			_num_client_answers=-1;
			GPS_UBX_AID_ALPSRV_CLI_t cli;

			// Copy the received request to a usable form
			memcpy(&cli, &pMsg[6], sizeof(cli));

			// The client requests the data in number of words and 
			// offsets of words. The data has however be delivered
			// as bytes as well as the size
			unsigned int size_in_bytes=cli.size *2;
			unsigned int offset_in_bytes= cli.ofs*2;
			if ( (size_in_bytes > 0) && (cli.idSize < iMsg) )
			{
				if (offset_in_bytes + size_in_bytes >= _transfer_size)   // if we exceed the file size
				{
					if(offset_in_bytes >= _transfer_size)
					{
						offset_in_bytes = _transfer_size-1;
					}
					// - just send all we've got starting at the offset
					size_in_bytes = _transfer_size - offset_in_bytes;
				}
				if (cli.type == 0xFF)
				{
					// If more data should be changed in the file than was
					// actually received, only the part which was actually
					// received will changed in the actual file
					if(cli.idSize+size_in_bytes+8>iMsg)
					{
						size_in_bytes=iMsg-8-cli.idSize;
					}
					// This is not a request. Instead the receiver
					// wants to update stale data in the file stored
					// here
					if (_action_data_id == cli.fileId)
					{
						_alpsrv_data_save_req=true;
						memcpy(&_transfer_buf[offset_in_bytes], &pMsg[6+cli.idSize], size_in_bytes);
						result=CALL_AGAIN;
						print_std("Updated Alp fileId %d offset %d size %d as requested by client", cli.fileId, offset_in_bytes, size_in_bytes);
					}
					else
					{
						print_err("ALPSRV request to update the stored data failed. The file id %d does not match %d.", cli.fileId, _action_data_id);
					}
				}
				else // Request for specific data
				{
					unsigned int size_data_out=size_in_bytes;
					if(cli.idSize<=sizeof(GPS_UBX_AID_ALPSRV_SRV_t))
					{
						size_data_out+=sizeof(GPS_UBX_AID_ALPSRV_SRV_t);
					}
					else
					{
						size_data_out+=cli.idSize;
					}
					unsigned int size_msg_out=8+size_data_out;
					unsigned char *msg_out=(unsigned char*)malloc(size_msg_out);
					if(msg_out)
					{
						GPS_UBX_AID_ALPSRV_SRV_t srv;
						memset(&srv, 0, sizeof(srv));
						// idSize bytes of the original message have to be
						// returned to the client at the beginning
						memcpy(&srv, &pMsg[6], cli.idSize<sizeof(srv)?cli.idSize:sizeof(srv));
						srv.fileId = _action_data_id;
						srv.dataSize = (U2) size_in_bytes;
						// Copy the stuff that has to be in the header
						// according to specification
						memcpy(msg_out+6, &srv, sizeof(srv));
						if(cli.idSize-sizeof(srv) > 0)
						{
							memcpy(msg_out+6+sizeof(srv), &pMsg[6]+sizeof(srv), cli.idSize-sizeof(srv));
						}
						memcpy(msg_out+6+(cli.idSize<sizeof(srv)?sizeof(srv):cli.idSize), &(_transfer_buf[offset_in_bytes]), size_in_bytes);
						msg_out[0]=0xb5;
						msg_out[1]=0x62;
						msg_out[2]=0x0b;
						msg_out[3]=0x32;
						msg_out[4]=(unsigned char) size_data_out;
						msg_out[5]=(unsigned char) (size_data_out>>8);
						unsigned char ckA = 0;
						unsigned char ckB = 0;
						for (unsigned int i = 2; i < size_data_out+6 ; i++)
						{
							ckA += msg_out[i];
							ckB += ckA;
						}
						msg_out[size_msg_out-2]=ckA;
						msg_out[size_msg_out-1]=ckB;
						writeToRcv(msg_out, size_msg_out);
						print_std("Sent Alp Answer idsize %d type %d ofs %d size %d fileId %d datasize %d as requested by client", srv.idSize, srv.type, srv.ofs, srv.size, srv.fileId, srv.dataSize);
						free(msg_out);
						result=CALL_AGAIN;
					}
					else
					{
						print_err("Allocating memory to send requested Alp data failed for fileId %d", _action_data_id);
					}
				}
			}
			else
			{
				print_err("Request Alp out of range fileId %d offset %d size %d. Actual size of offline data %i", cli.fileId, offset_in_bytes, size_in_bytes, _transfer_size);
			}
		}
		else // Not UBX or not the UBX required
		{
			result=CALL_AGAIN;
		}
	}
	else // Message not big enough for an ALPSRV
	{
		result=CALL_AGAIN;
	}

	// Disable ALPSRV requests from the receiver
	if(result==SUCCESS || result==FAIL)
	{
		U1 const msg_cfg_msg_alpsrv_dis[]= { 0xB5, 0x62, 0x06, 0x01, 0x03, 0x00, 0x0B, 0x32, 0x00, 0x47, 0xC4 };
		writeToRcv(msg_cfg_msg_alpsrv_dis, sizeof(msg_cfg_msg_alpsrv_dis));
		print_std("Disabled periodic ALPSRV requests from the receiver");
	}
    return result;
}

/*! This function will verify if the passed data is valid
    AssistNow Legacy Offline data.

    \param buf        : The buffer containing the AssistNow Legacy
                        Offline data that has to be checked. Must not be NULL
    \param size       : The size of the data pointed to by buf in bytes.
                        Must not be 0.
    \return             On success the 'true'. On failure 'false'.
*/
bool CAssistNowLeg::verifyOfflineData(unsigned char const * buf, size_t size)
{
	if(!buf || !size)
	{
		return false;
	}
	_online_msg_counter=0;
	bool result=false;

	if(size>0)
	{
		ALP_FILE_HEADER_t head;
		switch(verifyAlpData(buf, size, &head))
		{
			case 0:
			{
				print_std("Alp data with GPS time %d:%06d with a durtion of %.3fdays", head.p_wno, head.p_tow, (head.duration * 10.0) / (60.0 * 24.0));
				result=true;
				break;
			}
			case HELPER_ERR_INVALID_ARGUMENT:
			{
				print_err("Alp invalid argument");
				break;
			}
			case HELPER_ERR_ARG_TOO_SHORT:
			{
				print_err("Alp size %d too small", size);
				break;
			}
			case HELPER_ERR_INVALID_MAGIC:
			{
				print_err("Alp magic bad %08X", head.magic);
				break;
			}
			case HELPER_ERR_ENTRY_TOO_SHORT:
			{
				print_err("Alp size bad %d != %d", head.size*4, size);
				break;
			}
			default:
			{
				print_err("Alp Unknown error!");
			}
		}
	}

	if(result)
	{
		print_std("The magic of the offline data is correct");
	}
	else
	{
		print_err("The magic of the offline data is incorrect");
	}

	return result;
}

/*! This helper function will create a message containing information about
    the stored AssistNow Legacy Offline data and send it to the receiver.

    \return             'true' if the data could be sent successfully to the
                        receiver. 'false' otherwise.
*/
bool CAssistNowLeg::writeUbxAlpHeader()
{
	bool result=false;
	int size_data=sizeof(GPS_UBX_AID_ALPSRV_SRV_t)+sizeof(ALP_FILE_HEADER_t);
	int size_msg=size_data+8;
	if(_transfer_buf && _transfer_size>sizeof(ALP_FILE_HEADER_t))
	{
		unsigned char *msg=(unsigned char*)malloc(size_msg);
		if(msg)
		{
			GPS_UBX_AID_ALPSRV_SRV_t srv;
			memset(&srv, 0, sizeof(srv));
			srv.idSize   = sizeof(GPS_UBX_AID_ALPSRV_SRV_t);
			srv.type     = 1; // header need to be type 1
			srv.ofs      = 0;
			srv.size     = sizeof(ALP_FILE_HEADER_t) / 2;
			srv.fileId   = _action_data_id;
			srv.dataSize = sizeof(ALP_FILE_HEADER_t);
			memcpy(msg+6, &srv, sizeof(srv));
			memcpy(msg+6+sizeof(srv), _transfer_buf, sizeof(ALP_FILE_HEADER_t));
			msg[0]=0xb5;
			msg[1]=0x62;
			msg[2]=0x0b;
			msg[3]=0x32;
			msg[4]=(unsigned char) size_data;
			msg[5]=(unsigned char) (size_data>>8);
			unsigned char ckA = 0;
			unsigned char ckB = 0;
			for (int i = 2; i < size_data+6 ; i++)
			{
				ckA += msg[i];
				ckB += ckA;
			}
			msg[size_msg-2]=ckA;
			msg[size_msg-1]=ckB;
			print_std("Send Alp Header idsize %d type %d ofs %d size %d fileId %d datasize %d",
						srv.idSize, srv.type, srv.ofs, srv.size, srv.fileId,
						srv.dataSize);
			writeToRcv(msg, size_msg);
			result=true;
			free(msg);
		}
	}
	return result;
}


