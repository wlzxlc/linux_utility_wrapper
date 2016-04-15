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
 
 
 
LOCAL_SRC_FILES += gl/GLES11RenderEngine.cpp \
                   gl/Mesh.cpp \
                   gl/ProgramCache.cpp \
                   gl/Rect.cpp \
                   gl/Texture.cpp \
                   gl/Transform.cpp \
                   gl/RenderEngine.cpp \
                   gl/Region.cpp \
                   gl/GLExtensions.cpp \
                   gl/Description.cpp \
                   gl/Program.cpp \
                   gl/GLES20RenderEngine.cpp \
                   gl/GLES10RenderEngine.cpp \

LOCAL_LINK_MODE := c++
LOCAL_CFLAGS += -DGL_GLEXT_PROTOTYPES -Wno-enum-compare
LOCAL_CPPFLAGS += -D__STDC_FORMAT_MACROS
LOCAL_EXPORT_LDLIBS += -lGLESv2 -lEGL -lGLESv1_CM

LOCAL_C_INCLUDES += $(LOCAL_PATH) 

