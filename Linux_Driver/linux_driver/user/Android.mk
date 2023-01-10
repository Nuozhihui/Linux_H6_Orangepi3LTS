LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_MODULE := user_hello
LOCAL_SRC_FILES := user_hello.c
LOCAL_ARM_MODE := arm
LOCAL_CFLAGS += -pie -FPIE
LOACAL_LDFLAGS += -pie -fPIE

include $(BUILD_EXECUTABLE)

