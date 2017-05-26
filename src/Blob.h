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

        explicit Blob(const void * data, int size);
        Blob(const Blob &other);
        Blob(Blob &&other);
        Blob& operator=(const Blob &other);
        Blob& operator=(Blob &&other);
        const void * data() const;

        /** Used to get the size of the contained 'blob'.
         * @returns the size in bytes of the contained 'blob'
         */
        int size() const;

        private:
        std::unique_ptr<char[]> m_data;
        int m_size;
    };
}


#endif
