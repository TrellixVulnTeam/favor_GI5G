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



#ifndef favor_exception_include
#define favor_exception_include

#include <exception>
#include <stdexcept>

#ifdef ANDROID
#include <jni.h>
#endif

namespace favor {
    class exception : public std::runtime_error {
    protected:
        exception(const std::string &e) : runtime_error(e) {
        }
#ifdef ANDROID
    public:
        //The method can eventually be virtual and child classes can overwrite it if necessary
        jint jniException(JNIEnv* env);
#endif
    };

    //TODO: get some exception inheritance going on; network and authentication can inherit from the same thing, as can
    //bad user data and bad message data.
    class sqliteException : public exception {
    public:
        sqliteException(const std::string &e) : exception(e) {}

        sqliteException() : exception("An internal SQLite3 operation has failed") {}
    };

    class constraintViolationException : public sqliteException {
    public:
        constraintViolationException(const std::string &e) : sqliteException(e) {}

        constraintViolationException() : sqliteException("An SQLite3 operation has failed because of database constraint violations"){}
    };

    class networkConnectionException : public exception {
    public:
        networkConnectionException(const std::string &e) : exception(e) {}

        networkConnectionException() : exception("Failed to perform normal network operations") {}
    };

    class authenticationException : public networkConnectionException {
    public:
        authenticationException(const std::string &e) : networkConnectionException(e) {}

        authenticationException() : networkConnectionException("Failed to login with credentials provided") {}
    };

    class badUserDataException : public exception {
    public:
        badUserDataException(const std::string &e) : exception(e) {}

        badUserDataException() : exception("Bad account, contact or address data") {}
    };

    class badMessageDataException : public exception {
    public:
        badMessageDataException(const std::string &e) : exception(e) {}

        badMessageDataException() : exception("Unhandleable message data received") {}
    };

    class queryException : public exception{
    public:
        queryException(const std::string& e) : exception(e) {}
        queryException(): exception("Attempted to construct a database query with invalid input"){}
    };

    class threadingException : public exception {
    public:
        threadingException(const std::string &e) : exception(e){}

        threadingException(): exception("Misuse of favor threading internals"){}
    };

#ifdef FAVOR_EMAIL_MANAGER

    class emailException : public networkConnectionException {
    public:
        emailException(const std::string &e) : networkConnectionException(e) {}

        emailException() : networkConnectionException("Error interfacing with email server") {}
    };

#endif
}
#endif