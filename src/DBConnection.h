#ifndef __SQLITECXX_SQLITE_DBCONNECTION_H__
#define __SQLITECXX_SQLITE_DBCONNECTION_H__

#include <memory>
#include <chrono>
#include <thread>


namespace SQLite
{
    // 10 minutes
    static const int DEFAULT_TIMEOUT = 600000;

    inline const char* sqliteLibVersion() noexcept
    {
        return sqlite3_libversion();
    }

    inline const int sqliteLibVersionNumber() noexcept
    {
        return sqlite3_libversion_number();
    }

    inline int infinit_busy_handler(void * arg, int numCalls)
    {
        std::chrono::duration<int, std::milli> duration(20);
        std::this_thread::sleep_for(duration);
        return 1;
    }

    /** Helps with serializing access to a database connection.
     * Mutexes are only useful when threading mode is set to "Serialized".
     */
    class Mutex
    {
        sqlite3_mutex *native_handle;

        public:

        Mutex(sqlite3_mutex *mutex = nullptr) :
            native_handle(mutex)
        {}

        /** Locks the mutex, blocks if the mutex is not available
         */
        void lock() noexcept
        {
            sqlite3_mutex_enter(native_handle);
        }

        /** Tries to lock the mutex, returns if the mutex is not available
         */
        int tryLock() noexcept
        {
            return sqlite3_mutex_try(native_handle);
        }

        /** Unlocks the mutex
         */
        void unlock() noexcept
        {
            sqlite3_mutex_leave(native_handle);
        }
    };

    /** Locks the given Mutex.
     * Uses RAII to help with locking and unlocking the given Mutex.
     */
    class Lock
    {
        //sqlite3_mutex *m_mutex;
        Mutex m_mutex;

        Lock(const Lock &) = delete;
        Lock& operator==(const Lock &) = delete;

        public:

        Lock(const Mutex &m) :
            m_mutex(m)
        {
            m_mutex.lock();
        }

        ~Lock()
        {
            m_mutex.unlock();
        }
    };

    /** Class that represents a connection to the database.
     * DBConnection is a wrapper around the "sqlite3" structure.
     */
    class DBConnection
    {
        using ConnectionHandle = std::shared_ptr<sqlite3>;
        ConnectionHandle m_handle;

        template <typename F, typename C>
        void internalOpen(F open, C const * const filename)
        {
            sqlite3 *connection;
            if (SQLITE_OK != open(filename, &connection)) {
                const SQLite::Exception exception(connection);
                sqlite3_close(connection);
                throw exception;
            }

            m_handle.reset(connection, sqlite3_close);
        }

        public:

        DBConnection() noexcept :
            m_handle()
            {}

        DBConnection(const DBConnection &other) noexcept :
            m_handle(other.m_handle)
            {}

        DBConnection operator=(const DBConnection &other) noexcept
        {
            if (this != &other) {
                m_handle = other.m_handle;
            }

            return *this;
        }

        /** Returns a Mutex that serializes access to the database.
         */
        Mutex getMutex()
        {
            return Mutex(sqlite3_db_mutex(m_handle.get()));
        }

        /** Open the provided database UTF-8 filename.
         * @param filename [in] UTF-8/UTF-16 path/uri to the database database file
         * @param timeout [in] Amoutn of milliseconds to wait before returning SQLite::BusyException when a table is locked
         */
        template <typename C>
        explicit DBConnection(C const * const filename, int timeout = DEFAULT_TIMEOUT)
        {
            open(filename);
            sqlite3_busy_timeout(getHandle(), timeout);
            // 278.817s
            // sqlite3_busy_handler(getHandle(), infinit_busy_handler, NULL);
        }

        /** Create a purely in memory database.
         * @returns a purely in memory SQLite::DBConnection
         */
        static DBConnection memory()
        {
            return DBConnection(":memory:");
        }

        /** Create a purely in memory database with UTF-16 as the native byte order.
         * @returns a purely in memory SQLite::DBConnection
         */
        static DBConnection wideMemory()
        {
            return DBConnection(u":memory:");
        }

        explicit operator bool() const noexcept
        {
            return static_cast<bool>(m_handle);
        }

        sqlite3* getHandle() const noexcept
        {
            return m_handle.get();
        }

        void throwLastError() const
        {
            throwErrorCode(getHandle());
        }

        /** Open an SQLite database file as specified by the filname argument.
         * @param filename path to SQLite file
         */
        void open(char const * const filename)
        {
            this->internalOpen(sqlite3_open, filename);
        }

        /** Open an SQLite database file as specified by the filname argument.
         * The database file will have UTF-16 native byte order.
         * @param filename path to SQLite file
         */
        void open(char16_t const * const filename)
        {
            this->internalOpen(sqlite3_open16, filename);
        }

        /** Returns the rowid of the most recent successful "INSERT" into
         * a rowid table or virtual table on database connection.
         * @returns rowid of the most recent successful "INSERT" into the database, or 0 if there was none.
         */
        long long rowId() const noexcept
        {
            return sqlite3_last_insert_rowid(getHandle());
        }

        template <typename F>
        void profile(F callback, void * const context = nullptr)
        {
            sqlite3_profile(getHandle(), callback, context);
        }
    };
}

#endif
