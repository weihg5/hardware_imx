/******************************************************************************
 * Copyright (C) u-blox AG
 * u-blox AG, Thalwil, Switzerland
 *
 * All rights reserved.
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose without fee is hereby granted, provided that this entire notice
 * is included in all copies of any software which is or includes a copy
 * or modification of this software and in all copies of the supporting
 * documentation for such software.
 *
 * THIS SOFTWARE IS BEING PROVIDED "AS IS", WITHOUT ANY EXPRESS OR IMPLIED
 * WARRANTY. IN PARTICULAR, NEITHER THE AUTHOR NOR U-BLOX MAKES ANY
 * REPRESENTATION OR WARRANTY OF ANY KIND CONCERNING THE MERCHANTABILITY
 * OF THIS SOFTWARE OR ITS FITNESS FOR ANY PARTICULAR PURPOSE.
 *
 ******************************************************************************
 *
 * Project: libMga
 * Purpose: Library providing functions to help a host application to download
 *          MGA assistance data and pass it on to a u-blox GNSS receiver.
 *
 *****************************************************************************
 * $Id: libMga.h 93462 2015-03-30 11:24:00Z marcel.baracchi $
 * $HeadURL: http://svn.u-blox.ch/GPS/SOFTWARE/PRODUCTS/libMga/tags/1.03/src/libMga.h $
 *****************************************************************************/

#ifndef __MGA_LIB__
#define __MGA_LIB__

#include "CommonTypes.h"
#include <time.h>

///////////////////////////////////////////////////////////////////////////////
#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#define LIBMGA_VERSION      "1.03"   //!< libMGA version

#define MGA_GNSS_GPS        0x01    //!< MGA_GNSS_TYPE_FLAGS bit to specify GPS assistance data required.
#define MGA_GNSS_GLO        0x02    //!< MGA_GNSS_TYPE_FLAGS bit to specify GLONASS assistance data required.
#define MGA_GNSS_QZSS       0x04    //!< MGA_GNSS_TYPE_FLAGS bit to specify QZSS assistance data required.


//! Type definition for the flags specifying which GNSS system assistance data is required by the client.
typedef UBX_U1 MGA_GNSS_TYPE_FLAGS;

#define MGA_DATA_EPH    0x01    //!< MGA_DATA_TYPE_FLAGS bit to specify Ephemeris data required.
#define MGA_DATA_ALM    0x02    //!< MGA_DATA_TYPE_FLAGS bit to specify Almanac data required.
#define MGA_DATA_AUX    0x04    //!< MGA_DATA_TYPE_FLAGS bit to specify Auxiliary (Ionospheric & UTC) data required.
#define MGA_DATA_POS    0x08    //!< MGA_DATA_TYPE_FLAGS bit to specify Position approximation required.

//! Type definition for the flags which specify the type of assistance data requested by the client.
typedef UBX_U1 MGA_DATA_TYPE_FLAGS;

#define MGA_FLAGS_USE_POSITION  0x01    //!< MGA_FLAGS_USE bit to specify position fields are valid.
#define MGA_FLAGS_USE_LATENCY   0x02    //!< MGA_FLAGS_USE bit to specify latency field is valid.
#define MGA_FLAGS_USE_TIMEACC   0x04    //!< MGA_FLAGS_USE bit to specify time accuracy field is valid.

//! Type definition for the flags that specify which optional data fields are valid in the MgaOnlineServerConfig structure.
typedef UBX_U1 MGA_FLAGS_USE;

//! libMga API result codes.
/*! Calls to libMga functions always return one of these listed codes.
*/
typedef enum
{
    MGA_API_OK = 0,             //!< Call to API was successful.
    MGA_API_CANNOT_CONNECT,     //!< Could not connect to requested server.
    MGA_API_CANNOT_GET_DATA,    //!< Could not retrieve requested data from server.

    MGA_API_CANNOT_INITIALIZE,  //!< Could not initialize the libMga.
    MGA_API_ALREADY_RUNNING,    //!< MGA session is already running.
    MGA_API_ALREADY_IDLE,       //!< MGA session is already idle.
    MGA_API_IGNORED_MSG,        //!< No processing was performed on the suplied message.
    MGA_API_BAD_DATA,           //!< Parts of the data supplied to the MGA libary are not properly formed UBX messages.
    MGA_API_OUT_OF_MEMORY,      //!< A memory allocation failed.
    MGA_API_NO_MGA_INI_TIME,    //!< The first UBX message in the MGA Online data block is not MGA_INI_TIME (it should be).
    MGA_API_NO_DATA_TO_SEND    //!< The data block does not contain any MGA messages.
} MGA_API_RESULT;

//! Progress event types that can be generated from the libMga.
/*! The libMga can send progress events to the client application by calling the 'evtProgress' callback function.
    The following lists the type of possible progress events.
*/
typedef enum
{
    MGA_PROGRESS_EVT_START,                 //!< Message transfer session has started.
    MGA_PROGRESS_EVT_FINISH,                //!< Message transfer session has completed successfully.
    MGA_PROGRESS_EVT_MSG_SENT,              //!< A UBX MGA message has been sent to the receiver.
    MGA_PROGRESS_EVT_MSG_TRANSFER_FAILED,   //!< A message transfer has failed.
    MGA_PROGRESS_EVT_MSG_TRANSFER_COMPLETE, //!< A message has be successfully transferred.
    MGA_PROGRESS_EVT_TERMINATED,            //!< Message transfer has been terminated.

    MGA_PROGRESS_EVT_SERVER_CONNECTING,     //!< Connecting to server.
    MGA_PROGRESS_EVT_UNKNOWN_SERVER,        //!< Could not find server.
    MGA_PROGRESS_EVT_SERVER_CONNECT,        //!< Connected to a server.
    MGA_PROGRESS_EVT_SERVER_CANNOT_CONNECT, //!< Cannot connect to server.
    MGA_PROGRESS_EVT_REQUEST_HEADER,        //!< A request has been made to an AssistNow service for MGA data.
    MGA_PROGRESS_EVT_RETRIEVE_DATA,         //!< AssistNow MGA data is being received.
    MGA_PROGRESS_EVT_SERVICE_ERROR          //!< There was an error in the communication with the MGA service. The event info object will supply more detailed information.
} MGA_PROGRESS_EVENT_TYPE;

//! Defines the possible states the libMga maintains for each assistance message being processed.
/*! Each message messages given to the libMga for processing, is assigned a state, which changes
    as the message progresses through the system. These possible state are list here.
*/
typedef enum
{
    MGA_MSG_WAITING_TO_SEND,        //!< Initial state of a message. Waiting to be transmitted to receiver.
    MGA_MSG_WAITING_FOR_ACK,        //!< Message has been sent to receiver. libMga now waiting for some kind
                                    //!< of acknowledgement from the receiver.
    MGA_MSG_WAITING_FOR_RESEND,     //!< Receiver has failed to receive the message. libMga is waiting for
                                    //!< a suitable opportunity to resend the message.
    MGA_MSG_RECEIVED,               //!< Message has successfully been received by the receiver.
    MGA_MSG_FAILED                  //!< Receiver has failed to receive the message. libMga will not try to resend          .
} MGA_MSG_STATE;

//! Defines the possible flow control schemes for transfering MGA data between host and receiver.
typedef enum
{
    MGA_FLOW_SIMPLE,        //!< For each message transferred to the receiver, libMga waits for an Ack before sending the next. Reliable but slow.
    MGA_FLOW_NONE,          //!< No flow control. libMga sends MGA data to receiver as fast as possible. Fast but not necessarily reliable.
    MGA_FLOW_SMART          //!< Initially a burst of messages is sent, that will fit into the receiver's 1000 byte rx buffer. Then for every ACK received, the next message is sent.
} MGA_FLOW_CONTROL_TYPE;

//! Definitions of the possible states libMga is in.
/*! Depending on what the libMga is doing, it could be in one of the following possible states.
*/
typedef enum
{
    MGA_IDLE,                       //!< Library is not processing any messages, or expecting any more to process.
    MGA_ACTIVE_PROCESSING_DATA,     //!< Library is busy. There are still messages to process.
    MGA_ACTIVE_WAITING_FOR_DATA     //!< Library has processed successfully all messages given to it, but is expecting more.
} MGA_LIB_STATE;

//! Definitions of possible reasons for receiving a NACK response from the receiver.
typedef enum
{
    // Failed reason code from receiver
    MGA_FAILED_REASON_CODE_NOT_SET,     //!< The is currently no nack reason code.
    MGA_FAILED_NO_TIME,                 //!< The receiver doesn't know the time so can't use the data (To resolve this an UBX-MGA-INI-TIME message should be supplied first).
    MGA_FAILED_VERSION_NOT_SUPPORTED,   //!< The message version is not supported by the receiver.
    MGA_FAILED_SIZE_VERSION_MISMATCH,   //!< The message size does not match the message version.
    MGA_FAILED_COULD_NOT_STORE,         //!< The message data could not be stored to the receiver's database.
    MGA_FAILED_RECEIVER_NOT_READY,      //!< The receiver is not ready to use the message data.
    MGA_FAILED_MESSAGE_UNKNOWN,         //!< The message type is unknown.

    // Failed reason code generated by libMga
    MGA_FAILED_REASON_TOO_MANY_RETRIES = 1000  //!< Too many retries have occurred for this message.
} MGA_FAILED_REASON;


//! These enumerations reflect the types of adjustments that can be made to the MGA-INI-TIME message.
typedef enum
{
    MGA_TIME_ADJUST_ABSOLUTE,   //!< The supplied time should be used to replace the time in the MGA-INI-TIME message.
    MGA_TIME_ADJUST_RELATIVE    //!< The supplied time should be used to adjust the MGA-INI-TIME message. Only the hours, minutes and seconds fields are valid.
} MGA_TIME_ADJUST_TYPE;

//! Function definition for the progress event callback handler.
typedef void (*EvtProgress)(MGA_PROGRESS_EVENT_TYPE evtType,    //!< Progress event type.
                            const void* pContext,               //!< Pointer to context information supplied to the MGA libary in 'mgaInitialize'.
                            const void* pEvtInfo,               //!< Pointer to event information object. See application note for more detailed description.
                            UBX_I4 evtInfoSize                  //!< Size of event information object.
                            );

//! Function definition for the write device callback handler.
typedef void (*EvtWriteDevice)(const void* pContext,            //!< Pointer to the context information supplied to the libMga in 'mgaInitialize'.
                               const UBX_U1* pData,             //!< Pointer to UBX message to write.
                               UBX_I4 iSize                     //!< Number of bytes to write.
                               );

//! Event handler jump table.
/*! This structure defines the jump table of pointers to callback functions for implementing handlers
    to events generated from libMga. This jump table has to be supplied by the application to libMga.
*/
typedef struct
{
    EvtProgress     evtProgress;        //!< Pointer to the 'progress' event handler function.
    EvtWriteDevice  evtWriteDevice;     //!< Pointer to the 'write device data' event handler function.
} MgaEventInterface;

//! Flow control configuration structure.
/*! This structure supplies 'flow' configuration information to libMga.
    Flow information determines how libMga will manage the flow of MGA messages between the host and the receiver.
*/
typedef struct
{
    UBX_I4                  msgTimeOut;     //!< Time, in ms, libMga will wait for a message acknowledgement before marking the message as needing to be re-sent.
    UBX_I4                  msgRetryCount;  //!< The number of retries that a message can have before being declared a failure.
    MGA_FLOW_CONTROL_TYPE   mgaFlowControl; //!< The type of flow control to use for transferring MGA data to the receiver.
} MgaFlowConfiguration;

//! Message information structure.
/*! For each UBX message that needs to be transferred to the receiver, a message info structure
    instance is maintained by the libMga. These are used to manage the transfer of UBX messages
    to the receiver.
    When the progress events MGA_PROGRESS_EVT_MSG_SENT, MGA_PROGRESS_EVT_MSG_TRANSFER_COMPLETE or MGA_PROGRESS_EVT_MSG_TRANSFER_FAILED
    are generated, the 'pEvtinfo' parameter is a pointer to the message information structure of the respective UBX message.
*/

typedef struct
{
    MGA_MSG_STATE       state;              //!< Current state of the UBX message.
    const UBX_U1*       pMsg;               //!< Pointer to the start of the UBX message.
    UBX_U2              msgSize;            //!< The length in bytes of the UBX message.
    time_t              timeOut;            //!< The time in the future when the UBX message is considered to have been lost and not made it to the receiver.
    UBX_U1              retryCount;         //!< The number of times the UBX message has been re-sent to the receiver.
    UBX_U2              sequenceNumber;     //!< Sequence number (order) of the UBX message. Starts from zero.
    MGA_FAILED_REASON   mgaFailedReason;    //!< If this UBX message fails to be accepted by the receiver, this is the reason code.
    union
    {
        struct                  //!< Fields related to MGA message transfers
        {
            UBX_U1              msgId;              //!< UBX-MGA Message Id.
            UBX_U1              mgaPayloadStart[4]; //!< First four bytes of the UBX message payload.
        } mgaMsg;
        struct                  //!< Fields related to flash message transfers
        {
        } mgaFlash;
    };
} MgaMsgInfo;

//! Definitions of the possible errors from the AssistNow services.
/*! Definitions of the possible errors that can occur when retrieving MGA data from the AssistNow services.
*/
typedef enum
{
    MGA_SERVICE_ERROR_NOT_HTTP_HEADER,  //!< No http header was received from service.
    MGA_SERVICE_ERROR_NO_RESPONSE_CODE, //!< The received http header does not contain a response code.
    MGA_SERVICE_ERROR_BAD_STATUS,       //!< The received http header's response code is not 200 (i.e. not ok).
    MGA_SERVICE_ERROR_NO_LENGTH,        //!< The received http header does not contain a contents length field.
    MGA_SERVICE_ERROR_ZERO_LENGTH,      //!< The received http header content length field is zero. i.e no AssistNow data.
    MGA_SERVICE_ERROR_NO_CONTENT_TYPE,  //!< The received http header does not contain a content type field. So we don't know if the data is going to be MGA data.
    MGA_SERVICE_ERROR_NOT_UBX_CONTENT,  //!< The received http header's content type field is not UBX.
    MGA_SERVICE_ERROR_PARTIAL_CONTENT   //!< The amount of AssistNow data received is less than specified in the header's content length field.
} MgaServiceErrors;

//! Event information structure associated with the MGA_PROGRESS_EVT_SERVICE_ERROR progress event type.
/*! Whenever a MGA_PROGRESS_EVT_SERVICE_ERROR event is generated, a pointer to this structure is passed as the
    'pEvtInfo' parameter.
*/
typedef struct
{
    MgaServiceErrors errorType;     //!< AssistNow service error type.
    UBX_U4 httpRc;                  //!< The http response code (if available).
    char errorMessage[100];         //!< The http error message if the the http response code is not 200.
} EvtInfoServiceError;

//! The reasons why a transfer session has been terminated.
typedef enum
{
    TERMINATE_HOST_CANCEL,              //!< Host software cause the termination by calling the mgaSessionStop function.
    TERMINATE_RECEIVER_NACK,            //!< The receiver sent a NACK to the host. This only applies to transferring MGA data to flash.
    TERMINATE_RECEIVER_NOT_RESPONDING,  //!< The termination was caused by the receiver not responding to messages.
    TERMINATE_PROTOCOL_ERROR            //!< Termination caused by libMga receiving an unexpected ACK.
} EVT_TERMINATION_REASON;

//! Online service configuration information structure.
/*! When obtaining MGA data from the Online service, this structure is used to supply information on where the service
    is located and what data is being requested.
*/
typedef struct
{
    const char* strPrimaryServer;       //!< Pointer to string containing the FQDN of the primary server.
    const char* strSecondaryServer;     //!< Pointer to string containing the FQDN of the secondary server.
    const char* strServerToken;         //!< Pointer to the string containing the service access token.
    MGA_GNSS_TYPE_FLAGS gnssTypeFlags;  //!< Requested GNSS data - GPS, GLONASS, QZSS etc.
    MGA_DATA_TYPE_FLAGS dataTypeFlags;  //!< Requested assistance data types - Ephemeris, Almanac, Iono etc.
    MGA_FLAGS_USE useFlags;             //!< Flags specifying what optional data is to be used.
    double dblLatitude;                 //!< Latitude to be used for filtering returned assistance data.
    double dblLongitude;                //!< Longitude to be used for filtering returned assistance data.
    double dblAltitude;                 //!< Altitude to be used for filtering returned assistance data.
    double dblAccuracy;                 //!< Accuracy to be used for filtering returned assistance data.
    double latency;                     //!< Time in seconds to be added to any requested time assistance data.
    double timeAccuracy;                //!< Time in seconds, provided to service, to mark the accuracy of time assistance data.
    bool bFilterOnPos;                  //!< If true, use the position information (lat/lng/alt/acc) to filter returned assistance data.
    const char* strFilterOnSv;          //!< Reserved. Set to NULL.
    void* pInternal;                    //!< Reserved. Set to NULL.
} MgaOnlineServerConfig;

//! Offline service configuration information structure.
/*! When obtaining MGA data from the Offline service, this structure is used to supply information on where the service
    is located and what data is being requested.
*/
typedef struct
{
    const char* strPrimaryServer;       //!< Pointer to string containg the FQDN of the primary server.
    const char* strSecondaryServer;     //!< Pointer to string containg the FQDN of the secondary server.
    const char* strServerToken;         //!< Pointer to the string containg the service access token.
    MGA_GNSS_TYPE_FLAGS gnssTypeFlags;  //!< Requested GNSS data - GPS, GLONASS, QZSS etc.
    int period;                         //!< The number of weeks into the future the MGA data should be valid for. Min 1, max 5.
    int resolution;                     //!< The resolution of the MGA data: 1=every day, 2=every other day, 3=every third day.
    void* pInternal;                    //!< Reserved for u-blox internal use. Set to NULL.
} MgaOfflineServerConfig;


//! Time adjustment structure used by libMga.
/*! The transfer of MGA data to the receiver typically includes the transfer of a UBX-MGA-INI-TIME message.
    If the application needs to specify or adjust the time contained in this message, then the time adjustment
    structure is used to pass this time, either as an absolute time or a time modification, to the appropriate libMga API function.
*/
typedef struct
{
    MGA_TIME_ADJUST_TYPE mgaAdjustType; //!< Type of time adjustment. Relative or absolute.
    UBX_U2 mgaYear;                     //!< Year i.e. 2013
    UBX_U1 mgaMonth;                    //!< Month, starting at 1.
    UBX_U1 mgaDay;                      //!< Day, starting at 1.
    UBX_U1 mgaHour;                     //!< Hour, from 0 to 23.
    UBX_U1 mgaMinute;                   //!< Minute, from 0 to 59.
    UBX_U1 mgaSecond;                   //!< Seconds, from 0 to 59.
    UBX_U2 mgaAccuracyS;                //!< Accuracy of time - Seconds part
    UBX_U2 mgaAccuracyMs;               //!< Accuracy of time - Milli-seconds part
} MgaTimeAdjust;

///////////////////////////////////////////////////////////////////////////////
// libMGA Interface

//! Initialize libMga.
/*! First thing that should be done. Setups up any internal library resources.
    \return Always returns MGA_API_OK
*/
MGA_API_RESULT mgaInit(void);


//! De-initialise libMga.
/*! Last thing that should be done. Releases any internal library resources.

    \return Always returns MGA_API_OK
*/
MGA_API_RESULT mgaDeinit(void);


//! Get library version information.
/*!

    \return     Pointer to a text string containing the library version.
*/
const UBX_CH* mgaGetVersion(void);


//! Configure the libMga.
/*! Sets the flow control, event handler functions and event handler context data.
    \param pFlowConfig      Pointer to a MgaFlowConfiguration structure containing information about how to transfer messages to the receiver.
    \param pEvtInterface    Pointer to event handler callback function jump table.
    \param pCallbackContext Pointer to context information to pass in future to event handler callback functions.

    \return MGA_API_OK if configuration is successful.\n
            MGA_API_ALREADY_RUNNING if configuration failed because library is currently active.
*/
MGA_API_RESULT mgaConfigure(const MgaFlowConfiguration* pFlowConfig,
                            const MgaEventInterface *pEvtInterface,
                            const void* pCallbackContext);


//! Start a data transfer session.
/*!
    \return     MGA_API_OK if session started successfully.\n
                Fails with:\n
                MGA_API_OUT_OF_MEMORY if no more memory to allocate internal buffers.\n
                MGA_API_ALREADY_RUNNING if a session is already running and not idle.
*/
MGA_API_RESULT mgaSessionStart(void);


//! Transfer Online MGA message data to the receiver.
/*!
    \param pMgaData         Pointer to message data.
    \param iSize            Size in bytes.
    \param pMgaTimeAdjust   Pointer to time adjustment structure, to be used to adjust the MGA-TIME-INI message in the message stream. If NULL, then no adjustment will take place.

    \return     MGA_API_OK if session started successfully.\n
                Fails with:\n
                MGA_API_OUT_OF_MEMORY if no more memory to allocate internal buffers.\n
                MGA_API_ALREADY_IDLE if a session is not active and is idle.\n
*/
MGA_API_RESULT mgaSessionSendOnlineData(const UBX_U1* pMgaData,
                                        UBX_I4 iSize,
                                        const MgaTimeAdjust* pMgaTimeAdjust);


//! Transfer Offline message data to the receiver.
/*!
    \param pMgaData         Pointer to message data stream.
    \param iSize            Size in bytes.
    \param pTime            Pointer to time adjustment structure, to be used to set the MGA-TIME-INI message which is sent to the receiver before the supplied MGA data.

    \return     MGA_API_OK if message transfer started successfully.\n
                Fails with:\n
                MGA_API_NO_MGA_INI_TIME - No UBX-MGA-INI-TIME message in message data stream.\n
                MGA_API_NO_DATA_TO_SEND - No data to send. \n
                MGA_API_OUT_OF_MEMORY   - Not enough memory to start message transfer.\n
                MGA_API_ALREADY_IDLE    - If a session is not active and is idle.\n
                MGA_API_BAD_DATA        - If the supplied data is not MGA data.
*/
MGA_API_RESULT mgaSessionSendOfflineData(const UBX_U1* pMgaData,
                                         UBX_I4 iSize,
                                         const MgaTimeAdjust* pTime);


//! Transfer Offline messages to the receiver's flash.
/*!
    \param pMgaData     Pointer to message data.
    \param iSize        Size in bytes of message data.

    \return     MGA_API_OK if message processing successful.\n
                Fails with:\n
                MGA_API_OUT_OF_MEMORY   - Not enough memory to start message transfer.\n
                MGA_API_ALREADY_IDLE    - If a session is not active and is idle.\n
                MGA_API_BAD_DATA        - If the supplied data is not MGA data.


*/
MGA_API_RESULT mgaSessionSendOfflineToFlash(const UBX_U1* pMgaData, UBX_I4 iSize);


//! Stops a data transfer session.
/*!
    \return     MGA_API_OK if session started successfully.\n
                Fails with:\n
                MGA_API_ALREADY_IDLE if no session is running and library is already idle.
*/
MGA_API_RESULT mgaSessionStop(void);


//! Retrieve data from the MGA Online service.
/*! Connects to the MGA servers and requests and retrieves online assistance data according to the supplied
    server configuration information. A buffer is allocated to hold the retrieved data, which is
    returned to the client application.
    It is the client applications responsibility to release this buffer.

    \param pServerConfig    Pointer to server information structure.
    \param ppData           Pointer to return allocated buffer pointer to.
    \param piSize           Pointer to return size of buffer allocated.

    \return     MGA_API_OK if data retrieved successfully.\n
                Fails with:\n
                MGA_API_CANNOT_CONNECT if cannot connect to any servers.\n
                MGA_API_CANNOT_GET_DATA if all data cannot be downloaded.
*/
MGA_API_RESULT mgaGetOnlineData(const MgaOnlineServerConfig* pServerConfig,
                                UBX_U1** ppData,
                                UBX_I4* piSize);


//! Retrieve data from the MGA Offline service.
/*! Connects to the MGA servers and requests and retrieves offline assistance data according to the supplied
    server configuration information. A buffer is allocated to hold the retrieved data, which is
    returned to the client application.
    It is the client applications responsibility to release this buffer.

    \param pServerConfig    Pointer to server information structure.
    \param ppData           Pointer to return allocated buffer pointer to.
    \param piSize           Pointer to return size of buffer allocated.

    \return     MGA_API_OK if data retrieved successfully.\n
                Fails with:\n
                MGA_API_CANNOT_CONNECT if cannot connect to any servers.\n
                MGA_API_CANNOT_GET_DATA if all data cannot be downloaded.
*/
MGA_API_RESULT mgaGetOfflineData(const MgaOfflineServerConfig* pServerConfig,
                                 UBX_U1** ppData,
                                 UBX_I4* piSize);


//! Process a message that has come from the receiver.
/*! The host application should pass any messages received from the receiver to this function.
    The libMga will then inspect the message and if it is of interest, act upon it.
    Messages of interest are those concerned with ACK/NAK of MGA messages that have been transferred to the receiver.
    This function expects the pointer to the message data to pointer to the start of a message and be one message in length.

    \param pMgaData     Pointer to message data.
    \param iSize        Size in bytes of message data.

    \return     MGA_API_OK if message was processed.\n
                MGA_API_IGNORED_MSG if message was ignored.
*/
MGA_API_RESULT mgaProcessReceiverMessage(const UBX_U1* pMgaData,
                                         UBX_I4 iSize);


//! Poll the libMga for any overdue message ACKs.
/*! If any timeouts have been reached, then it will be resent.
    If the maximum number of resends has been reached, then libMga flags the mesage as failed and moves on the the
    next message to send.

    \return     Always returns MGA_API_OK.
*/
MGA_API_RESULT mgaCheckForTimeOuts(void);


//! Builds the query string which will be sent to the service to request Online data.
/*! This is really to aid debugging to see what is actually being sent to the service.
    The query string is constructed based on the contents of the supplied server information structure.

    \param pServerConfig    Pointer to server information structure.
    \param pBuffer          Pointer to the buffer to receive the constructed query string.
    \param iSize            Size in bytes of the buffer.

    \return     Always returns MGA_API_OK.
*/
MGA_API_RESULT mgaBuildOnlineRequestParams(const MgaOnlineServerConfig* pServerConfig,
                                           UBX_CH* pBuffer,
                                           UBX_I4 iSize);


//! Builds the query string which will be sent to the service to request Offline data.
/*! This is really to aid debugging to see what is actually being sent to the service.
    The query string is constructed based on the contents of the supplied server information structure.

    \param pServerConfig    Pointer to server information structure.
    \param pBuffer          Pointer to the buffer to receive the constructed query string.
    \param iSize            Size in bytes of the buffer.

    \return     Always returns MGA_API_OK.
*/
MGA_API_RESULT mgaBuildOfflineRequestParams(const MgaOfflineServerConfig* pServerConfig,
                                            UBX_CH* pBuffer,
                                            UBX_I4 iSize);


//! Erases the MGA Offline data in the receiver's flash.
/*!
    \return     MGA_API_OK if flash erased.\n
                MGA_API_ALREADY_IDLE if no session has been started.
*/
MGA_API_RESULT mgaEraseOfflineFlash(void);


//! Extracts Offline MGA messages for a given day from a superset of MGA Offline data.
/*! Used to support the 'Host Based' MGA Offline scenario where the host application holds all the MGA Offline data
    and a single days worth is extracted and sent to the receiver.
    This API function will 'malloc' a buffer to reurn tthe extracted MGA messages. It is the responsibility of the
    apllication to 'free' this buffer when it is finished with it.

    \param pTime            Pointer to a time structure containing the date of the MGA messages to extract. Only year, month & day fields are used.
    \param pOfflineData     Pointer to a buffer contained all MGA messages.
    \param offlineDataSize  Size in bytes of the buffer.
    \param ppTodaysData     Pointer to a pointer to return the allocated buffer containing the extracted MGA messages.
    \param pTodaysDataSize  Pointer to return the size of the allocated buffer.

    \return     MGA_API_OK if extraction took place.\n
                MGA_API_NO_DATA_TO_SEND if no data could be extracted.
*/
MGA_API_RESULT mgaGetTodaysOfflineData(const tm *pTime, UBX_U1* pOfflineData, UBX_I4 offlineDataSize, UBX_U1** ppTodaysData, UBX_I4* pTodaysDataSize);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif //__MGA_LIB__


