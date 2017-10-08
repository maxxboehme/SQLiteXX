/** @file */

#ifndef __SQLITEXX_SQLITE_EXCEPTION_H__
#define __SQLITEXX_SQLITE_EXCEPTION_H__

#include <sqlite3.h>

#include <stdexcept>
#include <string>

namespace sqlite
{
    /** Encapsulation of the error code and message from SQLite3, based on std::runtime_error. */
    class exception : public std::runtime_error
    {
        public:

        const int errcode;
        const std::string message;

        explicit exception(sqlite3 * const connection) :
            std::runtime_error(sqlite3_errmsg(connection)),
            errcode(sqlite3_extended_errcode(connection)),
            message(sqlite3_errmsg(connection))
        {}

        explicit exception(const int code, const std::string& message) :
            std::runtime_error(message),
            errcode(code),
            message(message)
        {}
    };

    /** Encapsulation of the SQLITE_BUSY error code derived from SQLite::Exception. */
    class busy_exception: public exception
    {
        public:
        explicit busy_exception(sqlite3 * const connection) :
            exception(connection)
        {}

        explicit busy_exception(const std::string& message) :
            exception(SQLITE_BUSY, message)
        {}
    };

    class SQLiteXXException: public std::runtime_error
    {
        public:
        explicit SQLiteXXException(const std::string& message) :
            std::runtime_error(message)
        {}
    };


    void throw_error_code(sqlite3 *connection);
    void throw_error_code(const int errcode, const std::string& message);
}

#endif

