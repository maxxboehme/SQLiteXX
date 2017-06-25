#ifndef __SQLITECXX_SQLITE_BLOB_H__
#define __SQLITECXX_SQLITE_BLOB_H__

#include <sqlite3.h>

#include <cassert>
#include <cstring>
#include <memory>
#include <utility>


namespace SQLite
{
    class Blob
    {
        public:

        Blob(const void * data, size_t size);
        Blob(const Blob &other);
        Blob(Blob &&other);
        Blob& operator=(const Blob &other);
        Blob& operator=(Blob &&other);
        const void* data() const;

        /** Used to get the size of the contained 'blob'.
         * @returns the size in bytes of the contained 'blob'
         */
        size_t size() const;

        private:
        std::unique_ptr<char[]> m_data;
        size_t m_size;
    };
}


#endif
