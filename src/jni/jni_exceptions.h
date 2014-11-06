#ifndef favor_jni_exception_include
#define favor_jni_exception_include
#include <jni.h>
#include "../favor.h"

#define jniExcept(x) try {\
    x\
}\
catch (favor::exception &e){\
    throwFavorException(env, e.what());\
}\

#define jniExceptReturnNull(x) try {\
    x\
}\
catch (favor::exception &e){\
    throwFavorException(env, e.what());\
    return NULL;\
}\

namespace favor{
    namespace jni{
        jint throwFavorException(JNIEnv* env, const char* message){
            jclass exClass = env->FindClass("com/favor/library/FavorException");
            if (exClass == NULL) {
                //TODO: we're in trouble now. maybe fall back on some more generic runtime exception?
            }
            return env->ThrowNew(exClass, message);
        }


    }
}





#endif