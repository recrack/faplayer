
LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_ARM_MODE := arm

LOCAL_MODULE := arm_neon

LOCAL_SRC_FILES := \
    memcpy.S

LOCAL_CFLAGS += $(COMMON_OPT_CFLAGS)
LOCAL_LDFLAGS += $(COMMON_OPT_LDFLAGS)

include $(BUILD_STATIC_LIBRARY)
