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

TEST_MODULE :=

ifdef HAVE_LOG
  TEST_MODULE += log
endif

ifdef HAVE_OPENGLES
  TEST_MODULE += gl
endif

ifdef HAVE_TELNETD
 TEST_MODULE += telnetd
endif

ifdef HAVE_THREAD
 TEST_MODULE += thread
endif

$(foreach module, $(TEST_MODULE), \
	$(eval $(call include-makefile, \
	$(LOCAL_PATH)/$(module)/make.mk)))

