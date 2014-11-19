LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)
LOCAL_MODULE := favor
LOCAL_CPP_FEATURES += exceptions
LOCAL_CPPFLAGS := -DSQLITE_THREADSAFE=2
LOCAL_C_INCLUDES := ../ ../managers ../lib ../messagequeries
LOCAL_SRC_FILES := ../lib/sqlite/sqlite3.c ../lib/pugixml/pugixml.cpp ../favor.cpp ../processor.cpp ../reader.cpp ../worker.cpp ../accountmanager.cpp ../logger.cpp ../message.cpp
LOCAL_SRC_FILES += ../contact.cpp ../managers/androidtextmanager.cpp ../messagequeries/messagequery.cpp
LOCAL_SRC_FILES += jni.cpp

LOCAL_LDLIBS := -llog 
#include <android/log.h>

include $(BUILD_SHARED_LIBRARY)