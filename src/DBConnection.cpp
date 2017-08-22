#include "DBConnection.h"

namespace SQLite
{
    const std::chrono::minutes DBConnection::DEFAULT_TIMEOUT(10);

    DBConnection::DBConnection() noexcept :
        m_handle()
    {}

    DBConnection::DBConnection(const DBConnection& other) noexcept :
        m_handle(other.m_handle)
    {}

    DBConnection::DBConnection(DBConnection&& other) noexcept :
        m_handle(std::move(other.m_handle))
    {}

    DBConnection& DBConnection::operator=(const DBConnection& other) noexcept
    {
        if (this != &other) {
            m_handle = other.m_handle;
        }

        return *this;
    }

    DBConnection& DBConnection::operator=(DBConnection&& other) noexcept
    {
        assert(this != &other);
        m_handle = std::move(other.m_handle);
        return *this;
    }

    /** Open the provided database UTF-8 filename.
     * @param[in] filename UTF-8 path/uri to the database database file
     * @param[in] mode     file opening options specified by combination of OpenMode flags
     * @param[in] timeout  amount of milliseconds to wait before returning SQLite::BusyException when a table is locked
     */
    DBConnection::DBConnection(
        const std::string& filename,
        OpenMode mode,
        const std::chrono::milliseconds timeout)
    {
        open(filename, mode);
        sqlite3_busy_timeout(getHandle(), timeout.count());
    }

    /** Open the provided database UTF-8 filename.
     * @param[in] filename UTF-8 path/uri to the database database file
     * @param[in] timeout  amount of milliseconds to wait before returning SQLite::BusyException when a table is locked
     */
    DBConnection::DBConnection(
        const std::string& filename,
        const std::chrono::milliseconds timeout)
    {
        open(filename);
        sqlite3_busy_timeout(getHandle(), timeout.count());
    }

    /** Open the provided database UTF-16 filename.
     * @param[in] filename UTF-16 path/uri to the database database file
     * @param[in] timeout  Amount of milliseconds to wait before returning SQLite::BusyException when a table is locked
     */
    DBConnection::DBConnection(
        const std::u16string& filename,
        const std::chrono::milliseconds timeout)
    {
        open(filename);
        sqlite3_busy_timeout(getHandle(), timeout.count());
    }

    DBConnection DBConnection::memory()
    {
        return DBConnection(":memory:");
    }

    DBConnection DBConnection::wideMemory()
    {
        return DBConnection(u":memory:");
    }

    Mutex DBConnection::getMutex()
    {
        sqlite3_mutex *mutexPtr = sqlite3_db_mutex(m_handle.get());
        if (mutexPtr == nullptr) {
           throw SQLiteXXException("This database connection was not able to create a valid mutex.");
        }
        return Mutex(mutexPtr);
    }

    DBConnection::operator bool() const noexcept
    {
        return static_cast<bool>(m_handle);
    }

    sqlite3* DBConnection::getHandle() const noexcept
    {
        return m_handle.get();
    }

    void DBConnection::open(const std::string& filename, OpenMode mode)
    {
        sqlite3 *connection;
        if (SQLITE_OK != sqlite3_open_v2(filename.c_str(), &connection, static_cast<int>(mode), nullptr)) {
            const SQLite::Exception exception(connection);
            sqlite3_close(connection);
            throw exception;
        }

        m_handle.reset(connection, sqlite3_close);
    }

    void DBConnection::open(const std::u16string& filename)
    {
        sqlite3 *connection;
        if (SQLITE_OK != sqlite3_open16(filename.c_str(), &connection)) {
            const SQLite::Exception exception(connection);
            sqlite3_close(connection);
            throw exception;
        }

        m_handle.reset(connection, sqlite3_close);
    }

    long long DBConnection::rowId() const noexcept
    {
        return sqlite3_last_insert_rowid(getHandle());
    }
}
