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