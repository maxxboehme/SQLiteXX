#include "Mutex.h"

#include <sqlite3.h>


namespace SQLite
{
    Mutex::Mutex(sqlite3_mutex *mutex) :
        native_handle(mutex)
    {}

    /** Locks the mutex, blocks if the mutex is not available
     */
    void Mutex::lock() noexcept
    {
        sqlite3_mutex_enter(native_handle);
    }

    /** Tries to lock the mutex, returns if the mutex is not available
     * @returns True if able to obtain lock. False otherwise.
     */
    bool Mutex::tryLock() noexcept
    {
        return sqlite3_mutex_try(native_handle) == SQLITE_OK;
    }

    /** Unlocks the mutex
     */
    void Mutex::unlock() noexcept
    {
        sqlite3_mutex_leave(native_handle);
    }

    Lock::Lock(const Mutex &m) :
        m_mutex(m)
    {
        m_mutex.lock();
    }

    Lock::~Lock()
    {
        m_mutex.unlock();
    }
}
