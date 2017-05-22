#ifndef __SQLITECXX_SQLITE_SQLITEXX_H__
#define __SQLITECXX_SQLITE_SQLITEXX_H__

#include "Backup.h"
#include "DBConnection.h"
#include "Exception.h"
#include "Functions.h"
#include "Open.h"
#include "Statement.h"
#include "Transaction.h"


#define SQLITEXX_VERSION "0.1.0"


namespace SQLite
{
    inline const char* SQliteLibVersion() noexcept
    {
        return sqlite3_libversion();
    }

    inline int SQliteLibVersionNumber() noexcept
    {
        return sqlite3_libversion_number();
    }

    inline const char* SQLiteXXLibVersion() noexcept
    {
        return SQLITEXX_VERSION;
    }
}

#endif

