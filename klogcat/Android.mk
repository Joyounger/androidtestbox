LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_SRC_FILES := klogcat.cpp
LOCAL_SHARED_LIBRARIES := liblog libm libc libprocessgroup libcutils libbase
LOCAL_CFLAGS := -Werror

LOCAL_MODULE := klogcat

include $(BUILD_EXECUTABLE)
