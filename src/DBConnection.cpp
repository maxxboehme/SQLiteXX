#include "DBConnection.h"

namespace sqlite
{
    const std::chrono::minutes dbconnection::DEFAULT_TIMEOUT(10);

    dbconnection::dbconnection() noexcept :
        m_handle()
    {}

    dbconnection::dbconnection(const dbconnection& other) noexcept :
        m_handle(other.m_handle)
    {}

    dbconnection::dbconnection(dbconnection&& other) noexcept :
        m_handle(std::move(other.m_handle))
    {}

    dbconnection& dbconnection::operator=(const dbconnection& other) noexcept
    {
        if (this != &other) {
            m_handle = other.m_handle;
        }

        return *this;
    }

    dbconnection& dbconnection::operator=(dbconnection&& other) noexcept
    {
        assert(this != &other);
        m_handle = std::move(other.m_handle);
        return *this;
    }

    dbconnection::dbconnection(
        const std::string& filename,
        openmode mode,
        const std::chrono::milliseconds timeout)
    {
        open(filename, mode);
        sqlite3_busy_timeout(handle(), static_cast<int>(timeout.count()));
    }

    dbconnection::dbconnection(
        const std::string& filename,
        const std::chrono::milliseconds timeout)
    {
        open(filename);
        sqlite3_busy_timeout(handle(), static_cast<int>(timeout.count()));
    }

    dbconnection::dbconnection(
        const std::u16string& filename,
        const std::chrono::milliseconds timeout)
    {
        open(filename);
        sqlite3_busy_timeout(handle(), static_cast<int>(timeout.count()));
    }

    dbconnection dbconnection::memory()
    {
        return dbconnection(":memory:");
    }

    dbconnection dbconnection::wide_memory()
    {
        return dbconnection(u":memory:");
    }

    mutex dbconnection::mutex()
    {
        sqlite3_mutex* mutexPtr = sqlite3_db_mutex(m_handle.get());
        if (mutexPtr == nullptr) {
           throw SQLiteXXException("This database connection was not able to create a valid mutex.");
        }
        return sqlite::mutex(mutexPtr);
    }

    dbconnection::operator bool() const noexcept
    {
        return static_cast<bool>(m_handle);
    }

    sqlite3* dbconnection::handle() const noexcept
    {
        return m_handle.get();
    }

    void dbconnection::open(const std::string& filename, openmode mode)
    {
        sqlite3 *connection;
        if (SQLITE_OK != sqlite3_open_v2(filename.c_str(), &connection, static_cast<int>(mode), nullptr)) {
            const sqlite::exception exception(connection);
            sqlite3_close(connection);
            throw exception;
        }

        m_handle.reset(connection, sqlite3_close);
    }

    void dbconnection::open(const std::u16string& filename)
    {
        sqlite3 *connection;
        if (SQLITE_OK != sqlite3_open16(filename.c_str(), &connection)) {
            const sqlite::exception exception(connection);
            sqlite3_close(connection);
            throw exception;
        }

        m_handle.reset(connection, sqlite3_close);
    }

    long long dbconnection::row_id() const noexcept
    {
        return sqlite3_last_insert_rowid(handle());
    }
}
