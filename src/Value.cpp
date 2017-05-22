#include "Value.h"


namespace SQLite
{
    Value::Value(const sqlite3_value * const value) :
        m_handle(sqlite3_value_dup(value), sqlite3_value_free)
    {}

    Value::Value(const Value &other) :
        m_handle(sqlite3_value_dup(other.m_handle.get()), sqlite3_value_free)
    {}

    Value::Value(Value &&other) :
        m_handle(std::move(other.m_handle))
    {}

    inline sqlite3_value * Value::getHandle() const noexcept {
        return m_handle.get();
    }

    Value& Value::operator=(const Value &other) {
        if (this != &other) {
            m_handle.reset(sqlite3_value_dup(other.m_handle.get()));
        }

        return *this;
    }

    Value& Value::operator=(Value && other) {
        assert(this != &other);
        m_handle = std::move(other.m_handle);
        return *this;
    }

    int Value::getInt() const noexcept
    {
        return sqlite3_value_int(getHandle());
    }

    int64_t Value::getInt64() const noexcept
    {
        return sqlite3_value_int64(getHandle());
    }

    unsigned int Value::getUInt() const noexcept
    {
        return static_cast<unsigned int>(getInt64());
    }

    double Value::getDouble() const noexcept
    {
        return sqlite3_value_double(getHandle());
    }

    const Blob Value::getBlob() const noexcept
    {
        const void *blob = sqlite3_value_blob(getHandle());
        return Blob(blob, getBytes());
    }

    const std::string Value::Value::getString() const noexcept
    {
        const char *txt = getText();
        return std::string(txt, getTextLength());
    }

    const std::u16string Value::Value::getU16String() const noexcept
    {
        const char16_t *txt = getText16();
        return std::u16string(txt, getText16Length());
    }

    int Value::getBytes() const noexcept
    {
        return sqlite3_value_bytes(getHandle());
    }

    Type Value::getType() const noexcept
    {
        return static_cast<Type>(sqlite3_value_type(getHandle()));
    }

    const char* Value::getText() const noexcept
    {
        return reinterpret_cast<const char *>(sqlite3_value_text(getHandle()));
    }

    /** Extracts a UTF-16 string in the native byte-order of the host machine.
     * Please pay attention to the fact that the pointer returned from:
     * getBlob(), getString(), or getWideString() can be invalidated by a subsequent call to
     * getBytes(), getBytes16(), getString(), getWideString().
     * */
    const char16_t* Value::getText16() const noexcept
    {
        return reinterpret_cast<const char16_t *>(sqlite3_value_text16(getHandle()));
    }

    int Value::getTextLength() const noexcept
    {
        // Make sure to only call this function after getText or getBlob was called
        // otherwise will not return correct value.
        return sqlite3_value_bytes(getHandle());
    }

    int Value::getText16Length() const noexcept
    {
        return sqlite3_value_bytes16(getHandle()) / sizeof(char16_t);
    }
}
