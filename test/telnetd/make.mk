LOCAL_PATH := $(call my-dir)
# Build test
include $(CLEAR_VARS)
LOCAL_MODULE := telnetd_test
LOCAL_SRC_FILES := telnetd.c
LOCAL_CFLAGS := -DTELNETD_PORT=3389
LOCAL_STATIC_LIBRARIES := linux_utility
ifeq ($(TARGET_PLATFORM),android)
 LOCAL_LDFLAGS := -pie -fPIE
endif
include $(BUILD_TEST)

include $(CLEAR_VARS)
LOCAL_MODULE := telnetd_c++_test
LOCAL_SRC_FILES :=  telnetd_wrapper.cc testc++.cc
LOCAL_CFLAGS := -DTELNETD_PORT=3389 -UNDEBUG -D_LINUX_

LOCAL_STATIC_LIBRARIES := linux_utility

LOCAL_LINK_MODE := c++
ifeq ($(TARGET_PLATFORM),android)
 LOCAL_LDFLAGS := -pie -fPIE
endif
include $(BUILD_TEST)
