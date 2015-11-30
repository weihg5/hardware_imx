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
 * $Id: AssistNowLeg.h 95458 2015-05-07 13:01:29Z fabio.robbiani $
 * $HeadURL: http://svn.u-blox.ch/GPS/SOFTWARE/hmw/android/release/release_v3.10/gps/agnss/AssistNowLeg.h $
 *****************************************************************************/
 
/*! \file
    \ref CAssistNowLeg (AssistNow Legacy Online/Offline) definition.

    \brief
    Definition of \ref CAssistNowLeg, a class derived from \ref CAgnss
*/

#ifndef __UBX_LEG_H__
#define __UBX_LEG_H__
#include <unistd.h>
#include "Agnss.h"
#include "helper/UbxCfgNavX5.h"
#include "helper/UbxPollAidEphAlm.h"
#include "helper/UbxAidEphAlm.h"
#define LEG_MAX_SEND_TRIES 5                 //!< Number of tries sending an
                                             //!< Online message
#define LEG_MAX_RECV_TRIES 15                //!< Number of messages to wait
                                             //!< for an ACK for an Online
                                             //!< message
#define LEG_MAX_CONFIG_ACK_SEND_TRIES 5      //!< Number of tries enabling
                                             //!< aiding ACKs
#define LEG_MAX_CONFIG_ALPSRV_SEND_TRIES 5   //!< Number of tries enabling
                                             //!< ALPSRV requests
#define LEG_MAX_CONFIG_ALPSRV_RECV_TRIES 15  //!< Number of messages to wait
                                             //!< for an ACK for enabling
                                             //!< ALPSRV
#define LEG_MAX_ALPSRV_RECV_TRIES 15         //!< Number of messages to wait
                                             //!< for a ALPSRV request
#define LEG_HTTP_TIMEOUT_S 5                 //!< Number of seconds until a
                                             //!< HTTP timeout kicks in

/*! \class CAssistNowLeg
    \brief Implements \ref CAgnss and accesses the AssistNow legacy services

    This class implements the interface described by \ref CAgnss.
    It will communicate with u-blox receiver generations <8 and the AssistNow
    Legacy Online and Offline servers of u-blox.
*/
class CAssistNowLeg : public CAgnss
{
private: 
	//! Private constructor for the singleton
	CAssistNowLeg(const char *online_primary_server,
         const char *offline_primary_server,
	     const char *server_token,
	     CONF_t *cb);

public:
	//! Default destructor
	virtual ~CAssistNowLeg();

	//! Creates a singleton of this class type
	static CAssistNowLeg *createInstance(const char *online_primary_server,
	                                     const char *offline_primary_server,
	                                     const char *server_token,
	                                     CONF_t *cb);

	//! Return the address of the singleton
	static CAssistNowLeg *getInstance();

	//! Return the address of the singleton
	static void destroyInstance();
protected:
	/////////////////////////////////////////////////////////////////
	// Start function declarations as required to implemet by base //
	/////////////////////////////////////////////////////////////////
	virtual bool impl_init();
	virtual bool impl_deinit();
	virtual CList<CFuncMngr> impl_initAction( SERVICE_t service
	                                         , ACTION_t action );
	virtual bool impl_isValidServiceAction( SERVICE_t service
	                                      , ACTION_t action );
	virtual bool impl_isValidData( SERVICE_t service
	                             , unsigned char const *buf
	                             , size_t size );
	virtual uint16_t impl_getDataId();
	////////////////////////////////////////////////////////////////
	// Stop function declarations as required to implemet by base //
	////////////////////////////////////////////////////////////////
private:

	//! Downloads online data and saves it
	TRI_STATE_FUNC_RESULT_t onlineDownload( unsigned char const *buf
	                                      , size_t size );

	//! Downloads offline data and saves it
	TRI_STATE_FUNC_RESULT_t offlineDownload( unsigned char const *buf
	                                       , size_t size );

	//! Download data for the specified service
	TRI_STATE_FUNC_RESULT_t performDownload(SERVICE_t service);

	//! Transfers online data and
	TRI_STATE_FUNC_RESULT_t onlineTransfer( unsigned char const *buf
	                                      , size_t size );

	//! Offer the stored Offline data to the receiver and handle receiver requests 
	TRI_STATE_FUNC_RESULT_t offlineTransfer( unsigned char const* buf
	                                       , size_t size );

	//! Transfers the current time to the receiver
	TRI_STATE_FUNC_RESULT_t timeTransfer( unsigned char const* buf
	                                    , size_t size );

	//! Transfers the current position to the receiver
	TRI_STATE_FUNC_RESULT_t posTransfer( unsigned char const* buf
	                                   , size_t size );

	//! Helper function to transfer time and position
	TRI_STATE_FUNC_RESULT_t timePosTransfer( unsigned char const *buf
	                                       , size_t size
	                                       , bool forcePosition );

	//! Transfers the current time to the receiver
	TRI_STATE_FUNC_RESULT_t finishTimeTransfer( unsigned char const* buf
	                                          , size_t size );

	//! Insert current time into the msgs that have to be transferred
	bool  insertTransferTimeMsg( unsigned char* msg );

	//! Transfer receiver state data to the receiver
	TRI_STATE_FUNC_RESULT_t recvStateTransfer( unsigned char const *buf
	                                         , size_t size );

	//! Performs polling of the data for the specified service from the receiver 
	TRI_STATE_FUNC_RESULT_t recvStatePoll( unsigned char const *buf
	                                     , size_t size );

	//! Performs the actual download from the specified service
	ssize_t getDataFromServer(SERVICE_t service, unsigned char ** pData);

	//! Will enable acknowoledgment for legacy transfers to the receiver
	TRI_STATE_FUNC_RESULT_t configureAidAck(unsigned char const *buf, size_t size);

	//! Will enable periodic ALPSRV requests by the receiver
	TRI_STATE_FUNC_RESULT_t configurePeriodicAlpSrv(unsigned char const *buf, size_t size);

	//! Parse receiver messages for ALP requests
	TRI_STATE_FUNC_RESULT_t checkForAlpAnswer(unsigned char const * pMsg, unsigned int iMsg);

	//! Verifies if data is valid AssistNow Legacy Offline data
	bool verifyOfflineData(unsigned char const * buf, size_t size);

	//! Create a summary of the stored Offline data and send it to the receiver
	bool writeUbxAlpHeader();

	CUbxAidEphAlm _ubx_aid_eph_alm;             //!< Send current receiver state
	CUbxPollAidEphAlm _ubx_poll_aid_eph_alm;    //!< Poll current receiver state
	CUbxAidIni _ubx_aid_ini;                    //!< UBX-AID-INI handler
	CUbxCfgNavX5  _ubx_cfg_navx5;               //!< Configure Acknowledgment
	TRI_STATE_FUNC_RESULT_t _action_state;      //!< Status of the action
	uint16_t _action_data_id;                   //!< Checksum of the data used
	                                            //!< for the last finished action
	unsigned char *_transfer_buf;               //!< Buffer that will be used
	                                            //!< during the transfer of
	                                            //!< online and offline data to
	                                            //!< the receiver
	size_t _transfer_size;                      //!< Size of _transfer_buf in bytes
	BUFC_t *_msgs;                              //!< Pointers and sizes of the UBX
	                                            //!< messages used for
	                                            //!< transferring online data
	size_t _msgs_num;                           //!< Number of BUF_t structures
	                                            //!< pointed to by _msgs_num
	bool _sent_online_msg;                      //!< Used to determine if a
	                                            //!< message has already been
	                                            //!< sent during the online
	                                            //!< transfer to the receiver
	size_t _online_msg_counter;                 //!< Identifies the current
	                                            //!< message that is processed
	                                            //!< while transferring the
	                                            //!< online messages to the
	                                            //!< receiver
	int _num_client_answers;                    //!< Keeps track of the number
	                                            //!< of messages of any type
	                                            //!< received from the receiver
	                                            //!< while waiting for AssistNow
	                                            //!< Legacy Offline ACKs
	size_t _num_aiding_send_tries;              //!< Keeps track of the number
	                                            //!< of tries while sending
	                                            //!< AssistNow Legacy Online
	                                            //!< messages
	int _num_aiding_recv_tries;                 //!< Keeps track of the number
	                                            //!< of messages of any type
	                                            //!< received from the receiver
	                                            //!< while waiting for AssistNow
	                                            //!< Legacy Online ACKs
	int _num_config_ack_send_tries;             //!< Keeps track of the number
	                                            //!< of tries while trying to 
	                                            //!< configure the receiver
	                                            //!< to send aiding ACKs
	bool _alpsrv_data_save_req;                 //!< Save data back to storage
	                                            //!< indicator
	int _num_config_alpsrv_recv_tries;          //!< Keeps track of the number
	                                            //!< of messages of any type
	                                            //!< received from the receiver
	                                            //!< while trying to configure it
	                                            //!< to send periodic ALPSRV
	                                            //!< requests
	int _num_config_alpsrv_send_tries;          //!< Keeps track of the number
	                                            //!< of tries while trying to 
	                                            //!< configure the receiver
	                                            //!< to send periodic ALPSRV
	                                            //!< requests 
	char * _online_primary_server;              //!< Contains the address
	                                            //!< of the AssistNow Online
	                                            //!< Legacy server
	char * _offline_primary_server;             //!< Contains the address
	                                            //!< of the AssistNow Offline
	                                            //!< Legacy server
	char * _strServerToken;                     //!< Contains the token to be
	                                            //!< used to authenticate to
	                                            //!< the AssistNow Legacy servers

	static CAssistNowLeg *_singleton;           //!< Reference to the singleton
	static pthread_mutex_t _singleton_mutex;    //!< Mutex to protect singleton
	CList<CFuncMngr>                            //!< Defines the action points
	      _fun_lookup[_NUM_SERVICES_]           //!< for a each Action/Service
	                 [_NUM_ACTIONS_];           //!< combination

	static const struct UBX_MSG_TYPE
	                    _aid_allowed[];      //!< Valid LEG Online UBX msgs
};
#endif //ifndef __UBX_LEG_H__

