#define CATCH_CONFIG_MAIN  // This tells Catch to provide a main() - only do this in one cpp file
#include "catch/catch.hpp"

inline void profileHandler(void *, char const * const statement, unsigned long long const time)
{
   unsigned long long const ms = time / 1000000;


   if (ms > 10)
   {
      printf("Profiler (%lld) %s\n", ms, statement);
   }
}

