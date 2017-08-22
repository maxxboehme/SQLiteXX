/** @file */

#ifndef __SQLITEXX_SQLITE_BACKUP_H__
#define __SQLITEXX_SQLITE_BACKUP_H__

#include "DBConnection.h"

#include <sqlite3.h>

#include <memory>

namespace SQLite
{
    /** Used to aid in the process of backing up a database.
     * The Backup object records state information about an ongoing
     * online backup operation.  It us useful either for creating
     * backups of databases or for copying in-memory databases to or
     * from persistent files.
     */
    class Backup
    {
        public:
        /** Construct a backup object.
         * @param source the database that will be the source of copied information
         * @param destination the database that will be backed up to
         * @param sourceName the source database name
         * @param destinationName the destination database name
         *
         * @throws SQLite::Exception If an error occurs when initializing a Backup
         *                           then an SQLite::Exception will be raised.
         */
        Backup(
            const DBConnection& source,
            const DBConnection& destination,
            const std::string& sourceName = "main",
            const std::string& destinationName = "main");

        /** Will copy a specified number of pages to the destination database.
         * If pages is negative, all remaining source pages are copied.
         *
         * @params pages the number of pages to copy from source to destination database.
         * @returns The returne value will be true if there are more pages to copy. False otherwise.
         *
         * @throws SQLite::Exception If an error occurs when initializing a Backup
         *                           then an SQLite::Exception will be raised.
         */
        bool step(const int pages = -1);

        /** Returns the total number of pages in the source database.
         * The values returned by this function is only updated by step().
         * If the source database is modified in a way that changes the size of
         * the source database or the number of pages remaining, those changes
         * are not reflected in the output of this method until after the next step().
         * @returns The total number of pages in the source database.
         */
        int getTotalPageCount() noexcept;

        /** Returns the number of pages still to be backed up.
         * The values returned by this function is only updated by step().
         * If the source database is modified in a way that changes the size of
         * the source database or the number of pages remaining, those changes
         * are not reflected in the output of this method until after the next step().
         * @returns The number of pages still to be backed up.
         */
        int getRemainingPageCount() noexcept;

        /** Returns the pointer to the underlying sqlite3_backup object.
         */
        sqlite3_backup* getHandle() noexcept;

        private:
        using BackupHandle = std::unique_ptr<sqlite3_backup, decltype(&sqlite3_backup_finish)>;
        BackupHandle m_handle;

        const DBConnection *m_destination = nullptr;

        Backup(const Backup& other) = delete;
        Backup& operator=(Backup& other) = delete;
    };

    void SaveToDisk(const DBConnection &source, const std::string &filename);
}

#endif
