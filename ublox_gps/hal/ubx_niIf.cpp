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
 * $Id: ubx_niIf.cpp 83032 2014-07-14 16:14:58Z fabio.robbiani $
 * $HeadURL: http://svn.u-blox.ch/GPS/SOFTWARE/hmw/android/release/release_v3.10/gps/hal/ubx_niIf.cpp $
 *****************************************************************************/

#include "ubx_niIf.h"

#ifdef SUPL_ENABLED
#include "suplSMmanager.h"
#endif

static CNiIf s_myIf;

const GpsNiInterface CNiIf::s_interface = {
	size:           sizeof(GpsNiInterface),
	init:           init,
	respond:        respond,
};

CNiIf::CNiIf()
{
	m_ready = false;
	m_thread = (pthread_t)NULL;
    sem_init(&sem, 0, 0);
    memset(&m_callbacks, 0, sizeof(m_callbacks));
    m_cmd = SUPL_AUTH_DENIED;
}

CNiIf* CNiIf::getInstance()
{
	return &s_myIf;
}

void CNiIf::init(GpsNiCallbacks* callbacks)
{
    if (s_myIf.m_ready)
        UBX_LOG(LCAT_ERROR, "already initialized");
    UBX_LOG(LCAT_VERBOSE, "");
	s_myIf.m_callbacks = *callbacks;
	s_myIf.m_ready = true;
//lint -e{818} remove  Pointer parameter 'callbacks' (line 53) could be declared as pointing to const
}

void CNiIf::respond(int notif_id, GpsUserResponseType user_response)
{
    UBX_LOG(LCAT_VERBOSE, "id=%d respond=%d(%s)",
         notif_id, user_response, _LOOKUPSTR((unsigned int) user_response, GpsUserResponseType));

#ifdef SUPL_ENABLED	
	s_myIf.m_cmd = SUPL_AUTH_DENIED;
	switch(user_response)
	{
	case GPS_NI_RESPONSE_ACCEPT:
		s_myIf.m_cmd = SUPL_AUTH_GRANT;
		break;
		
	case GPS_NI_RESPONSE_DENY:
		s_myIf.m_cmd = SUPL_AUTH_DENIED;
		break;
	
	case GPS_NI_RESPONSE_NORESP:
		break;
		
	default:
		break;
	}

    // Produce the semaphore
    sem_post(&s_myIf.sem);
#endif	
}


void CNiIf::timoutThread(void *pThreadData)
{
	UBX_LOG(LCAT_VERBOSE, "");
	((void)(pThreadData));
//Todo timeout function is empty
#if 0
	while (1)
	{
		/* wakes up every second to check timed out requests */
		sleep(1);
		
		//pthread_mutex_lock(&loc_eng_ni_data.loc_ni_lock);
		if (not.timeout > 0)
		{
			not.timeout--;
			UBX_LOG(LCAT_DEBUG, "timeout=%d", not.timeout);
			if (not.timeout == 0)
			{
				respond(not.notification_id, GPS_NI_RESPONSE_NORESP);
				pthread_exit(NULL);
			}    	
		}
		//pthread_mutex_unlock(&loc_eng_ni_data.loc_ni_lock);
	} /* while (1) */
#endif
}

#ifdef SUPL_ENABLED
void CNiIf::request(GpsNiNotification* pNotification)
{
    if (!s_myIf.m_ready)
    {
        UBX_LOG(LCAT_ERROR, "class not initialized");
        return;
    }
    UBX_LOG(LCAT_VERBOSE, "");

//	if (!m_thread)
//		m_thread = s_myIf.m_callbacks.create_thread_cb("ni thread", CNiIf::timoutThread, NULL);

	s_myIf.m_callbacks.notify_cb(pNotification);
}

#endif


