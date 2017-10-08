/** @file */

#ifndef __SQLITEXX_SQLITE_MUTEX_H__
#define __SQLITEXX_SQLITE_MUTEX_H__

#include <sqlite3.h>

namespace sqlite {

    /** Helps with serializing access to a database connection.
     * mutexes are only useful when threading mode is set to "Serialized".
     *
     * This mutex class can be used with
     * [std::lock_guard](http://en.cppreference.com/w/cpp/thread/lock_guard) as it
     * meets the
     * [BasicLockable](http://en.cppreference.com/w/cpp/concept/BasicLockable)
     * requirements.
     */
    class mutex {
        public:
        explicit mutex(sqlite3_mutex *mutex);
        mutex(const mutex &other) = default;

        /** Locks the mutex and blocks if the mutex is not available.
        */
        void lock() noexcept;

        /** Tries to lock the mutex, returns if the mutex is not available.
         * @returns True if able to obtain lock. False otherwise.
         */
        bool try_lock() noexcept;

        /** Unlocks the mutex.
        */
        void unlock() noexcept;

        private:
        sqlite3_mutex *native_handle;
    };
}

#endif
