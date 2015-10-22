ifeq ($(TARGET_ARCH),arm)

LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

#
# UIM Application
#


LOCAL_SRC_FILES:= \
	wireless.c
LOCAL_CFLAGS:= -g -c -W -Wall -O2 -D_POSIX_SOURCE -D_ANDROID
LOCAL_SHARED_LIBRARIES:= libnetutils libcutils liblog
LOCAL_MODULE:=wireless_ppt
LOCAL_MODULE_TAGS:= eng
include $(BUILD_EXECUTABLE)

endif
