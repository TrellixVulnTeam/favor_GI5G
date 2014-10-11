#ifndef favor_exception_include
#define favor_exception_include

#include <exception>
#include <stdexcept>

namespace favor {
    class exception : public std::runtime_error {
    protected:
        exception() : runtime_error("") {
        }

        exception(const std::string &e) : runtime_error(e) {
        }
    };

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

    class badAccountDataException : public exception {
    public:
        badAccountDataException(const std::string &e) : exception(e) {
        }

        badAccountDataException() : exception("Crucial account metadata missing") {
        }
    };

    class badMessageDataException : public exception {
    public:
        badMessageDataException(const std::string &e) : exception(e) {
        }

        badMessageDataException() : exception("Unhandleable message data received") {
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