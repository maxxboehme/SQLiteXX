/** @file */

#ifndef __SQLITEXX_SQLITE_VALUE_H__
#define __SQLITEXX_SQLITE_VALUE_H__

#include "SQLiteEnums.h"
#include "Blob.h"

#include <sqlite3.h>

#include <cassert>
#include <cstring>
#include <string>

namespace sqlite
{
    /** A SQLite dynamically typed value object, aka "sqlite3_value".
     * value objects represent all values that can be stored in a database table.
     * A value object may be either "protected" or "unprotected" which refers
     * to whether or not a mutex is held. An internal mutex is held for a protected value object but
     * not for an unprotected one. If SQLite is compiled to be single-threaded or if SQLite is run in one of reduced mutex modes
     * then there is no distinction between protected and unprotected sqlite3_value objects. A value objects will always be
     * "protected" as it stores a sqlite3_value objects created from calling the sqlite3_value_dup() interface which produces a "protected"
     * "sqlite3_value" from an "unprotected" one.
     * Only use a value object in the same thread as the SQL function that created it.
     */
    class value
    {
        public:

        /** Constructs a value object from a sqlite3_value object.
         * @param[in] value a pointer to an sqlite3_value object to initalize object with.
         */
        explicit value(const sqlite3_value* const value);

        /** Copy constructor.
         * Constructs a value object with a copy of the contents of other.
         * @param[in] other another value object to use as source to initialize object with.
         */
        value(const value& other);

        /** Move constructor.
         * Constructs a value object with a copy of the contents of other using move semantics.
         * @param[in] other another value object to use as source to initialize object with.
         */
        value(value&& other);

        /** Copy assignment operator.
         * Replaces the contents with those of other.
         * @param[in] other another value object to use as source to initialize object with.
         * @returns *this
         */
        value& operator=(const value& other);

        /** Move assignment operator.
         * Replaces the contents with those of other using move semantics.
         * @param[in] other another value object to use as source to initialize object with.
         * @returns *this
         */
        value& operator=(value&& other);

        /** Returns pointer to the underlying "sqlite3_value" object.
         */
        sqlite3_value* handle() const noexcept;

        /** Represents the value as an integer.
         * @returns An integer representing the value of the object.
         */
        int as_int() const noexcept;

        /** Represents the value as a 64-bit integer.
         * @returns An 64-bit integer representing the value of the object.
         */
        int64_t as_int64() const noexcept;

        /** Represents the value as an unsigned integer.
         * @returns An unsigned integer representing the value of the object.
         */
        unsigned int as_uint() const noexcept;

        /** Represents the value as a double.
         * @returns A double representing the value of the object.
         */
        double as_double() const noexcept;

        /** Represents the value as a blob object.
         * @returns A blob object representing the value of the object.
         */
        const blob as_blob() const noexcept;

        /** Represents the value as a string.
         * @returns A string representing the value of the object.
         */
        const std::string as_string() const noexcept;

        /** Represents the value as a UTF-16 string.
         * @returns A UTF-16 string representing the value of the object.
         */
        const std::u16string as_u16string() const noexcept;

        /** Returns the size in bytes of the value.
         * @returns The size in bytes of the value.
         */
        int bytes() const noexcept;

        /** Returns the datatype for the initial datatype of the value.
         *
         * Warning: Other interfaces might change the datatype for an value object.
         * For example, if the datatype is initially sqlite::datatype::integer and as_string() is called
         * to extract a text value for that integer, then subsequent calls to type() might return
         * sqlite::datatype::text.  Whether or not a persistent internal datatype conversion occurs is undefined
         * and my change from one release of sqlite to the next.
         * @returns The type of the value.
         */
        datatype type() const noexcept;

        operator int() const;
        operator unsigned int() const;
#if (LONG_MAX == INT_MAX) // sizeof(long)==4 means long is equivalent to int
        operator unsigned long() const;
#endif
        operator long() const;
        operator long long() const;
        operator double() const;
        operator const blob() const;
        operator const std::string() const;
        operator const std::u16string() const;

        private:

        using value_handle = std::unique_ptr<sqlite3_value, decltype(&sqlite3_value_free)>;
        value_handle m_handle;

        const char* as_text() const noexcept;

        /** Extracts a UTF-16 string in the native byte-order of the host machine.
         * Please pay attention to the fact that the pointer returned from:
         * getBlob(), getString(), or getWideString() can be invalidated by a subsequent call to
         * getBytes(), getBytes16(), getString(), getWideString().
         * */
        const char16_t* as_text16() const noexcept;
        int text_length() const noexcept;
        int text16_length() const noexcept;
    };

    inline value::operator int() const
    {
        return as_int();
    }

    inline value::operator unsigned int() const
    {
        return as_uint();
    }

#if (LONG_MAX == INT_MAX) // sizeof(long)==4 means long is equivalent to int
    inline value::operator long() const
    {
        return as_int();
    }

    inline value::operator unsigned long() const
    {
        return as_uint();
    }
#else
    inline value::operator long() const
    {
        return as_int64();
    }
#endif

    inline value::operator long long() const
    {
        return as_int64();
    }

    inline value::operator double() const
    {
        return as_double();
    }

    inline value::operator const blob() const
    {
        return as_blob();
    }

    inline value::operator const std::string() const
    {
        return as_string();
    }

    inline value::operator const std::u16string() const
    {
        return as_u16string();
    }
}

#endif

