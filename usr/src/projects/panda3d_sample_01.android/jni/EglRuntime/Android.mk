LOCAL_PATH := $(call my-dir)
include $(call my-dir)/../../board.tmp



# Include libpython3.5m.so

include $(CLEAR_VARS)
LOCAL_MODULE    := python3.5m
LOCAL_SRC_FILES := $(CRYSTAX_PATH)/sources/python/3.5/libs/$(TARGET_ARCH_ABI)/libpython3.5m.so
LOCAL_EXPORT_CFLAGS := -I $(CRYSTAX_PATH)/sources/python/3.5/include/python/
include $(PREBUILT_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := pandagles
LOCAL_SRC_FILES := ../../prebuilt/armeabi-v7a/libpandagles.so
LOCAL_EXPORT_CFLAGS := -I/usr/include/panda3d
include $(PREBUILT_SHARED_LIBRARY)


include $(CLEAR_VARS)
LOCAL_MODULE    := eglruntime
LOCAL_SRC_FILES := jniapi.cpp
#LOCAL_CFLAGS := -L/data/data/u.root/usr/src/projects/panda3d_sample_00.android/prebuilt/armeabi-v7a
LOCAL_LDLIBS := -lz -ldl -lc -lm -llog -lEGL -landroid -lGLESv1_CM
LOCAL_SHARED_LIBRARIES := python3.5m crystax pandagles
include $(BUILD_SHARED_LIBRARY)


