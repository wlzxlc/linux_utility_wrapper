# ----------------------------------------------------
# Auto genarate templements
# Author : lichao@kedacom.com
# Time   :Tue Mar 22 14:52:49 CST 2016
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

ROOT := $(LOCAL_PATH)

# Delete out and release directory.
.PHONY : project_clean

project_clean:
	@rm -rf $(APP_OUTPUT_DIR)
	@rm -rf $(APP_RELEASE_DIR)

$(call add-module-clean,,project_clean)

include $(call all-subdir-makefiles)
