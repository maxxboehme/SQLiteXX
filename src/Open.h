#ifndef __SQLITECXX_SQLITE_OPEN_H__
#define __SQLITECXX_SQLITE_OPEN_H__

#include <sqlite3.h>

#include <type_traits>

namespace SQLite
{
    /** Different ways to open a DBConnection.
     */
    enum class OpenMode : int {
        ReadOnly     = SQLITE_OPEN_READONLY,
        ReadWrite    = SQLITE_OPEN_READWRITE,
        Create       = SQLITE_OPEN_CREATE,
        URI          = SQLITE_OPEN_URI,
        Memory       = SQLITE_OPEN_MEMORY,
        NoMutex      = SQLITE_OPEN_NOMUTEX,
        FullMutex    = SQLITE_OPEN_FULLMUTEX,
        SharedCache  = SQLITE_OPEN_SHAREDCACHE,
        PrivateCache = SQLITE_OPEN_PRIVATECACHE,
    };

    inline OpenMode operator&(OpenMode lhs, OpenMode rhs) {
        return static_cast<OpenMode>(
                static_cast<std::underlying_type<OpenMode>::type>(lhs) &
                static_cast<std::underlying_type<OpenMode>::type>(rhs));
    }

    inline OpenMode operator|(OpenMode lhs, OpenMode rhs) {
        return static_cast<OpenMode>(
                static_cast<std::underlying_type<OpenMode>::type>(lhs) |
                static_cast<std::underlying_type<OpenMode>::type>(rhs));
    }

    inline OpenMode operator^(OpenMode lhs, OpenMode rhs) {
        return static_cast<OpenMode>(
                static_cast<std::underlying_type<OpenMode>::type>(lhs) |
                static_cast<std::underlying_type<OpenMode>::type>(rhs));
    }

    inline OpenMode operator~(OpenMode flag) {
        return static_cast<OpenMode>(
                static_cast<std::underlying_type<OpenMode>::type>(flag));
    }

    inline const OpenMode& operator&=(OpenMode &lhs, OpenMode rhs) {
        return lhs = lhs & rhs;
    }

    inline const OpenMode& operator|=(OpenMode &lhs, OpenMode rhs) {
        return lhs = lhs | rhs;
    }

    inline const OpenMode& operator^=(OpenMode &lhs, OpenMode rhs) {
        return lhs = lhs ^ rhs;
    }
}

#endif
