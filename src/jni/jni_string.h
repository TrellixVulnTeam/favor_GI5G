/*
Copyright (C) 2015  Joshua Tanner (mindful.jt@gmail.com)

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/



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