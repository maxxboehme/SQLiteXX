#include "Mutex.h"

#include <sqlite3.h>

namespace sqlite {
    mutex::mutex(sqlite3_mutex *mutex) :
        native_handle(mutex)
    {}

    void mutex::lock() noexcept {
        sqlite3_mutex_enter(native_handle);
    }

    bool mutex::try_lock() noexcept {
        return sqlite3_mutex_try(native_handle) == SQLITE_OK;
    }

    void mutex::unlock() noexcept {
        sqlite3_mutex_leave(native_handle);
    }
}
