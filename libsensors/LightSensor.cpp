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

#include "LightSensor.h"

#define SYSFS_ROOT		"/sys/bus/i2c/devices/2-0044/"
#define FILE_ENABLE		SYSFS_ROOT "alsir_en"
#define FILE_LUX		SYSFS_ROOT "lux"
#define FILE_INT_LT		SYSFS_ROOT "int_lt_lux"
#define FILE_INT_HT		SYSFS_ROOT "int_ht_lux"

/*****************************************************************************/
LightSensor::LightSensor()
    : SensorBase(NULL, "isl29044 light and proximity sensor"),
      mInputReader(4),
      mHasPendingEvent(false)
{
    char  buffer[PROPERTY_VALUE_MAX];

	pr_pos_info();

	mEnabled = fileReadBool(FILE_ENABLE, false);

    mPendingEvent.version = sizeof(sensors_event_t);
    mPendingEvent.sensor = ID_L;
    mPendingEvent.type = SENSOR_TYPE_LIGHT;
    memset(mPendingEvent.data, 0, sizeof(mPendingEvent.data));
}

LightSensor::~LightSensor() {
    if (mEnabled) {
        setEnable(0, 0);
    }
}

int LightSensor::setDelay(int32_t handle, int64_t ns)
{
	pr_pos_info();

    return 0;
}

int LightSensor::setEnable(int32_t handle, int enable)
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

    mPreviousLight = -1;
	mEnabled = enable;

    return 0;
}

bool LightSensor::hasPendingEvents() const
{
    return mHasPendingEvent;
}

int LightSensor::readEvents(sensors_event_t* data, int count)
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
            if (event->code == EVENT_TYPE_LIGHT) {
                mPendingEvent.light = event->value;
            }
        } else if (type == EV_SYN) {
            mPendingEvent.timestamp = timevalToNano(event->time);
            if (mEnabled && (mPendingEvent.light != mPreviousLight)) {
                *data++ = mPendingEvent;
                count--;
                numEventReceived++;
                mPreviousLight = mPendingEvent.light;
            }
        } else {
            ALOGE("LightSensor: unknown event (type=%d, code=%d)",
                    type, event->code);
        }
        mInputReader.next();
    }

    return numEventReceived;
}

void LightSensor::processEvent(int code, int value)
{
	pr_pos_info();
}
