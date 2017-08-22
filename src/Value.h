/** @file */

#ifndef __SQLITEXX_SQLITE_VALUE_H__
#define __SQLITEXX_SQLITE_VALUE_H__

#include "SQLiteEnums.h"
#include "Blob.h"

#include <sqlite3.h>

#include <cassert>
#include <cstring>
#include <string>

namespace SQLite
{
    /** A SQLite dynamically typed value object, aka "sqlite3_value".
     * Value objects represent all values that can be stored in a database table.
     * A Value object may be either "protected" or "unprotected" which refers
     * to whether or not a mutex is held. An internal mutex is held for a protected value object but
     * not for an unprotected one. If SQLite is compiled to be single-threaded or if SQLite is run in one of reduced mutex modes
     * then there is no distinction between protected and unprotected sqlite3_value objects. A Value objects will always be
     * "protected" as it stores a sqlite3_value objects created from calling the sqlite3_value_dup() interface which produces a "protected"
     * "sqlite3_value" from an "unprotected" one.
     * Only use a Value object in the same thread as the SQL function that created it.
     */
    class Value
    {
        public:

        /** Constructs a Value object from a sqlite3_value object.
         * @param[in] value a pointer to an sqlite3_value object to initalize object with.
         */
        Value(const sqlite3_value* const value);

        /** Copy constructor.
         * Constructs a Value object with a copy of the contents of other.
         * @param[in] other another Value object to use as source to initialize object with.
         */
        Value(const Value& other);

        /** Move constructor.
         * Constructs a Value object with a copy of the contents of other using move semantics.
         * @param[in] other another Value object to use as source to initialize object with.
         */
        Value(Value&& other);

        /** Copy assignment operator.
         * Replaces the contents with those of other.
         * @param[in] other another Value object to use as source to initialize object with.
         * @returns *this
         */
        Value& operator=(const Value& other);

        /** Move assignment operator.
         * Replaces the contents with those of other using move semantics.
         * @param[in] other another Value object to use as source to initialize object with.
         * @returns *this
         */
        Value& operator=(Value&& other);

        /** Returns pointer to the underlying "sqlite3_value" object.
         */
        sqlite3_value* getHandle() const noexcept;

        /** Represents the value as an integer.
         * @returns An integer representing the value of the object.
         */
        int getInt() const noexcept;

        /** Represents the value as a 64-bit integer.
         * @returns An 64-bit integer representing the value of the object.
         */
        int64_t getInt64() const noexcept;

        /** Represents the value as an unsigned integer.
         * @returns An unsigned integer representing the value of the object.
         */
        unsigned int getUInt() const noexcept;

        /** Represents the value as a double.
         * @returns A double representing the value of the object.
         */
        double getDouble() const noexcept;

        /** Represents the value as a blob object.
         * @returns A Blob object representing the value of the object.
         */
        const Blob getBlob() const noexcept;

        /** Represents the value as a string.
         * @returns A string representing the value of the object.
         */
        const std::string getString() const noexcept;

        /** Represents the value as a UTF-16 string.
         * @returns A UTF-16 string representing the value of the object.
         */
        const std::u16string getU16String() const noexcept;

        /** Returns the size in bytes of the value.
         * @returns The size in bytes of the value.
         */
        int getBytes() const noexcept;

        /** Returns the Type for the inital datatype of the value.
         *
         * Warning: Other interfaces might change the datatype for an Value object.
         * For example, if the datatype is initially SQLite::Type::Integer and getString() is called
         * to extract a text value for that integer, then subsequent calls to getType() might return
         * SQLite::Type::String.  Whether or not a persistent internal datype conversion occurs is undefined
         * and my change from one release of SQLite to the nexxt.
         * @returns The type of the value.
         */
        Type getType() const noexcept;

        operator int() const;
        operator unsigned int() const;
#if (LONG_MAX == INT_MAX) // sizeof(long)==4 means long is equivalent to int
        operator unsigned long() const;
#endif
        operator long() const;
        operator long long() const;
        operator double() const;
        operator const Blob() const;
        operator const std::string() const;
        operator const std::u16string() const;

        private:

        using ValueHandle = std::unique_ptr<sqlite3_value, decltype(&sqlite3_value_free)>;
        ValueHandle m_handle;

        const char* getText() const noexcept;

        /** Extracts a UTF-16 string in the native byte-order of the host machine.
         * Please pay attention to the fact that the pointer returned from:
         * getBlob(), getString(), or getWideString() can be invalidated by a subsequent call to
         * getBytes(), getBytes16(), getString(), getWideString().
         * */
        const char16_t* getText16() const noexcept;
        int getTextLength() const noexcept;
        int getText16Length() const noexcept;
    };

    inline Value::operator int() const
    {
        return getInt();
    }

    inline Value::operator unsigned int() const
    {
        return getUInt();
    }

#if (LONG_MAX == INT_MAX) // sizeof(long)==4 means long is equivalent to int
    inline Value::operator long() const
    {
        return getInt();
    }

    inline Value::operator unsigned long() const
    {
        return getUInt();
    }
#else
    inline Value::operator long() const
    {
        return getInt64();
    }
#endif

    inline Value::operator long long() const
    {
        return getInt64();
    }

    inline Value::operator double() const
    {
        return getDouble();
    }

    inline Value::operator const Blob() const
    {
        return getBlob();
    }

    inline Value::operator const std::string() const
    {
        return getString();
    }

    inline Value::operator const std::u16string() const
    {
        return getU16String();
    }
}

#endif

