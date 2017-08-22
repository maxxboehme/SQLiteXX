/** @file */

#ifndef __SQLITEXX_SQLITE_MUTEX_H__
#define __SQLITEXX_SQLITE_MUTEX_H__

#include <sqlite3.h>


namespace SQLite
{
    /** Helps with serializing access to a database connection.
     * Mutexes are only useful when threading mode is set to "Serialized".
     *
     * This mutex class can be used with the std::lock_guard as it meets the
     * BasicLockable requirements.
     */
    class Mutex
    {
        public:

        explicit Mutex(sqlite3_mutex *mutex);
        Mutex(const Mutex& other) = default;

        /** Locks the mutex and blocks if the mutex is not available.
         */
        void lock() noexcept;

        /** Tries to lock the mutex, returns if the mutex is not available.
         * @returns True if able to obtain lock. False otherwise.
         */
        bool tryLock() noexcept;

        /** Unlocks the mutex.
         */
        void unlock() noexcept;

        private:
        sqlite3_mutex* native_handle;
    };
}

#endif
