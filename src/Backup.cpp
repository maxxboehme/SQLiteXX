#include "Backup.h"
#include "Exception.h"


namespace sqlite
{
    void save(const dbconnection &source, const std::string &filename)
    {
        dbconnection destination(filename);
        backup backup(source, destination);
        backup.step();
    }

    backup::backup(
        dbconnection const & source,
        dbconnection const & destination,
        const std::string &sourceName,
        const std::string &destinationName) :
        m_handle(
            sqlite3_backup_init(
                destination.handle(),
                destinationName.c_str(),
                source.handle(),
                sourceName.c_str()),
            sqlite3_backup_finish),
        m_destination(&destination)
    {
        if (!m_handle)
        {
            throw_error_code(destination.handle());
        }
    }

    bool backup::step(const int pages)
    {
        int const result = sqlite3_backup_step(m_handle.get(), pages);

        if (result == SQLITE_OK) return true;
        if (result == SQLITE_DONE) return false;

        m_handle.reset();
        throw_error_code(m_destination->handle());
        return false;
    }

    int backup::total_page_count() noexcept
    {
        return sqlite3_backup_pagecount(m_handle.get());
    }

    int backup::remaining_page_count() noexcept
    {
        return sqlite3_backup_remaining(m_handle.get());
    }

    sqlite3_backup* backup::handle() noexcept
    {
        return m_handle.get();
    }
}
