#ifndef __SQLITECXX_SQLITE_MUTEX_H__
#define __SQLITECXX_SQLITE_MUTEX_H__

#include <sqlite3.h>


namespace SQLite
{
    /** Helps with serializing access to a database connection.
     * Mutexes are only useful when threading mode is set to "Serialized".
     */
    class Mutex
    {
        public:

        explicit Mutex(sqlite3_mutex *mutex);

        /** Locks the mutex, blocks if the mutex is not available
         */
        void lock() noexcept;

        /** Tries to lock the mutex, returns if the mutex is not available
         * @returns True if able to obtain lock. False otherwise.
         */
        bool tryLock() noexcept;

        /** Unlocks the mutex
         */
        void unlock() noexcept;

        private:
        sqlite3_mutex *native_handle;
    };

    /** Locks the given Mutex.
     * Uses RAII to help with locking and unlocking the given Mutex.
     */
    class Lock
    {
        public:

        explicit Lock(const Mutex &m);
        ~Lock();

        private:
        Mutex m_mutex;

        Lock(const Lock &) = delete;
        Lock& operator==(const Lock &) = delete;
    };
}

#endif
