#include "Backup.h"
#include "Exception.h"


namespace SQLite
{
    void SaveToDisk(const DBConnection &source, const std::string &filename)
    {
        DBConnection destination(filename);
        Backup backup(source, destination);
        backup.step();
    }

    Backup::Backup(
        DBConnection const & source,
        DBConnection const & destination,
        const std::string &sourceName,
        const std::string &destinationName) :
        m_handle(
            sqlite3_backup_init(
                destination.getHandle(),
                destinationName.c_str(),
                source.getHandle(),
                sourceName.c_str()),
            sqlite3_backup_finish),
        m_destination(&destination)
    {
        if (!m_handle)
        {
            throwErrorCode(destination.getHandle());
        }
    }

    bool Backup::step(const int pages)
    {
        int const result = sqlite3_backup_step(m_handle.get(), pages);

        if (result == SQLITE_OK) return true;
        if (result == SQLITE_DONE) return false;

        m_handle.reset();
        throwErrorCode(m_destination->getHandle());
        return false;
    }

    int Backup::getTotalPageCount() noexcept
    {
        return sqlite3_backup_pagecount(m_handle.get());
    }

    int Backup::getRemainingPageCount() noexcept
    {
        return sqlite3_backup_remaining(m_handle.get());
    }

    sqlite3_backup* Backup::getHandle() noexcept
    {
        return m_handle.get();
    }
}
