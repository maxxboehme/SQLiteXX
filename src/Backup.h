#ifndef __SQLITECXX_SQLITE_BACKUP_H__
#define __SQLITECXX_SQLITE_BACKUP_H__

#include "DBConnection.h"

#include <sqlite3.h>

#include <memory>

namespace SQLite
{
    class Backup
    {
        using BackupHandle = std::unique_ptr<sqlite3_backup, decltype(&sqlite3_backup_finish)>;
        BackupHandle m_handle;

        const DBConnection  *m_destination = nullptr;

        public:

        Backup(const Backup& other) = delete;
        Backup& operator=(Backup& other) = delete;

        Backup(
            DBConnection const & source,
            DBConnection const & destination,
            const std::string &sourceName = "main",
            const std::string &destinationName = "main") :
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
                destination.throwLastError();
            }
        }

        bool step(const int pages = -1)
        {
            int const result = sqlite3_backup_step(m_handle.get(), pages);

            if (result == SQLITE_OK) return true;
            if (result == SQLITE_DONE) return false;

            m_handle.reset();
            m_destination->throwLastError();
            return false;
        }

        int getTotalPageCount()
        {
            return sqlite3_backup_pagecount(m_handle.get());
        }

        int getRemainingPageCount()
        {
            return sqlite3_backup_remaining(m_handle.get());
        }

        sqlite3_backup * getHandle() const noexcept
        {
            return m_handle.get();
        }
    };

    inline void SaveToDisk(const DBConnection &source, const std::string &filename)
    {
        DBConnection destination(filename);
        Backup backup(destination, source);
        backup.step();
    }
}

#endif
