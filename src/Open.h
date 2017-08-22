/** @file */

#ifndef __SQLITEXX_SQLITE_OPEN_H__
#define __SQLITEXX_SQLITE_OPEN_H__

#include <sqlite3.h>

#include <type_traits>

namespace SQLite
{
    /** Different ways to open a DBConnection.
     */
    enum class OpenMode : int {
        ReadOnly     = SQLITE_OPEN_READONLY,     ///< opened in read-only mode
        ReadWrite    = SQLITE_OPEN_READWRITE,    ///< opened for reading and writing if possible, or reading only if the file is write protected by the operating system.
        Create       = SQLITE_OPEN_CREATE,       ///< database will be created if it does not already exist
        URI          = SQLITE_OPEN_URI,          ///< URI filename interpretation is enabled
        Memory       = SQLITE_OPEN_MEMORY,       ///< open an in memory database
        NoMutex      = SQLITE_OPEN_NOMUTEX,      ///< database connections opens in the multi-thread threading mode as long as the single-thread mode has not been set at compile-time or start-time
        FullMutex    = SQLITE_OPEN_FULLMUTEX,    ///< database connection opens in the serialized threading mode unless single-thread was previously selected at compile-time or start-time.
        SharedCache  = SQLITE_OPEN_SHAREDCACHE,  ///< causes the database connection to be eligible to use shared cache mode, regardless of whether or not shared cache is enabled.
        PrivateCache = SQLITE_OPEN_PRIVATECACHE, ///< causes the database connection to not participate in shared cache mode even if it is enabled.
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
