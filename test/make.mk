# ----------------------------------------------------
# Auto genarate templements
# Author : lichao@keacom.com
# Time   :Tue Mar 22 16:36:31 CST 2016
# ----------------------------------------------------
# Always to point an absolute path of the this make.mk
LOCAL_PATH := $(call my-dir)

# These variables by script auto genarate. In order 
# to reduce the current make writing burden. if you 
# are have any question and to see:
# $(sysbuild_root)/docs/A&Q.txt. 
# About to usage of the this make.mk, you are can 
# to see :
# $(sysbuild_root)/docs/make_mk.txt
 
 
# Include others make.mk
# $(call include-makefiles, /foo/make.mk /boo/make.mk)
 
 
 
###################Module 'linux_utility' begin####################
include $(CLEAR_VARS)
# Declare module name
LOCAL_MODULE := linux_utility_test_log
# LOCAL_MODULE_FILENAME :=

LOCAL_SRC_FILES := test_log.cpp

LOCAL_LINK_MODE := c++
LOCAL_C_INCLUDES := $(LOCAL_PATH) 

LOCAL_STATIC_LIBRARIES := linux_utility

ifeq ($(TARGET_PLATFORM),android)
 LOCAL_LDFLAGS := -pie -fPIE
endif

ifdef TARGET_RELEASE_DIR
# LOCAL_RELEASE_PATH := $(TARGET_RELEASE_DIR)/....
endif
include $(BUILD_TEST)

include $(call all-subdir-makefiles)
