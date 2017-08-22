#include "Mutex.h"

#include <sqlite3.h>


namespace SQLite
{
    Mutex::Mutex(sqlite3_mutex *mutex) :
        native_handle(mutex)
    {}

    void Mutex::lock() noexcept
    {
        sqlite3_mutex_enter(native_handle);
    }

    bool Mutex::tryLock() noexcept
    {
        return sqlite3_mutex_try(native_handle) == SQLITE_OK;
    }

    void Mutex::unlock() noexcept
    {
        sqlite3_mutex_leave(native_handle);
    }
}
