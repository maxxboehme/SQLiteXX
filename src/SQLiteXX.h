#ifndef __SQLITECXX_SQLITE_SQLITECXX_H__
#define __SQLITECXX_SQLITE_SQLITECXX_H__

#include "Exception.h"
#include "DBConnection.h"
#include "Statement.h"
#include "Backup.h"
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

