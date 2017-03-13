#ifndef __SQLITECXX_SQLITE_EXCEPTION_H__
#define __SQLITECXX_SQLITE_EXCEPTION_H__

#include <sqlite3.h>

#include <stdexcept>
#include <string>

namespace SQLite
{
    /** Encapsulation of the error code and message from SQLite3, based on std::runtime_error. */
    class Exception : public std::runtime_error
    {
        public:

        const int errcode;
        const std::string message;

        explicit Exception(sqlite3 * const connection) :
            std::runtime_error(sqlite3_errmsg(connection)),
            errcode(sqlite3_extended_errcode(connection)),
            message(sqlite3_errmsg(connection))
        {}

        explicit Exception(const int code, const std::string& message) :
            std::runtime_error(message),
            errcode(code),
            message(message)
        {}
    };

    /** Encapsulation of the SQLITE_BUSY error code derived from SQLite::Exception. */
    class BusyException: public Exception
    {
        public:
        explicit BusyException(sqlite3 * const connection) :
            Exception(connection)
        {}

        explicit BusyException(const std::string& message) :
            Exception(SQLITE_BUSY, message)
        {}
    };

    inline void throwErrorCode(sqlite3 *connection)
    {
        const int errcode = sqlite3_extended_errcode(connection);
        if (errcode == SQLITE_OK) return;
        if (errcode == SQLITE_DONE) return;

        switch(errcode)
        {
            case SQLITE_BUSY:
                throw BusyException(connection);
                break;
            default:
                throw Exception(connection);
                break;
        }
    }

    inline void throwErrorCode(const int errcode, const std::string& message)
    {
        if (errcode == SQLITE_OK) return;
        if (errcode == SQLITE_DONE) return;

        switch(errcode)
        {
            case SQLITE_BUSY:
                throw BusyException(message);
                break;
            default:
                throw Exception(errcode, message);
                break;
        }
    }
}

#endif

