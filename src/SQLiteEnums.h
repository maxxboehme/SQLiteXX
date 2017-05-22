#ifndef __SQLITECXX_SQLITE_SQLITEENUMS_H__
#define __SQLITECXX_SQLITE_SQLITEENUMS_H__

#include <sqlite3.h>


namespace SQLite
{
    enum class Type: int {
        Integer = SQLITE_INTEGER,
        Float   = SQLITE_FLOAT,
        Blob    = SQLITE_BLOB,
        Null    = SQLITE_NULL,
        Text    = SQLITE_TEXT,
    };

    enum class BindType: int {
        Static,
        Transient
    };
}

#endif
