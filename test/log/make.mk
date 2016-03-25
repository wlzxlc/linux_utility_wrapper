# ----------------------------------------------------
# Auto genarate templements
# Author : lichao@keacom.com
# Time   :2016年 03月 08日 星期二 20:21:56 CST
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
include $(CLEAR_VARS)
LOCAL_MODULE := log_test

LOCAL_SRC_FILES := log.cpp

LOCAL_LINK_MODE := c++

LOCAL_C_INCLUDES := $(LOCAL_PATH) 

LOCAL_STATIC_LIBRARIES := linux_utility
include $(BUILD_TEST)
