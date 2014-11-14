#ifndef favor_jni_exception_include
#define favor_jni_exception_include
#include <jni.h>
#include "../favor.h"

#define jniExcept(x) try {\
    x\
}\
catch (favor::exception &e){\
    e.jniException(env);\
}\

#define jniExceptReturnNull(x) try {\
    x\
}\
catch (favor::exception &e){\
    e.jniException(env);\
    return NULL;\
}\

namespace favor{
    jint exception::jniException(JNIEnv* env){
        return env->ThrowNew(jni::favor_exception, what());
    }

    class jniException : public exception{
    public:
        jniException(const std::string &e) : exception(e) {}

        jniException() : exception("An operation across the JNI boundary has failed") {}
    };
}





#endif