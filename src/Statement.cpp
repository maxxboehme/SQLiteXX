
#include "Statement.h"

namespace sqlite
{
    row_iterator begin(const statement &statement) noexcept
    {
        return row_iterator(statement);
    }

    row_iterator end(const statement &statement) noexcept
    {
        // statement argument is unused because the end is a nullptr.
        // This will stop warnings
        (void)statement;

        return row_iterator();
    }

    statement::statement() noexcept :
        m_handle(nullptr, sqlite3_finalize),
        m_done(false)
    {}

    statement::operator bool() const noexcept
    {
        return static_cast<bool>(m_handle);
    }

    /** Returns pointer to the underlying "sqlite3_stmt" object.
     * */
    sqlite3_stmt* statement::handle() const noexcept
    {
        return m_handle.get();
    }

    bool statement::step() const
    {
        // This is to signal when the user has reached the end but
        // calls step again. The official SQLite3 API specifies that "sqlite3_step"
        // should not be called again after done before a reset. We just want it to
        // return false signaling done and be a NOP.
        if (m_done) return false;

        const int result =  sqlite3_step(handle());

        if (result == SQLITE_ROW) return true;
        if (result == SQLITE_DONE) {
            m_done = true;
            return false;
        }

        throw_last_error();
        return false;
    }

    int statement::execute() const
    {
        bool done = !step();
        // This variable is only used for the assert.
        // Doing this so there is no warnings on release builds.
        ((void)done);
        assert(done);

        return sqlite3_changes(sqlite3_db_handle(handle()));
    }

    void statement::bind(const int index, const int value) const
    {
        if (SQLITE_OK != sqlite3_bind_int(handle(), index, value))
        {
            throw_last_error();
        }
    }

    void statement::bind(const int index, const double value) const
    {
        if (SQLITE_OK != sqlite3_bind_double(handle(), index, value))
        {
            throw_last_error();
        }
    }

    void statement::bind(const int index, const void * const value, const int size, bindtype type) const
    {
        if (SQLITE_OK != sqlite3_bind_blob(handle(), index, value, size, type == bindtype::transient ? SQLITE_TRANSIENT : SQLITE_STATIC))
        {
            throw_last_error();
        }
    }

    void statement::bind(const int index, const blob &value) const
    {
        if (SQLITE_OK != sqlite3_bind_blob(handle(), index, value.data(), value.size(), SQLITE_TRANSIENT))
        {
            throw_last_error();
        }
    }

    void statement::bind(const int index, const char * const value, const int size, bindtype type) const
    {
        if (SQLITE_OK != sqlite3_bind_text(handle(), index, value, size, type == bindtype::transient ? SQLITE_TRANSIENT : SQLITE_STATIC))
        {
            throw_last_error();
        }
    }

    void statement::bind(const int index, const char16_t * const value, const int size, bindtype type) const
    {
        if (SQLITE_OK != sqlite3_bind_text16(handle(), index, value, size, type == bindtype::transient ? SQLITE_TRANSIENT : SQLITE_STATIC))
        {
            throw_last_error();
        }
    }

    void statement::bind(const int index, const std::string &value) const
    {
        bind(index, value.c_str(), value.size());
    }

    void statement::bind(const int index, const std::u16string &value) const
    {
        bind(index, value.c_str(), value.size() * sizeof(char16_t));
    }

    void statement::throw_last_error() const
    {
        throw_error_code(sqlite3_db_handle(handle()));
    }

    row_iterator::row_iterator(const statement &statement) noexcept
    {
        if (statement.step())
        {
            m_statement = &statement;
        }
    }

    row_iterator& row_iterator::operator++() noexcept
    {
        if (!m_statement->step())
        {
            m_statement = nullptr;
        }

        return *this;
    }

    bool row_iterator::operator!=(const row_iterator &other) const noexcept
    {
        return m_statement != other.m_statement;
    }

    row row_iterator::operator*() const noexcept
    {
        return row(m_statement->handle());
    }
}
