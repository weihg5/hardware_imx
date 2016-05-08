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

#ifdef MODEM_EC20
#define WM8962_AUDIO_MODE_CALL                "Call"
#else
#define WM8962_AUDIO_MODE_CALL                "SlaveCall"
#endif

#define WM8962_AUDIO_MODE_BT_MUSIC            "BtSlaveMusic"
#define WM8962_AUDIO_MODE_BT_CALL             "BtSlaveCall"

#define WM8962_PGA_MUX_DAC                    "DAC"
#define WM8962_PGA_MUX_MIXER                  "Mixer"
#define WM8962_SPEAKER_JACK_SWITCH            "Speaker Jack Switch"
#define WM8962_EARPIECE_JACK_SWITCH           "Earpiece Jack Switch"

#define WM8962_SPEAKER_VOLUME                 "Speaker Volume"
#define WM8962_SPEAKER_VOLUME_VALUE           121
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
#define WM8962_MIXINL_PGA_VALUE               0

#define WM8962_DIGITAL_CAPTURE_VOLUME         "Digital Capture Volume"

#define WM8962_DIGITAL_PLAYBACK_VOLUME        "Digital Playback Volume"
#define WM8962_INPGAL_IN4L_SWITCH             "INPGAL IN4L Switch"
#define WM8962_HPMIXL_MIXINL_SWITCH           "HPMIXL MIXINL Switch"
#define WM8962_INPGAL_IN1L_SWITCH            "INPGAL IN1L Switch"
#define WM8962_INPGAR_IN1R_SWITCH            "INPGAR IN1R Switch"
/* These are values that never change */
static struct route_setting defaults_wm8962[] = {
    /* general */
    {
        .ctl_name = WM8962_DIGITAL_PLAYBACK_VOLUME,
        .intval = 96,
    },
    {
		.ctl_name = WM8962_INPGAL_IN1L_SWITCH,
		.intval = 0,
	},
	{
		.ctl_name = WM8962_INPGAR_IN1R_SWITCH,
		.intval = 0,
	},
    {
        .ctl_name = NULL,
    },
};

static struct route_setting wm8962_ppt_mode[] = {
    {
        .ctl_name = "Capture SwitchL",
        .intval = 1,
    },
    {
        .ctl_name = "Capture VolumeL",
        .intval = 60,
    },
	
	{
		.ctl_name = "INPGAL IN2L Switch",
		.intval = 1,
	},
    {
        .ctl_name = "MIXINL PGA Switch",
        .intval = 1,
    },
	{
		.ctl_name = "HPMIXR MIXINL Switch",
		.intval = 1,
	},
	{
		.ctl_name = "HPOUTR PGA",
		.strval = "Mixer"
	},
	{
		.ctl_name = "Headphone MixerR Switch",
		.intval = 1,
	},

    {
		.ctl_name = "Headphone SwitchR",
		.intval = 1,
	},
	{
		.ctl_name = "Earpiece Jack Switch",
		.intval = 1,
	},
	{
		.ctl_name = "Headphone VolumeR",
		.intval = 115,
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

static struct route_setting call_speek_output_wm8962[] = {

	{
		.ctl_name = WM8962_SPKOUTL_IN4L_SWITCH,
		.intval = 1,
	},
	{
		.ctl_name = WM8962_SPKOUTR_IN4L_SWITCH,
		.intval = 1,
	},
#if 0
	{
		.ctl_name = WM8962_SPEAKER_SWITCH,
		.intval = 1,
	},
	{
		.ctl_name = WM8962_SPEAKER_VOLUME,
		.intval = WM8962_SPEAKER_VOLUME_VALUE,
	},
	{
		.ctl_name = WM8962_SPEAKER_MIXER_SWITCH,
		.intval = 1,
	},
	{
		.ctl_name = WM8962_SPEAKER_JACK_SWITCH,
		.intval = 1,
	},
#endif
	{
		.ctl_name = NULL,
	},
};


static struct route_setting call_earpiece_output_wm8962[] = {
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
		.ctl_name = WM8962_HPMIXL_IN4L_SWITCH,
		.intval = 1,
	},
	{
		.ctl_name = "Headphone MixerL Switch",
		.intval = 1,
	},

	{
		.ctl_name = "Headphone SwitchL",
		.intval = 1,
	},
	{
		.ctl_name = WM8962_HPOUTL_PGA,
		.strval = WM8962_PGA_MUX_MIXER,
	},
	{
		.ctl_name = "Headphone VolumeL",
		.intval = WM8962_HEADPHONE_VOLUME_VALUE,
	},
#ifdef MODEM_MC9090
	{
		.ctl_name = WM8962_HPMIXL_DACL_SWITCH,
		.intval = 1,
	},
#endif
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
#ifdef MODEM_EC20
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
        .intval = 50,
    },
    {
        .ctl_name = WM8962_DIGITAL_CAPTURE_VOLUME,
        .intval = 110,
    },
    {
        .ctl_name = WM8962_INPGAL_IN2L_SWITCH,
        .intval = 1,
    },
    {
        .ctl_name = WM8962_MIXINL_PGA_SWITCH,
        .intval = 1,
    },
    {
        .ctl_name = WM8962_MIXINL_PGA_VOLUME,
        .intval = 50,
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
//#else
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
#ifdef MODEM_EC20
    {
        .ctl_name = "Capture SwitchL",
        .intval = 1,
    },
    {
        .ctl_name = "Capture VolumeL",
        .intval = 55,
    },

    {	.ctl_name = "INPGAL IN2L Switch",
    	.intval = 1,
    },
    {	.ctl_name = "MIXINL PGA Switch",
    	.intval = 1,    
    },    
    {
    	.ctl_name = "HPMIXR MIXINL Switch",
		.intval = 1,
	},    
    {
    	.ctl_name = "HPOUTR PGA",
		.strval = "Mixer",
	},    
    {
    	.ctl_name = "Headphone MixerR Switch",
		.intval = 1,
	},    
    {
    	.ctl_name = "Headphone SwitchR",
		.intval = 1,
	},    
    {
    	.ctl_name = "Earpiece Jack Switch",
		.intval = 1,
	},    
    {
    	.ctl_name = "Headphone VolumeR",
		.intval = 100,
	},
#else
    {
        .ctl_name = WM8962_DIGITAL_CAPTURE_VOLUME,
        .intval = 127,
    },

	{
		.ctl_name = WM8962_MIXINL_IN2L_SWITCH,
		.intval = 0,
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
#ifdef MODEM_EC20
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
#if 0	
	{
		.ctl_name = WM8962_SPKOUTL_IN4L_SWITCH,
		.intval = 1,
	},
	{
		.ctl_name = WM8962_SPKOUTR_IN4L_SWITCH,
		.intval = 1,
	},
#endif
#endif
    {
        .ctl_name = WM8962_AUDIO_MODE,
        .strval = WM8962_AUDIO_MODE_CALL,
    },
    {
        .ctl_name = NULL,
    },
};

static struct route_setting audio_mode_bt_incall_wm8962[] = {
    {
        .ctl_name = WM8962_AUDIO_MODE,
        .strval = WM8962_AUDIO_MODE_BT_CALL,
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
    .earpiece_output     = call_earpiece_output_wm8962,
    .call_speaker_output = call_speek_output_wm8962,
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
    .bt_incall_mode      = audio_mode_bt_incall_wm8962,
    .card                = 0,
    .out_rate            = 0,
    .out_channels        = 0,
    .out_format          = 0,
    .in_rate             = 0,
    .in_channels         = 0,
    .in_format           = 0,
};

#endif  /* ANDROID_INCLUDE_IMX_CONFIG_WM8962_H */
