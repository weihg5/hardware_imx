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
      mEnabled(0),
      mInputReader(4),
      mHasPendingEvent(false),
      mThresholdLux(10)
{
    char  buffer[PROPERTY_VALUE_MAX];

    mPendingEvent.version = sizeof(sensors_event_t);
    mPendingEvent.sensor = ID_L;
    mPendingEvent.type = SENSOR_TYPE_LIGHT;
    memset(mPendingEvent.data, 0, sizeof(mPendingEvent.data));

    if (data_fd >= 0) {
        enable(0, 1);
    }
}

LightSensor::~LightSensor() {
    if (mEnabled) {
        enable(0, 0);
    }
}

int LightSensor::setDelay(int32_t handle, int64_t ns)
{
    //dummy due to not support in driver....
    return 0;
}

int LightSensor::enable(int32_t handle, int en)
{
    int ret;
    bool enable = (en != 0);

	if (enable == mEnabled) {
		return 0;
	}

	ret = fileWriteBool(FILE_ENABLE, enable);
	if (ret < 0) {
		return ret;
	}

    mPreviousLight = -1;
	mEnabled = enable;
	setIntLux();

    return 0;
}

int LightSensor::setIntLux()
{
	int ret;
    int lux, lux_ht, lux_lt;

	ret = fileReadInt(FILE_LUX, &lux);
	if (ret < 0) {
		return ret;
	}

	lux_ht = lux + mThresholdLux;
	lux_lt = lux - mThresholdLux;
	if (lux_lt < 0) {
		lux_lt = 0;
	}

    return fileWriteInt(FILE_INT_HT, lux_ht) | fileWriteInt(FILE_INT_LT, lux_lt);
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
                setIntLux();
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
}
