
#include "Statement.h"

namespace SQLite
{
    RowIterator begin(const Statement &statement) noexcept
    {
        return RowIterator(statement);
    }

    RowIterator end(const Statement &statement) noexcept
    {
        // statement argument is unused because the end is a nullptr.
        // This will stop warnings
        (void)statement;

        return RowIterator();
    }

    Statement::Statement() noexcept :
        m_handle(nullptr, sqlite3_finalize),
        m_done(false)
    {}

    Statement::operator bool() const noexcept
    {
        return static_cast<bool>(m_handle);
    }

    /** Returns pointer to the underlying "sqlite3_stmt" object.
     * */
    sqlite3_stmt* Statement::getHandle() const noexcept
    {
        return m_handle.get();
    }

    bool Statement::step() const
    {
        // This is to signal when the user has reached the end but
        // calls step again. The official SQLite3 API specifies that "sqlite3_step"
        // should not be called again after done before a reset. We just want it to
        // return false signaling done and be a NOP.
        if (m_done) return false;

        const int result =  sqlite3_step(getHandle());

        if (result == SQLITE_ROW) return true;
        if (result == SQLITE_DONE) {
            m_done = true;
            return false;
        }

        throwLastError();
        return false;
    }

    int Statement::execute() const
    {
        bool done = !step();
        // This variable is only used for the assert.
        // Doing this so there is no warnings on release builds.
        ((void)done);
        assert(done);

        return sqlite3_changes(sqlite3_db_handle(getHandle()));
    }

    void Statement::bind(const int index, const int value) const
    {
        if (SQLITE_OK != sqlite3_bind_int(getHandle(), index, value))
        {
            throwLastError();
        }
    }

    void Statement::bind(const int index, const double value) const
    {
        if (SQLITE_OK != sqlite3_bind_double(getHandle(), index, value))
        {
            throwLastError();
        }
    }

    void Statement::bind(const int index, const void * const value, const int size, BindType type) const
    {
        if (SQLITE_OK != sqlite3_bind_blob(getHandle(), index, value, size, type == BindType::Transient? SQLITE_TRANSIENT : SQLITE_STATIC))
        {
            throwLastError();
        }
    }

    void Statement::bind(const int index, const Blob &value) const
    {
        if (SQLITE_OK != sqlite3_bind_blob(getHandle(), index, value.data(), value.size(), SQLITE_TRANSIENT))
        {
            throwLastError();
        }
    }

    void Statement::bind(const int index, const char * const value, const int size, BindType type) const
    {
        if (SQLITE_OK != sqlite3_bind_text(getHandle(), index, value, size, type == BindType::Transient? SQLITE_TRANSIENT : SQLITE_STATIC))
        {
            throwLastError();
        }
    }

    void Statement::bind(const int index, const char16_t * const value, const int size, BindType type) const
    {
        if (SQLITE_OK != sqlite3_bind_text16(getHandle(), index, value, size, type == BindType::Transient? SQLITE_TRANSIENT : SQLITE_STATIC))
        {
            throwLastError();
        }
    }

    void Statement::bind(const int index, const std::string &value) const
    {
        bind(index, value.c_str(), value.size());
    }

    void Statement::bind(const int index, const std::u16string &value) const
    {
        bind(index, value.c_str(), value.size() * sizeof(char16_t));
    }

    void Statement::throwLastError() const
    {
        throwErrorCode(sqlite3_db_handle(getHandle()));
    }

    RowIterator::RowIterator(const Statement &statement) noexcept
    {
        if (statement.step())
        {
            m_statement = &statement;
        }
    }

    RowIterator& RowIterator::operator++() noexcept
    {
        if (!m_statement->step())
        {
            m_statement = nullptr;
        }

        return *this;
    }

    bool RowIterator::operator!=(const RowIterator &other) const noexcept
    {
        return m_statement != other.m_statement;
    }

    Row RowIterator::operator*() const noexcept
    {
        return Row(m_statement->getHandle());
    }
}
