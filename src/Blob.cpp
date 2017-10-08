#include "Blob.h"


namespace sqlite
{
    blob::blob(const void* data, const size_t size) :
        m_data(data != nullptr? new char[size]: nullptr),
        m_size(size)
    {
        assert(data == nullptr? size == 0: size > 0);
        memcpy(m_data.get(), data, size);
    }

    blob::blob(const blob& other) :
        m_data(other.m_size == 0? nullptr: new char[other.m_size]),
        m_size(other.m_size)
    {
        memcpy(m_data.get(), other.m_data.get(), other.m_size);
    }

    blob::blob(blob &&other) :
        m_data(std::move(other.m_data)),
        m_size(other.m_size)
    {}

    blob& blob::operator=(const blob &other) {
        if (this != &other) {
            m_data.reset(other.m_size == 0? nullptr: new char[other.m_size]);
            memcpy(m_data.get(), other.m_data.get(), other.m_size);

            m_size = other.m_size;
        }

        return *this;
    }

    blob& blob::operator=(blob &&other) {
        assert(this != &other);
        m_data = std::move(other.m_data);
        m_size = other.m_size;
        return *this;
    }

    const void* blob::data() const {
        return m_data.get();
    }

    size_t blob::size() const {
        return m_size;
    }
}

