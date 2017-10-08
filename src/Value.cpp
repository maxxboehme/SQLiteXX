#include "Value.h"


namespace sqlite
{
    value::value(const sqlite3_value* const value) :
        m_handle(sqlite3_value_dup(value), sqlite3_value_free)
    {}

    value::value(const value& other) :
        m_handle(sqlite3_value_dup(other.m_handle.get()), sqlite3_value_free)
    {}

    value::value(value&& other) :
        m_handle(std::move(other.m_handle))
    {}

    sqlite3_value* value::handle() const noexcept {
        return m_handle.get();
    }

    value& value::operator=(const value& other) {
        if (this != &other) {
            m_handle.reset(sqlite3_value_dup(other.m_handle.get()));
        }

        return *this;
    }

    value& value::operator=(value&& other) {
        assert(this != &other);
        m_handle = std::move(other.m_handle);
        return *this;
    }

    int value::as_int() const noexcept
    {
        return sqlite3_value_int(handle());
    }

    int64_t value::as_int64() const noexcept
    {
        return sqlite3_value_int64(handle());
    }

    unsigned int value::as_uint() const noexcept
    {
        return static_cast<unsigned int>(as_int64());
    }

    double value::as_double() const noexcept
    {
        return sqlite3_value_double(handle());
    }

    const blob value::as_blob() const noexcept
    {
        const void *blob = sqlite3_value_blob(handle());
        return sqlite::blob(blob, bytes());
    }

    const std::string value::as_string() const noexcept
    {
        const char *txt = as_text();
        return std::string(txt, text_length());
    }

    const std::u16string value::as_u16string() const noexcept
    {
        const char16_t *txt = as_text16();
        return std::u16string(txt, text16_length());
    }

    int value::bytes() const noexcept
    {
        return sqlite3_value_bytes(handle());
    }

    datatype value::type() const noexcept
    {
        return static_cast<datatype>(sqlite3_value_type(handle()));
    }

    const char* value::as_text() const noexcept
    {
        return reinterpret_cast<const char *>(sqlite3_value_text(handle()));
    }

    /** Extracts a UTF-16 string in the native byte-order of the host machine.
     * Please pay attention to the fact that the pointer returned from:
     * as_blob(), as_string(), or as_u16string() can be invalidated by a subsequent call to
     * bytes(), as_string(), as_u16tring().
     * */
    const char16_t* value::as_text16() const noexcept
    {
        return reinterpret_cast<const char16_t *>(sqlite3_value_text16(handle()));
    }

    int value::text_length() const noexcept
    {
        // Make sure to only call this function after as_text or as_blob was called
        // otherwise will not return correct value.
        return sqlite3_value_bytes(handle());
    }

    int value::text16_length() const noexcept
    {
        return sqlite3_value_bytes16(handle()) / sizeof(char16_t);
    }
}
