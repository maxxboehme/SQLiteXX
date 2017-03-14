#ifndef __SQLITECXX_SQLITE_DBCONNECTION_H__
#define __SQLITECXX_SQLITE_DBCONNECTION_H__

#include "Exception.h"
#include "Open.h"

#include <sqlite3.h>

#include <cassert>
#include <chrono>
#include <memory>
#include <thread>


namespace SQLite
{
    static const std::chrono::minutes DEFAULT_TIMEOUT(10);

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
        void internalOpen(F open, const C  * const filename)
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

        DBConnection(DBConnection &&other) noexcept :
            m_handle(std::move(other.m_handle))
        {}

        DBConnection& operator=(const DBConnection &other) noexcept
        {
            if (this != &other) {
                m_handle = other.m_handle;
            }

            return *this;
        }

        DBConnection& operator=(DBConnection &&other) noexcept
        {
            assert(this != &other);
            m_handle = std::move(other.m_handle);
            return *this;
        }

        /** Returns a Mutex that serializes access to the database.
         */
        Mutex getMutex()
        {
            sqlite3_mutex *mutexPtr = sqlite3_db_mutex(m_handle.get());
            if (mutexPtr == nullptr) {
                //TODO: throw exeption
            }
            return Mutex(mutexPtr);
        }

        /** Open the provided database UTF-8 filename.
         * @param[in] filename UTF-8 path/uri to the database database file
         * @param[in] mode     file opening options specified by combination of OpenMode flags
         * @param[in] timeout  amount of milliseconds to wait before returning SQLite::BusyException when a table is locked
         */
        explicit DBConnection(const std::string &filename,
            OpenMode mode = OpenMode::ReadWrite | OpenMode::Create,
            const std::chrono::milliseconds timeout = DEFAULT_TIMEOUT)
        {
            open(filename, mode);
            sqlite3_busy_timeout(getHandle(), timeout.count());
            // 278.817s
            // sqlite3_busy_handler(getHandle(), infinit_busy_handler, NULL);
        }

        /** Open the provided database UTF-8 filename.
         * @param[in] filename UTF-8 path/uri to the database database file
         * @param[in] timeout  amount of milliseconds to wait before returning SQLite::BusyException when a table is locked
         */
        explicit DBConnection(const std::string &filename,
            const std::chrono::milliseconds timeout)
        {
            this->internalOpen(sqlite3_open, filename.c_str());
            sqlite3_busy_timeout(getHandle(), timeout.count());
            // 278.817s
            // sqlite3_busy_handler(getHandle(), infinit_busy_handler, NULL);
        }

        /** Open the provided database UTF-16 filename.
         * @param[in] filename UTF-16 path/uri to the database database file
         * @param[in] timeout  Amount of milliseconds to wait before returning SQLite::BusyException when a table is locked
         */
        explicit DBConnection(
            const std::u16string &filename,
            const std::chrono::milliseconds timeout = DEFAULT_TIMEOUT)
        {
            open(filename);
            sqlite3_busy_timeout(getHandle(), timeout.count());
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

        /** Open an SQLite database file as specified by the filename argument.
         * @param filename path to SQLite file
         */
        void open(const std::string &filename, OpenMode mode = OpenMode::ReadWrite | OpenMode::Create)
        {
            sqlite3 *connection;
            if (SQLITE_OK != sqlite3_open_v2(filename.c_str(), &connection, static_cast<int>(mode), nullptr)) {
                const SQLite::Exception exception(connection);
                sqlite3_close(connection);
                throw exception;
            }

            m_handle.reset(connection, sqlite3_close);
        }

        /** Open an SQLite database file as specified by the filname argument.
         * The database file will have UTF-16 native byte order.
         * @param filename path to SQLite file
         */
        void open(const std::u16string &filename)
        {
            this->internalOpen(sqlite3_open16, filename.c_str());
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
