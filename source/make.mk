# ----------------------------------------------------
# Auto genarate templements
# Author : lichao@keacom.com
# Time   :Tue Mar 22 14:38:44 CST 2016
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
LOCAL_SRC_FILES += ./log.c

#include telnetd
LOCAL_SRC_FILES += telnetd/ospsock.c \
				   telnetd/osptele.c

LOCAL_CFLAGS := -D_LINUX_\
	            -UNDEBUG \
         	    -Wno-pointer-to-int-cast 

LOCAL_C_INCLUDES := $(ROOT)/include

LOCAL_MODULE := linux_utility
LOCAL_MODULE_FILENAME := liblinux_utility

LOCAL_EXPORT_C_INCLUDES := $(ROOT)/include

ifeq ($(TARGET_PLATFORM),android)

ifdef ANDROID_LOG
 LOCAL_CFLAGS += -DANDROID_LOG
 LOCAL_EXPORT_LDLIBS := -llog
endif
endif

ifdef TARGET_RELEASE_DIR
 LOCAL_RELEASE_PATH := $(TARGET_RELEASE_DIR)/libs
endif
include $(BUILD_STATIC_LIBRARY)
