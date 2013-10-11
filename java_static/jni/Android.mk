LOCAL_PATH:= $(call my-dir)

##############################################################################
include $(CLEAR_VARS)

LOCAL_MODULE := Regal_static
LOCAL_SRC_FILES := libRegal_static.a

include $(PREBUILT_STATIC_LIBRARY)
##############################################################################
include $(CLEAR_VARS)

LOCAL_MODULE    := libdreamtri
LOCAL_SRC_FILES := gl_code.cpp
LOCAL_C_INCLUDES += $(LOCAL_PATH)/include 
LOCAL_LDLIBS    := -llog
LOCAL_WHOLE_STATIC_LIBRARIES := Regal_static

include $(BUILD_SHARED_LIBRARY)
