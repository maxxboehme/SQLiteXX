/** @file */

#ifndef __SQLITEXX_SQLITE_OPEN_H__
#define __SQLITEXX_SQLITE_OPEN_H__

#include <sqlite3.h>

#include <type_traits>

namespace sqlite
{
    /** Different ways to open a dbconnection.
     */
    enum class openmode : int {
        read_only     = SQLITE_OPEN_READONLY,     ///< opened in read-only mode
        read_write    = SQLITE_OPEN_READWRITE,    ///< opened for reading and writing if possible, or reading only if the file is write protected by the operating system.
        create        = SQLITE_OPEN_CREATE,       ///< database will be created if it does not already exist
        uri           = SQLITE_OPEN_URI,          ///< URI filename interpretation is enabled
        memory        = SQLITE_OPEN_MEMORY,       ///< open an in memory database
        no_mutex      = SQLITE_OPEN_NOMUTEX,      ///< database connections opens in the multi-thread threading mode as long as the single-thread mode has not been set at compile-time or start-time
        full_mutex    = SQLITE_OPEN_FULLMUTEX,    ///< database connection opens in the serialized threading mode unless single-thread was previously selected at compile-time or start-time.
        shared_cache  = SQLITE_OPEN_SHAREDCACHE,  ///< causes the database connection to be eligible to use shared cache mode, regardless of whether or not shared cache is enabled.
        private_cache = SQLITE_OPEN_PRIVATECACHE, ///< causes the database connection to not participate in shared cache mode even if it is enabled.
    };

    inline openmode operator&(openmode lhs, openmode rhs) {
        return static_cast<openmode>(
                static_cast<std::underlying_type<openmode>::type>(lhs) &
                static_cast<std::underlying_type<openmode>::type>(rhs));
    }

    inline openmode operator|(openmode lhs, openmode rhs) {
        return static_cast<openmode>(
                static_cast<std::underlying_type<openmode>::type>(lhs) |
                static_cast<std::underlying_type<openmode>::type>(rhs));
    }

    inline openmode operator^(openmode lhs, openmode rhs) {
        return static_cast<openmode>(
                static_cast<std::underlying_type<openmode>::type>(lhs) |
                static_cast<std::underlying_type<openmode>::type>(rhs));
    }

    inline openmode operator~(openmode flag) {
        return static_cast<openmode>(
                static_cast<std::underlying_type<openmode>::type>(flag));
    }

    inline const openmode& operator&=(openmode &lhs, openmode rhs) {
        return lhs = lhs & rhs;
    }

    inline const openmode& operator|=(openmode &lhs, openmode rhs) {
        return lhs = lhs | rhs;
    }

    inline const openmode& operator^=(openmode &lhs, openmode rhs) {
        return lhs = lhs ^ rhs;
    }
}

#endif
