LOCAL_PATH := $(call my-dir)

##############################################################################
include $(CLEAR_VARS)

LOCAL_MODULE := Regal_static
LOCAL_SRC_FILES := libRegal_static.a

include $(PREBUILT_STATIC_LIBRARY)
##############################################################################
include $(CLEAR_VARS)

LOCAL_MODULE := dreamtri
LOCAL_SRC_FILES := main.cpp
LOCAL_C_INCLUDES += $(LOCAL_PATH)/include
LOCAL_LDLIBS := -llog -landroid
LOCAL_WHOLE_STATIC_LIBRARIES := Regal_static
LOCAL_STATIC_LIBRARIES := android_native_app_glue

include $(BUILD_SHARED_LIBRARY)

$(call import-module,android/native_app_glue)
