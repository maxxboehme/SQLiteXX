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


#define SQLITEXX_VERSION "0.1.0"


/** SQLiteXX classes and functions are defined in this namespace.
 */
namespace SQLite
{
    inline const char* sqliteLibVersion() noexcept
    {
        return sqlite3_libversion();
    }

    inline int sqliteLibVersionNumber() noexcept
    {
        return sqlite3_libversion_number();
    }

    inline const char* sqlitexxLibVersion() noexcept
    {
        return SQLITEXX_VERSION;
    }
}

#endif

