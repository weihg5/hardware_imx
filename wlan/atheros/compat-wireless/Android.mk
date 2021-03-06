#------------------------------------------------------------------------------
# <copyright file="makefile" company="Atheros">
# Copyright (c) 2005-2010 Atheros Corporation.  All rights reserved.
# Copyright (c)2010-2012 Freescale Semiconductor, Inc.
#
# Permission to use, copy, modify, and/or distribute this software for any
# purpose with or without fee is hereby granted, provided that the above
# copyright notice and this permission notice appear in all copies.
#
# THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
# WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
# MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
# ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
# WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
# ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
# OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
#
#
#------------------------------------------------------------------------------
#==============================================================================
# Author(s): ="Atheros"
#==============================================================================

ifneq ($(TARGET_SIMULATOR),true)

LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

ATH_ANDROID_SRC_BASE:= $(LOCAL_PATH)

ATH_ANDROID_ROOT:= $(CURDIR)

ATH_LINUXPATH=$(ATH_ANDROID_ROOT)/kernel_imx
ATH_CROSS_COMPILE=$(ATH_ANDROID_ROOT)/prebuilts/gcc/linux-x86/arm/arm-eabi-4.6/bin/arm-eabi-

mod_cleanup := $(ATH_ANDROID_ROOT)/$(ATH_ANDROID_SRC_BASE)/dummy

$(mod_cleanup) :
	$(MAKE) -C $(ATH_ANDROID_SRC_BASE) ARCH=arm CROSS_COMPILE=$(ATH_CROSS_COMPILE) KLIB=$(ATH_LINUXPATH) KLIB_BUILD=$(ATH_LINUXPATH) COMPAT_CURDIR=$(ATH_ANDROID_ROOT)/$(ATH_ANDROID_SRC_BASE) clean
	mkdir -p $(TARGET_OUT)/etc/firmware/ath6k/AR6003/hw2.1.1/
	mkdir -p $(TARGET_OUT)/lib/modules/

ath6kl_module_file :=drivers/net/wireless/ath/ath6kl/ath6kl_sdio.ko
$(ATH_ANDROID_SRC_BASE)/$(ath6kl_module_file):$(mod_cleanup) $(TARGET_PREBUILT_KERNEL) $(ACP)
	$(MAKE) -C $(ATH_ANDROID_SRC_BASE) O=$(ATH_LINUXPATH) ARCH=arm CROSS_COMPILE=$(ATH_CROSS_COMPILE) KLIB=$(ATH_LINUXPATH) KLIB_BUILD=$(ATH_LINUXPATH) COMPAT_CURDIR=$(ATH_ANDROID_ROOT)/$(ATH_ANDROID_SRC_BASE)
	$(ACP) -fpt $(ATH_ANDROID_SRC_BASE)/compat/compat.ko $(TARGET_OUT)/lib/modules/
	$(ACP) -fpt $(ATH_ANDROID_SRC_BASE)/net/wireless/cfg80211.ko $(TARGET_OUT)/lib/modules/

include $(CLEAR_VARS)
LOCAL_MODULE := ath6kl_sdio.ko
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_CLASS := ETC
LOCAL_MODULE_PATH := $(TARGET_OUT)/lib/modules
LOCAL_SRC_FILES := $(ath6kl_module_file)
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := check_wifi_mac.sh
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_CLASS := EXECUTABLES
LOCAL_MODULE_PATH := $(TARGET_OUT)/etc
LOCAL_SRC_FILES := check_wifi_mac.sh
include $(BUILD_PREBUILT)

endif
