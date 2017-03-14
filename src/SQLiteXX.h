#ifndef __SQLITECXX_SQLITE_SQLITEXX_H__
#define __SQLITECXX_SQLITE_SQLITEXX_H__

#include "Backup.h"
#include "DBConnection.h"
#include "Exception.h"
#include "Functions.h"
#include "Open.h"
#include "Statement.h"
#include "Transaction.h"

inline void profileHandler(void *, char const * const statement, unsigned long long const time)
{
   unsigned long long const ms = time / 1000000;


   if (ms > 10)
   {
      printf("Profiler (%lld) %s\n", ms, statement);
   }
}

#endif

