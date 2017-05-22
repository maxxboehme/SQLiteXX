#ifndef __SQLITECXX_SQLITE_BACKUP_H__
#define __SQLITECXX_SQLITE_BACKUP_H__

#include "DBConnection.h"

#include <sqlite3.h>

#include <memory>

namespace SQLite
{
    class Backup
    {
        public:

        Backup(const Backup& other) = delete;
        Backup& operator=(Backup& other) = delete;

        Backup(
            DBConnection const & source,
            DBConnection const & destination,
            const std::string &sourceName = "main",
            const std::string &destinationName = "main");

        bool step(const int pages = -1);
        int getTotalPageCount();
        int getRemainingPageCount();
        sqlite3_backup* getHandle() const noexcept;

        private:
        using BackupHandle = std::unique_ptr<sqlite3_backup, decltype(&sqlite3_backup_finish)>;
        BackupHandle m_handle;

        const DBConnection *m_destination = nullptr;
    };

    inline int Backup::getTotalPageCount()
    {
        return sqlite3_backup_pagecount(m_handle.get());
    }

    inline int Backup::getRemainingPageCount()
    {
        return sqlite3_backup_remaining(m_handle.get());
    }

    inline sqlite3_backup* Backup::getHandle() const noexcept
    {
        return m_handle.get();
    }

}

#endif
