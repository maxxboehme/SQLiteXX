#ifndef __SQLITECXX_SQLITE_VALUE_H__
#define __SQLITECXX_SQLITE_VALUE_H__

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

        Value(const sqlite3_value * const value);
        Value(const Value &other);
        Value(Value &&other);
        Value& operator=(const Value &other);
        Value& operator=(Value && other);

        /** Returns pointer to the underlying "sqlite3_value" object.
         * */
        sqlite3_value * getHandle() const noexcept;
        int getInt() const noexcept;
        int64_t getInt64() const noexcept;
        unsigned int getUInt() const noexcept;
        double getDouble() const noexcept;
        const Blob getBlob() const noexcept;
        const std::string getString() const noexcept;
        const std::u16string getU16String() const noexcept;
        int getBytes() const noexcept;
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

