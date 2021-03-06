/*
 * Copyright (C) 2008 The Android Open Source Project
 * Copyright (C) 2011-2012 Freescale Semiconductor, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#define LOG_TAG "Sensors"

#include <fcntl.h>
#include <errno.h>
#include <math.h>
#include <stdlib.h>
#include <poll.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/select.h>
#include <cutils/log.h>
#include <cutils/properties.h>

#include "ProximitySensor.h"

#define SYSFS_ROOT		"/sys/bus/i2c/devices/2-0044/"
#define FILE_ENABLE		SYSFS_ROOT "prox_en"
#define FILE_PROX		SYSFS_ROOT "prox"
#define FILE_INT_LT		SYSFS_ROOT "int_lt_prox"
#define FILE_INT_HT		SYSFS_ROOT "int_ht_prox"

/*****************************************************************************/
ProximitySensor::ProximitySensor()
    : SensorBase(NULL, "isl29044 light and proximity sensor"),
      mEnabled(0),
      mInputReader(4),
	  mHasPendingEvent(false)
{
	pr_pos_info();

	mEnabled = fileReadBool(FILE_ENABLE, false);

    mPendingEvent.version = sizeof(sensors_event_t);
    mPendingEvent.sensor = ID_PX;
    mPendingEvent.type = SENSOR_TYPE_PROXIMITY;
    memset(mPendingEvent.data, 0, sizeof(mPendingEvent.data));
}

ProximitySensor::~ProximitySensor() {
    if (mEnabled) {
        setEnable(0, 0);
    }
}

int ProximitySensor::setDelay(int32_t handle, int64_t ns)
{
	pr_pos_info();

    return 0;
}

int ProximitySensor::setEnable(int32_t handle, int enable)
{
	int ret;

	pr_func_info("handle = %d, enable = %d", handle, enable);

	enable = !!enable;
    if (enable == mEnabled) {
		return 0;
    }

	ret = fileWriteBool(FILE_ENABLE, enable);
	if (ret < 0) {
		return ret;
	}

	mEnabled = enable;
    mPreviousProximity = -1;

    return 0;
}

bool ProximitySensor::hasPendingEvents() const
{
    return mHasPendingEvent;
}

int ProximitySensor::readEvents(sensors_event_t* data, int count)
{
    if (count < 1)
        return -EINVAL;

    if (mHasPendingEvent) {
        mHasPendingEvent = false;
        mPendingEvent.timestamp = getTimestamp();
        *data = mPendingEvent;
        return mEnabled ? 1 : 0;
    }

    ssize_t n = mInputReader.fill(data_fd);
    if (n < 0)
        return n;

    int numEventReceived = 0;
    input_event const* event;

    while (count && mInputReader.readEvent(&event)) {
        int type = event->type;
        if (type == EV_ABS) {
            if (event->code == EVENT_TYPE_PROXIMITY) {
                mPendingEvent.distance = event->value;
            }
        } else if (type == EV_SYN) {
            mPendingEvent.timestamp = timevalToNano(event->time);
            if (mEnabled && (mPendingEvent.distance != mPreviousProximity)) {
                *data++ = mPendingEvent;
                count--;
                numEventReceived++;
                mPreviousProximity = mPendingEvent.distance;
            }
        } else {
            ALOGE("ProximitySensor: unknown event (type=%d, code=%d)",
                    type, event->code);
        }
        mInputReader.next();
    }

    return numEventReceived;
}

void ProximitySensor::processEvent(int code, int value)
{
	pr_pos_info();
}
