#ifndef favor_jni_globals_include
#define favor_jni_globals_include
#include <jni.h>
#include "../definitions.h"

namespace favor{
    namespace jni{
        const char* coreClassPath = CLASS_PATH "Core";
        const char* readerClassPath = CLASS_PATH "Reader";
        const char* workerClassPath = CLASS_PATH "Worker";
        const char* accountManagerClassPath = CLASS_PATH "AccountManager";
        const char* androidTextManagerClassPath = CLASS_PATH "AndroidTextManager";

        jclass account_manager;
        jmethodID account_manager_constructor;
        jclass android_text_manager;
        jmethodID android_text_manager_constructor;
        jclass favor_exception;
        jclass java_string;
    }
}

#endif