LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)
LOCAL_MODULE := favor
LOCAL_CPP_FEATURES += exceptions
LOCAL_SRC_FILES := ../lib/sqlite/sqlite3.c ../favor.cpp ../processor.cpp ../reader.cpp ../worker.cpp ../accountmanager.cpp ../logger.cpp ../managers/emailmanager.cpp
LOCAL_SRC_FILES += jni.cpp
include $(BUILD_SHARED_LIBRARY)