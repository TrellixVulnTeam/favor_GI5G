LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)
LOCAL_MODULE := favor
LOCAL_CPP_FEATURES += exceptions
LOCAL_C_INCLUDES := ../ ../managers ../lib
LOCAL_SRC_FILES := ../lib/sqlite/sqlite3.c ../lib/pugixml/pugixml.cpp ../favor.cpp ../processor.cpp ../reader.cpp ../worker.cpp ../accountmanager.cpp ../logger.cpp ../message.cpp ../contact.cpp ../managers/androidtextmanager.cpp
LOCAL_SRC_FILES += jni.cpp

LOCAL_LDLIBS := -llog 
#include <android/log.h>

include $(BUILD_SHARED_LIBRARY)