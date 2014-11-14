#ifndef favor_jni_string_include
#define favor_jni_string_include
#include <jni.h>
#include <string>
#include "jni_globals.h"
#include "jni_exceptions.h"

//TODO: we'll need to test this explicitly at some point, make sure it's doing its job of not leaking memory

namespace favor{
    namespace jni{
        class JNIString {
            JNIString() = delete;
            NONCOPYABLE(JNIString)
        private:
            const char* jniChars;
            string str;
            bool nullStr;
            jstring jstr;
            jboolean copied;
            JNIEnv* env;

            void setNull(){
                jniChars = NULL;
                str = "";
                nullStr = true;
            }

        public:

            JNIString(JNIEnv* environment, jstring stringRef) : env(environment), jstr(stringRef) {
                if (jstr == NULL){
                    copied = false;
                    setNull();
                } else{
                    jniChars = env->GetStringUTFChars(jstr, &copied);
                    if (jniChars == NULL){
                        setNull();
                        throw jniException("Could not get string UTF chars");
                    } else{
                        str = string(jniChars);
                        nullStr = false;
                    }
                }
            }


            bool isNull() const {
                return nullStr;
            }

            bool isCopied() const {
                return copied;
            }

            void invalidate(){
                if (nullStr) return;
                env->ReleaseStringUTFChars(jstr, jniChars);
                setNull();
            }

            void deleteRefAndInvalidate(){
                invalidate();
                env->DeleteLocalRef(jstr);
            }

            std::string getString() const{
                if (nullStr) throw jniException("Attempt to use null JNIString");
                return str;
            }

            operator std::string() const{
                if (nullStr) throw jniException("Attempt to use null JNIString");
                else return str;
            }

            ~JNIString(){
                invalidate();
            }
        };
    }
}

#endif