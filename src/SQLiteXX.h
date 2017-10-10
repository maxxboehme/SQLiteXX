/** @file */
#ifndef __SQLITEXX_SQLITE_SQLITEXX_H__
#define __SQLITEXX_SQLITE_SQLITEXX_H__

#include "Backup.h"
#include "DBConnection.h"
#include "Exception.h"
#include "Functions.h"
#include "Open.h"
#include "Statement.h"
#include "Transaction.h"

#include <sqlite3.h>


#define SQLITEXX_VERSION "0.1.0"


/** SQLiteXX classes and functions are defined in this namespace.
 */
namespace sqlite
{
    inline const char* sqlite_libversion() noexcept
    {
        return sqlite3_libversion();
    }

    inline int sqlite_libversion_number() noexcept
    {
        return sqlite3_libversion_number();
    }

    inline const char* sqlitexx_libversion() noexcept
    {
        return SQLITEXX_VERSION;
    }
}

#endif

