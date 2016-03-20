/*
 * Copyright (C) 2011 The Android Open Source Project
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
/* Copyright (C) 2012-2014 Freescale Semiconductor, Inc. */

#ifndef ANDROID_INCLUDE_IMX_CONFIG_WM8962_H
#define ANDROID_INCLUDE_IMX_CONFIG_WM8962_H

#include "audio_hardware.h"

#define WM8962_DEBUG	0

#define MAX_OUT_VOLUME 127
#define MIN_OUT_VOLUME 100

#define WM8962_AUDIO_MODE                     "AudioMode"
#define WM8962_AUDIO_MODE_MUSIC               "SlaveMusic"
#define WM8962_AUDIO_MODE_CALL                "SlaveCall"
#define WM8962_AUDIO_MODE_BT_MUSIC            "BtSlaveMusic"
#define WM8962_AUDIO_MODE_BT_CALL             "BtSlaveCall"

#define WM8962_PGA_MUX_DAC                    "DAC"
#define WM8962_PGA_MUX_MIXER                  "Mixer"
#define WM8962_SPEAKER_JACK_SWITCH            "Speaker Jack Switch"
#define WM8962_EARPIECE_JACK_SWITCH           "Earpiece Jack Switch"
#define WM8962_RADIO_IN_SWITCH                "RADIO_IN Switch"
#define WM8962_RADIO_OUT_SWITCH               "RADIO_OUT Switch"

#define WM8962_SPEAKER_VOLUME                 "Speaker Volume"
#define WM8962_SPEAKER_VOLUME_VALUE           127
#define WM8962_SPEAKER_SWITCH                 "Speaker Switch"
#define WM8962_SPEAKER_MIXER_SWITCH           "Speaker Mixer Switch"
#define WM8962_SPKOUTL_PGA                    "SPKOUTL PGA"
#define WM8962_SPKOUTR_PGA                    "SPKOUTR PGA"
#define WM8962_SPKOUTL_IN4L_SWITCH            "SPKOUTL Mixer IN4L Switch"
#define WM8962_SPKOUTL_DACL_SWITCH            "SPKOUTL Mixer DACL Switch"
#define WM8962_SPKOUTL_MIXINL_SWITCH          "SPKOUTL Mixer MIXINL Switch"
#define WM8962_SPKOUTR_IN4L_SWITCH            "SPKOUTR Mixer IN4L Switch"
#define WM8962_SPKOUTR_DACL_SWITCH            "SPKOUTR Mixer DACL Switch"
#define WM8962_SPKOUTR_MIXINL_SWITCH          "SPKOUTR Mixer MIXINL Switch"

#define WM8962_HEADPHONE_VOLUME               "Headphone Volume"
#define WM8962_HEADPHONE_VOLUME_VALUE         120
#define WM8962_HEADPHONE_SWITCH               "Headphone Switch"
#define WM8962_HEADPHONE_MIXER_SWITCH         "Headphone Mixer Switch"
#define WM8962_HPOUTL_PGA                     "HPOUTL PGA"
#define WM8962_HPOUTR_PGA                     "HPOUTR PGA"
#define WM8962_HPMIXL_IN4L_SWITCH             "HPMIXL IN4L Switch"
#define WM8962_HPMIXL_DACL_SWITCH             "HPMIXL DACL Switch"

#define WM8962_CAPTURE_SWITCH                 "Capture Switch"
#define WM8962_CAPTURE_VOLUME                 "Capture Volume"

#define WM8962_INPGAL_IN2L_SWITCH             "INPGAL IN2L Switch"
#define WM8962_MIXINL_IN2L_SWITCH             "MIXINL IN2L Switch"
#define WM8962_MIXINL_IN2L_VOLUME             "MIXINL IN2L Volume"

#define WM8962_INPGAR_IN3R_SWITCH             "INPGAR IN3R Switch"
#define WM8962_MIXINR_IN3R_SWITCH             "MIXINR IN3R Switch"
#define WM8962_MIXINR_IN3R_VOLUME             "MIXINR IN3R Volume"

#define WM8962_MIXINR_PGA_SWITCH              "MIXINR PGA Switch"
#define WM8962_MIXINR_PGA_VOLUME              "MIXINR PGA Volume"

#define WM8962_MIXINL_PGA_SWITCH              "MIXINL PGA Switch"
#define WM8962_MIXINL_PGA_VOLUME              "MIXINL PGA Volume"

#ifdef MODEM_EC20
#define WM8962_MIXINL_PGA_VALUE               1
#else
#define WM8962_MIXINL_PGA_VALUE               0
#endif

#define WM8962_DIGITAL_CAPTURE_VOLUME         "Digital Capture Volume"

#define WM8962_DIGITAL_PLAYBACK_VOLUME        "Digital Playback Volume"
#define WM8962_INPGAL_IN4L_SWITCH             "INPGAL IN4L Switch"
#define WM8962_HPMIXL_MIXINL_SWITCH           "HPMIXL MIXINL Switch"
/* These are values that never change */
static struct route_setting defaults_wm8962[] = {
    /* general */
    {
        .ctl_name = WM8962_DIGITAL_PLAYBACK_VOLUME,
        .intval = 96,
    },
    {
        .ctl_name = NULL,
    },
};

static struct route_setting bt_output_wm8962[] = {
    {
        .ctl_name = NULL,
    },
};

static struct route_setting speaker_output_wm8962[] = {
    {
        .ctl_name = WM8962_RADIO_IN_SWITCH,
        .intval = 0,
    },
    {
        .ctl_name = WM8962_RADIO_OUT_SWITCH,
        .intval = 0,
    },
    {
        .ctl_name = WM8962_SPEAKER_SWITCH,
        .intval = 1,
    },
    {
        .ctl_name = WM8962_SPEAKER_VOLUME,
        .intval = WM8962_SPEAKER_VOLUME_VALUE,
    },
	{
		.ctl_name = WM8962_SPKOUTL_DACL_SWITCH,
		.intval = 1,
	},
	{
		.ctl_name = WM8962_SPKOUTR_DACL_SWITCH,
		.intval = 1,
	},
	{
		.ctl_name = WM8962_SPEAKER_MIXER_SWITCH,
		.intval = 1,
	},
    {
        .ctl_name = WM8962_SPKOUTL_PGA,
        .strval = WM8962_PGA_MUX_MIXER,
    },
    {
        .ctl_name = WM8962_SPKOUTR_PGA,
        .strval = WM8962_PGA_MUX_MIXER,
    },
	{
		.ctl_name = WM8962_SPEAKER_JACK_SWITCH,
		.intval = 1,
	},
#if WM8962_DEBUG
    {
        .ctl_name = WM8962_HEADPHONE_SWITCH,
        .intval = 1,
    },
    {
        .ctl_name = WM8962_HEADPHONE_VOLUME,
        .intval = WM8962_HEADPHONE_VOLUME_VALUE,
    },
	{
		.ctl_name = WM8962_HPMIXL_DACL_SWITCH,
		.intval = 1,
	},
	{
		.ctl_name = WM8962_HEADPHONE_MIXER_SWITCH,
		.intval = 1,
	},
    {
        .ctl_name = WM8962_HPOUTL_PGA,
        .strval = WM8962_PGA_MUX_MIXER,
    },
	{
		.ctl_name = WM8962_EARPIECE_JACK_SWITCH,
		.intval = 1,
	},
#endif
    {
        .ctl_name = NULL,
    },
};

static struct route_setting hs_output_wm8962[] = {
	{
		.ctl_name = NULL,
	},
};

static struct route_setting earpiece_output_wm8962[] = {
    {
        .ctl_name = WM8962_RADIO_IN_SWITCH,
        .intval = 0,
    },
    {
        .ctl_name = WM8962_RADIO_OUT_SWITCH,
        .intval = 0,
    },
#if WM8962_DEBUG
	{
		.ctl_name = WM8962_SPEAKER_SWITCH,
		.intval = 1,
	},
	{
		.ctl_name = WM8962_SPEAKER_VOLUME,
		.intval = WM8962_SPEAKER_VOLUME_VALUE,
	},
	{
		.ctl_name = WM8962_SPKOUTL_DACL_SWITCH,
		.intval = 1,
	},
	{
		.ctl_name = WM8962_SPKOUTR_DACL_SWITCH,
		.intval = 1,
	},
	{
		.ctl_name = WM8962_SPEAKER_MIXER_SWITCH,
		.intval = 1,
	},
	{
		.ctl_name = WM8962_SPKOUTL_PGA,
		.strval = WM8962_PGA_MUX_MIXER,
	},
	{
		.ctl_name = WM8962_SPKOUTR_PGA,
		.strval = WM8962_PGA_MUX_MIXER,
	},
	{
		.ctl_name = WM8962_SPEAKER_JACK_SWITCH,
		.intval = 1,
	},
#endif
	{
		.ctl_name = WM8962_HEADPHONE_SWITCH,
		.intval = 1,
	},
	{
		.ctl_name = WM8962_HEADPHONE_VOLUME,
		.intval = WM8962_HEADPHONE_VOLUME_VALUE,
	},
	{
		.ctl_name = WM8962_HPMIXL_DACL_SWITCH,
		.intval = 1,
	},
	{
		.ctl_name = WM8962_HEADPHONE_MIXER_SWITCH,
		.intval = 1,
	},
	{
		.ctl_name = WM8962_HPOUTL_PGA,
		.strval = WM8962_PGA_MUX_MIXER,
	},
	{
		.ctl_name = WM8962_EARPIECE_JACK_SWITCH,
		.intval = 1,
	},
	{
		.ctl_name = NULL,
	},
};

static struct route_setting vx_hs_mic_input_wm8962[] = {
    {
        .ctl_name = WM8962_CAPTURE_SWITCH,
        .intval = 1,
    },
    {
        .ctl_name = WM8962_CAPTURE_VOLUME,
        .intval = 63,
    },
    {
        .ctl_name = WM8962_DIGITAL_CAPTURE_VOLUME,
        .intval = 127,
    },
#if 0
    {
        .ctl_name = WM8962_MIXINR_IN3R_SWITCH,
        .intval = 1,
    },
    {
        .ctl_name = WM8962_MIXINR_IN3R_VOLUME,
        .intval = 7,
    },
#else
#if WM8962_MIXINL_PGA_VALUE
	{
		.ctl_name = WM8962_MIXINL_IN2L_SWITCH,
		.intval = 0,
	},
#else
	{
		.ctl_name = WM8962_MIXINL_IN2L_SWITCH,
		.intval = 1,
	},
#endif
	{
		.ctl_name = WM8962_MIXINL_PGA_SWITCH,
		.intval = WM8962_MIXINL_PGA_VALUE,
	},
#endif
#if 0 // WM8962_DEBUG
    {
        .ctl_name = WM8962_HPMIXL_MIXINL_SWITCH,
        .intval = 1,
    },
    {
        .ctl_name = WM8962_HPMIXL_MIXINR_SWITCH,
        .intval = 1,
    },
    {
        .ctl_name = WM8962_HEADPHONE_MIXER_SWITCH,
        .intval = 1,
    },
    {
        .ctl_name = WM8962_HPOUTL_PGA,
        .strval = WM8962_PGA_MUX_MIXER,
    },
    {
        .ctl_name = WM8962_HEADPHONE_SWITCH,
        .intval = 1,
    },
    {
        .ctl_name = WM8962_HEADPHONE_VOLUME,
        .intval = WM8962_HEADPHONE_VOLUME_VALUE,
    },
#endif
    {
        .ctl_name = NULL,
    },
};


static struct route_setting mm_main_mic_input_wm8962[] = {
    {
        .ctl_name = WM8962_CAPTURE_SWITCH,
        .intval = 1,
    },
    {
        .ctl_name = WM8962_CAPTURE_VOLUME,
        .intval = 63,
    },
    {
        .ctl_name = WM8962_DIGITAL_CAPTURE_VOLUME,
        .intval = 127,
    },/*
    {
        .ctl_name = WM8962_INPGAR_IN3R_SWITCH,
        .intval = 1,
    },
    {
        .ctl_name = WM8962_MIXINR_PGA_SWITCH,
        .intval = 1,
    },
    {
        .ctl_name = WM8962_MIXINR_PGA_VOLUME,
        .intval = 7,
    },*/
#if 0
    {
        .ctl_name = WM8962_MIXINR_IN3R_SWITCH,
        .intval = 1,
    },
    {
        .ctl_name = WM8962_MIXINR_IN3R_VOLUME,
        .intval = 7,
    },
#else
	{
		.ctl_name = WM8962_MIXINL_IN2L_SWITCH,
		.intval = 1,
	},
	{
		.ctl_name = WM8962_MIXINL_PGA_SWITCH,
		.intval = 0,
	},
#endif
#if 0 // WM8962_DEBUG
    {
        .ctl_name = WM8962_HPMIXL_MIXINL_SWITCH,
        .intval = 1,
    },
    {
        .ctl_name = WM8962_HPMIXL_MIXINR_SWITCH,
        .intval = 1,
    },
    {
        .ctl_name = WM8962_HEADPHONE_MIXER_SWITCH,
        .intval = 1,
    },
    {
        .ctl_name = WM8962_HPOUTL_PGA,
        .strval = WM8962_PGA_MUX_MIXER,
    },
    {
        .ctl_name = WM8962_HEADPHONE_SWITCH,
        .intval = 1,
    },
    {
        .ctl_name = WM8962_HEADPHONE_VOLUME,
        .intval = WM8962_HEADPHONE_VOLUME_VALUE,
    },
#endif
    {
        .ctl_name = NULL,
    },
};


static struct route_setting vx_main_mic_input_wm8962[] = {
    {
        .ctl_name = WM8962_CAPTURE_SWITCH,
        .intval = 1,
    },
    {
        .ctl_name = WM8962_CAPTURE_VOLUME,
        .intval = 63,
    },
    {
        .ctl_name = WM8962_DIGITAL_CAPTURE_VOLUME,
        .intval = 127,
    },
#if 0
    {
        .ctl_name = WM8962_MIXINR_IN3R_SWITCH,
        .intval = 1,
    },
    {
        .ctl_name = WM8962_MIXINR_IN3R_VOLUME,
        .intval = 7,
    },
#else
#if WM8962_MIXINL_PGA_VALUE
	{
		.ctl_name = WM8962_MIXINL_IN2L_SWITCH,
		.intval = 0,
	},
#else
	{
		.ctl_name = WM8962_MIXINL_IN2L_SWITCH,
		.intval = 1,
	},
#endif
	{
		.ctl_name = WM8962_MIXINL_PGA_SWITCH,
		.intval = WM8962_MIXINL_PGA_VALUE,
	},
#endif
#if 0 // WM8962_DEBUG
    {
        .ctl_name = WM8962_HPMIXL_MIXINL_SWITCH,
        .intval = 1,
    },
    {
        .ctl_name = WM8962_HPMIXL_MIXINR_SWITCH,
        .intval = 1,
    },
    {
        .ctl_name = WM8962_HEADPHONE_MIXER_SWITCH,
        .intval = 1,
    },
    {
        .ctl_name = WM8962_HPOUTL_PGA,
        .strval = WM8962_PGA_MUX_MIXER,
    },
    {
        .ctl_name = WM8962_HEADPHONE_SWITCH,
        .intval = 1,
    },
    {
        .ctl_name = WM8962_HEADPHONE_VOLUME,
        .intval = WM8962_HEADPHONE_VOLUME_VALUE,
    },
#endif
    {
        .ctl_name = NULL,
    },
};

/*hs_mic exchanged with main mic for sabresd, because the the main is no implemented*/
static struct route_setting mm_hs_mic_input_wm8962[] = {
    {
        .ctl_name = WM8962_CAPTURE_SWITCH,
        .intval = 1,
    },
    {
        .ctl_name = WM8962_CAPTURE_VOLUME,
        .intval = 63,
    },
    {
        .ctl_name = WM8962_DIGITAL_CAPTURE_VOLUME,
        .intval = 127,
    },/*
    {
        .ctl_name = WM8962_INPGAR_IN3R_SWITCH,
        .intval = 1,
    },
    {
        .ctl_name = WM8962_MIXINR_PGA_SWITCH,
        .intval = 1,
    },
    {
        .ctl_name = WM8962_MIXINR_PGA_VOLUME,
        .intval = 7,
    },*/
    {
        .ctl_name = WM8962_MIXINR_IN3R_SWITCH,
        .intval = 1,
    },
    {
        .ctl_name = WM8962_MIXINR_IN3R_VOLUME,
        .intval = 7,
    },
    {
        .ctl_name = NULL,
    },
};

static struct route_setting vx_bt_mic_input_wm8962[] = {
    {
        .ctl_name = WM8962_CAPTURE_SWITCH,
        .intval = 1,
    },
    {
        .ctl_name = WM8962_CAPTURE_VOLUME,
        .intval = 63,
    },
    {
        .ctl_name = WM8962_DIGITAL_CAPTURE_VOLUME,
        .intval = 127,
    },
#if 0
    {
        .ctl_name = WM8962_MIXINR_IN3R_SWITCH,
        .intval = 1,
    },
    {
        .ctl_name = WM8962_MIXINR_IN3R_VOLUME,
        .intval = 7,
    },
#else
#if WM8962_MIXINL_PGA_VALUE
	{
		.ctl_name = WM8962_MIXINL_IN2L_SWITCH,
		.intval = 0,
	},
#else
	{
		.ctl_name = WM8962_MIXINL_IN2L_SWITCH,
		.intval = 1,
	},
#endif
	{
		.ctl_name = WM8962_MIXINL_PGA_SWITCH,
		.intval = WM8962_MIXINL_PGA_VALUE,
	},
#endif
#if 0 // WM8962_DEBUG
    {
        .ctl_name = WM8962_HPMIXL_MIXINL_SWITCH,
        .intval = 1,
    },
    {
        .ctl_name = WM8962_HPMIXL_MIXINR_SWITCH,
        .intval = 1,
    },
    {
        .ctl_name = WM8962_HEADPHONE_MIXER_SWITCH,
        .intval = 1,
    },
    {
        .ctl_name = WM8962_HPOUTL_PGA,
        .strval = WM8962_PGA_MUX_MIXER,
    },
    {
        .ctl_name = WM8962_HEADPHONE_SWITCH,
        .intval = 1,
    },
    {
        .ctl_name = WM8962_HEADPHONE_VOLUME,
        .intval = WM8962_HEADPHONE_VOLUME_VALUE,
    },
#endif
    {
        .ctl_name = NULL,
    },
};

static struct route_setting mm_bt_mic_input_wm8962[] = {
    {
        .ctl_name = NULL,
    },
};

static struct route_setting audio_mode_normal_wm8962[] = {
    {
        .ctl_name = WM8962_AUDIO_MODE,
        .strval = WM8962_AUDIO_MODE_MUSIC,
    },
#if WM8962_MIXINL_PGA_VALUE
	{
		.ctl_name = WM8962_INPGAL_IN4L_SWITCH,
		.intval = 0,
	},
	{
		.ctl_name = WM8962_HPMIXL_MIXINL_SWITCH,
		.intval = 0,
	},
	{
		.ctl_name = WM8962_SPKOUTL_MIXINL_SWITCH,
		.intval = 0,
	},
	{
		.ctl_name = WM8962_SPKOUTR_MIXINL_SWITCH,
		.intval = 0,
	},
#else
	{
		.ctl_name = WM8962_HPMIXL_IN4L_SWITCH,
		.intval = 0,
	},
	{
		.ctl_name = WM8962_SPKOUTL_IN4L_SWITCH,
		.intval = 0,
	},
	{
		.ctl_name = WM8962_SPKOUTR_IN4L_SWITCH,
		.intval = 0,
	},
#endif
    {
        .ctl_name = NULL,
    },
};

static struct route_setting audio_mode_incall_wm8962[] = {
#if WM8962_MIXINL_PGA_VALUE
	{
		.ctl_name = WM8962_INPGAL_IN4L_SWITCH,
		.intval = 1,
	},
	{
		.ctl_name = WM8962_HPMIXL_MIXINL_SWITCH,
		.intval = 1,
	},
	{
		.ctl_name = WM8962_SPKOUTL_MIXINL_SWITCH,
		.intval = 1,
	},
	{
		.ctl_name = WM8962_SPKOUTR_MIXINL_SWITCH,
		.intval = 1,
	},
#else
	{
		.ctl_name = WM8962_HPMIXL_IN4L_SWITCH,
		.intval = 1,
	},
	{
		.ctl_name = WM8962_SPKOUTL_IN4L_SWITCH,
		.intval = 1,
	},
	{
		.ctl_name = WM8962_SPKOUTR_IN4L_SWITCH,
		.intval = 1,
	},
#endif
    {
        .ctl_name = WM8962_AUDIO_MODE,
        .strval = WM8962_AUDIO_MODE_CALL,
    },
    {
        .ctl_name = NULL,
    },
};

/* ALSA cards for IMX, these must be defined according different board / kernel config*/
static struct audio_card  wm8962_card = {
    .name = "wm8962-audio",
    .driver_name = "wm8962-audio",
    .supported_out_devices = (AUDIO_DEVICE_OUT_EARPIECE |
            AUDIO_DEVICE_OUT_SPEAKER |
            AUDIO_DEVICE_OUT_WIRED_HEADSET |
            AUDIO_DEVICE_OUT_WIRED_HEADPHONE |
            AUDIO_DEVICE_OUT_ANLG_DOCK_HEADSET |
            AUDIO_DEVICE_OUT_ALL_SCO |
            AUDIO_DEVICE_OUT_DEFAULT ),
    .supported_in_devices = (
            AUDIO_DEVICE_IN_COMMUNICATION |
            AUDIO_DEVICE_IN_AMBIENT |
            AUDIO_DEVICE_IN_BUILTIN_MIC |
            AUDIO_DEVICE_IN_WIRED_HEADSET |
            AUDIO_DEVICE_IN_BACK_MIC |
            AUDIO_DEVICE_IN_ALL_SCO |
            AUDIO_DEVICE_IN_DEFAULT),
    .defaults            = defaults_wm8962,
    .bt_output           = bt_output_wm8962,
    .speaker_output      = speaker_output_wm8962,
    .hs_output           = hs_output_wm8962,
    .earpiece_output     = earpiece_output_wm8962,
    .vx_hs_mic_input     = vx_hs_mic_input_wm8962,
    .mm_main_mic_input   = mm_main_mic_input_wm8962,
    .vx_main_mic_input   = vx_main_mic_input_wm8962,
    .mm_hs_mic_input     = mm_hs_mic_input_wm8962,
    .vx_bt_mic_input     = vx_bt_mic_input_wm8962,
    .mm_bt_mic_input     = mm_bt_mic_input_wm8962,
    .audio_modes         = {
		audio_mode_normal_wm8962,
		audio_mode_normal_wm8962,
		audio_mode_incall_wm8962,
		audio_mode_normal_wm8962,
    },
    .card                = 0,
    .out_rate            = 0,
    .out_channels        = 0,
    .out_format          = 0,
    .in_rate             = 0,
    .in_channels         = 0,
    .in_format           = 0,
};

#endif  /* ANDROID_INCLUDE_IMX_CONFIG_WM8962_H */
