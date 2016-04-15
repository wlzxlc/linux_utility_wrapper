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
LOCAL_CFLAGS += -UNDEBUG -Werror
LOCAL_C_INCLUDES += $(ROOT)/include

ifdef HAVE_LOG
LOCAL_SRC_FILES += log.c
endif

ifdef HAVE_THREAD
  LOCAL_EXPORT_CFLAGS += -DHAVE_THREAD
endif

ifdef HAVE_TELNETD
LOCAL_SRC_FILES += telnetd/ospsock.c
LOCAL_SRC_FILES += telnetd/osptele.c
LOCAL_CFLAGS += -D_LINUX_
endif

ifdef HAVE_OPENGLES
LOCAL_SRC_FILES += gl/GLES11RenderEngine.cpp
LOCAL_SRC_FILES += gl/Mesh.cpp
LOCAL_SRC_FILES += gl/ProgramCache.cpp
LOCAL_SRC_FILES += gl/Rect.cpp
LOCAL_SRC_FILES += gl/Texture.cpp
LOCAL_SRC_FILES += gl/Transform.cpp
LOCAL_SRC_FILES += gl/RenderEngine.cpp
LOCAL_SRC_FILES += gl/Region.cpp
LOCAL_SRC_FILES += gl/GLExtensions.cpp
LOCAL_SRC_FILES += gl/Description.cpp
LOCAL_SRC_FILES += gl/Program.cpp
LOCAL_SRC_FILES += gl/GLES20RenderEngine.cpp
LOCAL_SRC_FILES += gl/GLES10RenderEngine.cpp
LOCAL_CFLAGS += -DGL_GLEXT_PROTOTYPES
LOCAL_CPPFLAGS += -D__STDC_FORMAT_MACROS
LOCAL_CFLAGS += -Wno-enum-compare
LOCAL_EXPORT_LDLIBS += -lGLESv2 
LOCAL_EXPORT_LDLIBS += -lEGL
LOCAL_EXPORT_LDLIBS += -lGLESv1_CM 
LOCAL_EXPORT_CFLAGS += -DGL_GLEXT_PROTOTYPES
LOCAL_EXPORT_CFLAGS += -DEGL_EGLEXT_PROTOTYPES
LOCAL_C_INCLUDES += $(LOCAL_PATH)/gl 
endif

LOCAL_MODULE := linux_utility
LOCAL_MODULE_FILENAME := liblinux_utility

LOCAL_EXPORT_C_INCLUDES += $(ROOT)/include
LOCAL_EXPORT_CFLAGS += -Werror

ifeq ($(TARGET_PLATFORM),android)
 ifdef ANDROID_LOG
  LOCAL_CFLAGS += -DANDROID_LOG
  LOCAL_EXPORT_LDLIBS := -llog
 endif
else
 ifdef HAVE_THREAD
  LOCAL_EXPORT_LDLIBS += -pthread
 endif
endif

ifdef TARGET_RELEASE_DIR
 LOCAL_RELEASE_PATH := $(TARGET_RELEASE_DIR)/libs
endif
include $(BUILD_STATIC_LIBRARY)
