#ifndef favor_jni_exception_include
#define favor_jni_exception_include
#include <jni.h>
#include "../favor.h"

namespace favor{
    namespace jni{
        jint throwFavorException(JNIEnv* env, char* message){
            jclass exClass = env->FindClass("com/favor/library/FavprException");
            if (exClass == NULL) {
                //TODO: we're in trouble now. maybe fall back on some more generic runtime exception?
            }
            return env->ThrowNew(exClass, message);
        }


    }
}





#endif