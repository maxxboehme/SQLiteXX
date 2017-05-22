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

    inline Blob::Blob(const void * data, int size) :
        m_data(data != nullptr? new char[size]: nullptr),
        m_size(size)
    {
        assert(data == nullptr? size == 0: size > 0);
        memcpy(m_data.get(), data, size);
    }

    inline Blob::Blob(const Blob &other) :
        m_data(other.m_size == 0? nullptr: new char[other.m_size]),
        m_size(other.m_size)
    {
        memcpy(m_data.get(), other.m_data.get(), other.m_size);
    }

    inline Blob::Blob(Blob &&other) :
        m_data(std::move(other.m_data)),
        m_size(other.m_size)
    {}

    inline Blob& Blob::operator=(const Blob &other) {
        if (this != &other) {
            m_data.reset(other.m_size == 0? nullptr: new char[other.m_size]);
            memcpy(m_data.get(), other.m_data.get(), other.m_size);

            m_size = other.m_size;
        }

        return *this;
    }

    inline Blob& Blob::operator=(Blob &&other) {
        assert(this != &other);
        m_data = std::move(other.m_data);
        m_size = other.m_size;
        return *this;
    }

    inline const void * Blob::data() const {
        return m_data.get();
    }

    inline int Blob::size() const {
        return m_size;
    }
}


#endif
