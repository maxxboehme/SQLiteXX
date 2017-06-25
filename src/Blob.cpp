#include "Blob.h"


namespace SQLite
{
    Blob::Blob(const void * data, size_t size) :
        m_data(data != nullptr? new char[size]: nullptr),
        m_size(size)
    {
        assert(data == nullptr? size == 0: size > 0);
        memcpy(m_data.get(), data, size);
    }

    Blob::Blob(const Blob &other) :
        m_data(other.m_size == 0? nullptr: new char[other.m_size]),
        m_size(other.m_size)
    {
        memcpy(m_data.get(), other.m_data.get(), other.m_size);
    }

    Blob::Blob(Blob &&other) :
        m_data(std::move(other.m_data)),
        m_size(other.m_size)
    {}

    Blob& Blob::operator=(const Blob &other) {
        if (this != &other) {
            m_data.reset(other.m_size == 0? nullptr: new char[other.m_size]);
            memcpy(m_data.get(), other.m_data.get(), other.m_size);

            m_size = other.m_size;
        }

        return *this;
    }

    Blob& Blob::operator=(Blob &&other) {
        assert(this != &other);
        m_data = std::move(other.m_data);
        m_size = other.m_size;
        return *this;
    }

    const void* Blob::data() const {
        return m_data.get();
    }

    size_t Blob::size() const {
        return m_size;
    }
}

