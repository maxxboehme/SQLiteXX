/** @file */

#ifndef __SQLITEXX_SQLITE_SQLITEENUMS_H__
#define __SQLITEXX_SQLITE_SQLITEENUMS_H__

#include <sqlite3.h>


namespace SQLite
{
    /** Every value in SQLite has one of the following fundamental datatypes.
     */
    enum class Type: int {
        Integer = SQLITE_INTEGER, ///< 64-bit signed integer
        Float   = SQLITE_FLOAT,   ///< 64-bit IEEE floating point number
        Blob    = SQLITE_BLOB,    ///< a group and bits
        Null    = SQLITE_NULL,    ///< NULL
        Text    = SQLITE3_TEXT,   ///< a string
    };

    /** Used to specify the way to bind a value to a statement.
     */
    enum class BindType: int {
        Static,   ///< means that the content pointer is constant and will never change.
        Transient ///< means that the content will likely change in the near future and that SQLite should make its own private copy of the content before returning.
    };
}

#endif
