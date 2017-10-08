/** @file */

#ifndef __SQLITEXX_SQLITE_SQLITEENUMS_H__
#define __SQLITEXX_SQLITE_SQLITEENUMS_H__

#include <sqlite3.h>


namespace sqlite
{
    /** Every value in SQLite has one of the following fundamental datatypes.
     */
    enum class datatype: int {
        integer  = SQLITE_INTEGER, ///< 64-bit signed integer
        floating = SQLITE_FLOAT,   ///< 64-bit IEEE floating point number
        blob     = SQLITE_BLOB,    ///< a group and bits
        null     = SQLITE_NULL,    ///< NULL
        text     = SQLITE3_TEXT,   ///< a string
    };

    /** Used to specify the way to bind a value to a statement.
     */
    enum class bindtype: int {
        staticly,   ///< means that the content pointer is constant and will never change.
        transient ///< means that the content will likely change in the near future and that SQLite should make its own private copy of the content before returning.
    };
}

#endif
