/** @file */

#ifndef __SQLITEXX_SQLITE_BACKUP_H__
#define __SQLITEXX_SQLITE_BACKUP_H__

#include "DBConnection.h"

#include <sqlite3.h>

#include <memory>

namespace sqlite
{
    /** Used to aid in the process of backing up a database.
     * The backup object records state information about an ongoing
     * online backup operation.  It us useful either for creating
     * backups of databases or for copying in-memory databases to or
     * from persistent files.
     */
    class backup
    {
        public:
        /** Construct a backup object.
         * @param source the database that will be the source of copied information
         * @param destination the database that will be backed up to
         * @param sourceName the source database name
         * @param destinationName the destination database name
         *
         * @throws sqlite::exception If an error occurs when initializing a backup
         *                           then an sqlite::exception will be raised.
         */
        backup(
            const dbconnection& source,
            const dbconnection& destination,
            const std::string& sourceName = "main",
            const std::string& destinationName = "main");

        /** Will copy a specified number of pages to the destination database.
         * If pages is negative, all remaining source pages are copied.
         *
         * @params pages the number of pages to copy from source to destination database.
         * @returns The returns value will be true if there are more pages to copy. False otherwise.
         *
         * @throws sqlite::exception If an error occurs when initializing a backup
         *                           then an sqlite::exception will be raised.
         */
        bool step(const int pages = -1);

        /** Returns the total number of pages in the source database.
         * The values returned by this function is only updated by step().
         * If the source database is modified in a way that changes the size of
         * the source database or the number of pages remaining, those changes
         * are not reflected in the output of this method until after the next step().
         * @returns The total number of pages in the source database.
         */
        int total_page_count() noexcept;

        /** Returns the number of pages still to be backed up.
         * The values returned by this function is only updated by step().
         * If the source database is modified in a way that changes the size of
         * the source database or the number of pages remaining, those changes
         * are not reflected in the output of this method until after the next step().
         * @returns The number of pages still to be backed up.
         */
        int remaining_page_count() noexcept;

        /** Returns the pointer to the underlying sqlite3_backup object.
         */
        sqlite3_backup* handle() noexcept;

        private:
        using backup_handle = std::unique_ptr<sqlite3_backup, decltype(&sqlite3_backup_finish)>;
        backup_handle m_handle;

        const dbconnection *m_destination = nullptr;

        backup(const backup& other) = delete;
        backup& operator=(backup& other) = delete;
    };

    void save(const dbconnection &source, const std::string &filename);
}

#endif
