#ifndef favor_exception_include
#define favor_exception_include

#include <exception>
#include <stdexcept>

namespace favor {
    class exception : public std::runtime_error {
    protected:
        exception(const std::string &e) : runtime_error(e) {
        }
    };

    //TODO: get some exception inheritance going on; network and authentication can inherit from the same thing, as can
    //bad user data and bad message data.
    class sqliteException : public exception {
    public:
        sqliteException(const std::string &e) : exception(e) {
        }

        sqliteException() : exception("An internal SQLite3 operation has failed") {
        }
    };

    class networkConnectionException : public exception {
    public:
        networkConnectionException(const std::string &e) : exception(e) {
        }

        networkConnectionException() : exception("Failed to perform normal network operations") {
        }
    };

    class authenticationException : public exception {
    public:
        authenticationException(const std::string &e) : exception(e) {
        }

        authenticationException() : exception("Failed to login with credentials provided") {
        }
    };

    class badUserDataException : public exception {
    public:
        badUserDataException(const std::string &e) : exception(e) {
        }

        badUserDataException() : exception("Bad account, contact or address data") {
        }
    };

    class badMessageDataException : public exception {
    public:
        badMessageDataException(const std::string &e) : exception(e) {
        }

        badMessageDataException() : exception("Unhandleable message data received") {
        }
    };

    class threadingException : public exception {
    public:
        threadingException(const std::string &e) : exception(e){
        }

        threadingException(): exception("Misuse of favor threading internals"){

        }
    };

#ifdef FAVOR_EMAIL_MANAGER

    class emailException : public exception {
    public:
        emailException(const std::string &e) : exception(e) {
        }

        emailException() : exception("Error interfacing with email server") {
        }
    };

#endif
}
#endif