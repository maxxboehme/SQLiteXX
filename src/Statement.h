#ifndef __SQLITECXX_SQLITE_STATEMENT_H__
#define __SQLITECXX_SQLITE_STATEMENT_H__

#include "DBConnection.h"

#include <sqlite3.h>

#include <cassert>
#include <cstring>
#include <functional>
#include <iostream>
#include <map>
#include <memory>
#include <string>
#include <utility>
#include <vector>
#include <limits.h>

namespace SQLite
{
    enum class Type: int {
        Integer = SQLITE_INTEGER,
        Float   = SQLITE_FLOAT,
        Blob    = SQLITE_BLOB,
        Null    = SQLITE_NULL,
        Text    = SQLITE_TEXT,
    };

    enum class BindType: int {
        Static,
        Transient
    };

    class Blob
    {
        public:
        explicit Blob(const void * data, int size) :
            m_data(data != nullptr? new char[size]: nullptr),
            m_size(size)
        {
            assert(data == nullptr? size == 0: size > 0);
            memcpy(m_data.get(), data, size);
        }

        Blob(const Blob &other) :
            m_data(other.m_size == 0? nullptr: new char[other.m_size]),
            m_size(other.m_size)
        {
            memcpy(m_data.get(), other.m_data.get(), other.m_size);
        }

        Blob(Blob &&other) :
            m_data(std::move(other.m_data)),
            m_size(other.m_size)
        {}

        Blob& operator=(const Blob &other) {
            if (this != &other) {
                m_data.reset(other.m_size == 0? nullptr: new char[other.m_size]);
                memcpy(m_data.get(), other.m_data.get(), other.m_size);

                m_size = other.m_size;
            }

            return *this;
        }

        Blob& operator=(Blob &&other) {
            assert(this != &other);
            m_data = std::move(other.m_data);
            m_size = other.m_size;
            return *this;
        }

        const void * data() const {
            return m_data.get();
        }

        /** Used to get the size of the contained 'blob'.
         * @returns the size in bytes of the contained 'blob'
         */
        int size() const {
            return m_size;
        }

        private:
        std::unique_ptr<char[]> m_data;
        int m_size;
    };

    /** A SQLite dynamically typed value object, aka "sqlite3_value".
     * Value objects represent all values that can be stored in a database table.
     * A Value object may be either "protected" or "unprotected" which refers
     * to whether or not a mutex is held. An internal mutex is held for a protected value object but
     * not for an unprotected one. If SQLite is compiled to be single-threaded or if SQLite is run in one of reduced mutex modes
     * then there is no distinction between protected and unprotected sqlite3_value objects. A Value objects will always be
     * "protected" as it stores a sqlite3_value objects created from calling the sqlite3_value_dup() interface which produces a "protected"
     * "sqlite3_value" from an "unprotected" one.
     * Only use a Value object in the same thread as the SQL function that created it.
     */
    class Value
    {
        public:

        Value(const sqlite3_value * const value) :
            m_handle(sqlite3_value_dup(value), sqlite3_value_free)
        {}

        Value(const Value &other) :
            m_handle(sqlite3_value_dup(other.m_handle.get()), sqlite3_value_free)
        {}

        Value(Value &&other) :
            m_handle(std::move(other.m_handle))
        {}

        Value& operator=(const Value &other) {
            if (this != &other) {
                m_handle.reset(sqlite3_value_dup(other.m_handle.get()));
            }

            return *this;
        }

        Value& operator=(Value && other) {
            assert(this != &other);
            m_handle = std::move(other.m_handle);
            return *this;
        }

        /** Returns pointer to the underlying "sqlite3_value" object.
         * */
        sqlite3_value * getHandle() const noexcept {
            return m_handle.get();
        }

        int getInt() const noexcept
        {
            return sqlite3_value_int(getHandle());
        }

        int64_t getInt64() const noexcept
        {
            return sqlite3_value_int64(getHandle());
        }

        unsigned int getUInt() const noexcept
        {
            return static_cast<unsigned int>(getInt64());
        }

        double getDouble() const noexcept
        {
            return sqlite3_value_double(getHandle());
        }

        const Blob getBlob() const noexcept
        {
            const void *blob = sqlite3_value_blob(getHandle());
            return Blob(blob, getBytes());
        }

        const std::string getString() const noexcept
        {
            const char *txt = getText();
            return std::string(txt, getTextLength());
        }

        const std::u16string getU16String() const noexcept
        {
            const char16_t *txt = getText16();
            return std::u16string(txt, getText16Length());
        }

        int getBytes() const noexcept
        {
            return sqlite3_value_bytes(getHandle());
        }

        Type getType() const noexcept
        {
            return static_cast<Type>(sqlite3_value_type(getHandle()));
        }

        operator int() const
        {
            return getInt();
        }

        operator unsigned int() const
        {
            return static_cast<unsigned int>(getInt64());
        }

#if (LONG_MAX == INT_MAX) // sizeof(long)==4 means long is equivalent to int
        operator long() const
        {
            return getInt();
        }

        operator unsigned long() const
        {
            return getUInt();
        }
#else
        operator long() const
        {
            return getInt64();
        }
#endif

        operator long long() const
        {
            return getInt64();
        }

        operator double() const
        {
            return getDouble();
        }

        operator const Blob() const
        {
            return getBlob();
        }

        operator const std::string() const
        {
            return getString();
        }

        operator const std::u16string() const
        {
            return getU16String();
        }

        private:

        using ValueHandle = std::unique_ptr<sqlite3_value, decltype(&sqlite3_value_free)>;
        ValueHandle m_handle;

        const char* getText() const noexcept
        {
            return reinterpret_cast<const char *>(sqlite3_value_text(getHandle()));
        }

        /** Extracts a UTF-16 string in the native byte-order of the host machine.
         * Please pay attention to the fact that the pointer returned from:
         * getBlob(), getString(), or getWideString() can be invalidated by a subsequent call to
         * getBytes(), getBytes16(), getString(), getWideString().
         * */
        const char16_t* getText16() const noexcept
        {
            return reinterpret_cast<const char16_t *>(sqlite3_value_text16(getHandle()));
        }

        int getTextLength() const noexcept
        {
            // Make sure to only call this function after getText or getBlob was called
            // otherwise will not return correct value.
            return sqlite3_value_bytes(getHandle());
        }

        int getText16Length() const noexcept
        {
            return sqlite3_value_bytes16(getHandle()) / sizeof(char16_t);
        }

    };


    template <typename T>
    struct Reader
    {
        int getInt(const int column) const noexcept
        {
            return sqlite3_column_int(static_cast<T const *>(this)->getHandle(), column);
        }

        int64_t getInt64(const int column) const noexcept
        {
            return sqlite3_column_int64(static_cast<T const *>(this)->getHandle(), column);
        }

        unsigned int getUInt(const int column) const noexcept
        {
            return static_cast<unsigned int>(getInt64(column));
        }

        double getDouble(const int column) const noexcept
        {
            return sqlite3_column_double(static_cast<T const *>(this)->getHandle(), column);
        }

        const Blob getBlob(const int column) const noexcept
        {
            const void *blob = sqlite3_column_blob(static_cast<T const *>(this)->getHandle(), column);
            return Blob(blob, getBytes(column));
        }

        const std::string getString(const int column) const noexcept
        {
            const char *txt = getText(column);
            return std::string(txt, getTextLength(column));
        }

        const std::u16string getU16String(const int column) const noexcept
        {
            const char16_t *txt = getText16(column);
            return std::u16string(txt, getText16Length(column));
        }

        int getBytes(const int column) const noexcept
        {
            return sqlite3_column_bytes(static_cast<T const *>(this)->getHandle(), column);
        }

        Type getType(const int column) const noexcept
        {
            return static_cast<Type>(sqlite3_column_type(static_cast<T const *>(this)->getHandle(), column));
        }

        Value getValue(const int column) const noexcept
        {
            return Value(sqlite3_column_value(static_cast<T const *>(this)->getHandle(), column));
        }

        Value getValue(const char *name) const
        {
            const int column = getColumnIndex(name);
            return Value(sqlite3_column_value(static_cast<T const *>(this)->getHandle(), column));
        }

        int getColumnCount() const noexcept
        {
            return sqlite3_column_count(static_cast<T const *>(this)->getHandle());
        }

        const char * getColumnName(const int index) const noexcept
        {
            return sqlite3_column_name(static_cast<T const *>(this)->getHandle(), index);
        }

        const char * getColumnWideName(const int index) const noexcept
        {
            return sqlite3_column_name16(static_cast<T const *>(this)->getHandle(), index);
        }

        int getColumnIndex(const std::string &name) const
        {
            std::map<std::string, int> columnNamesToIndex;

            const int columnCount = getColumnCount();
            for (int i = 0; i < columnCount; ++i) {
                const char * name = sqlite3_column_name(static_cast<T const *>(this)->getHandle(), i);
                columnNamesToIndex[name] = i;
            }

            const std::map<std::string, int>::const_iterator index = columnNamesToIndex.find(name);
            if (index == columnNamesToIndex.end())
            {
                //TODO: Throw Exception
            }
            return index->second;
        }

        private:

        const char* getText(const int column) const noexcept
        {
            return reinterpret_cast<char const *>(sqlite3_column_text(
                    static_cast<T const *>(this)->getHandle(), column));
        }

        const char16_t* getText16(const int column) const noexcept
        {
            return reinterpret_cast<char16_t const *>(sqlite3_column_text16(
                    static_cast<T const *>(this)->getHandle(), column));
        }

        int getTextLength(const int column) const noexcept
        {
            return sqlite3_column_bytes(static_cast<T const *>(this)->getHandle(), column);
        }

        int getText16Length(const int column) const noexcept
        {
            return sqlite3_column_bytes16(static_cast<T const *>(this)->getHandle(), column) / sizeof(char16_t);
        }


    };


    /** Represents a returned row when stepping through a "SELECT" statement. */
    class Row : public Reader<Row>
    {
        sqlite3_stmt * m_statement = nullptr;

        public:

        /** Returns pointer to the underlying "sqlite3_stmt" object.
         * */
        sqlite3_stmt * getHandle() const noexcept
        {
            return m_statement;
        }

        Row(sqlite3_stmt * const statement) noexcept :
            m_statement(statement)
            {}

        Value operator[](int column) const {
            return Value(sqlite3_column_value(getHandle(), column));
        }

    };


    /** Represents a single SQL statement that has been compiled into binary
     * form and is ready to be evaluated, aka "sqlite3_stmt".
     */
    class Statement : public Reader<Statement>
    {
        public:
        Statement() noexcept :
            m_handle(nullptr, sqlite3_finalize),
            m_done(false)
        {}

        template <typename C, typename ... Values>
        Statement(
            DBConnection const & connection,
            C const * const text,
            Values && ... values) :
        m_handle(nullptr, sqlite3_finalize),
        m_done(false)
        {
            prepare(connection, text, std::forward<Values>(values) ...);
        }

        explicit operator bool() const noexcept
        {
            return static_cast<bool>(m_handle);
        }

        /** Returns pointer to the underlying "sqlite3_stmt" object.
         * */
        sqlite3_stmt* getHandle() const noexcept
        {
            return m_handle.get();
        }

        /** Turn an SQL query into byte code.
         * @param connection a successfully opened database connection
         * @param text the statement to be compiled, encoded as UTF-8
         * @param values possible bindings
         */
        template <typename ... Values>
        void prepare(
            DBConnection const & connection,
            char const * const text,
            Values && ... values)
        {
            internalPrepare(connection, sqlite3_prepare_v2, text, std::forward<Values>(values) ...);
        }

        /** Turn an SQL query into byte code.
         * @param connection a successfully opened database connection
         * @param text the statement to be compiled, encoded as UTF-16
         * @param values possible bindings
         */
        template <typename ... Values>
        void prepare(
            const DBConnection &connection,
            const char16_t * const text,
            Values && ... values)
        {
            internalPrepare(connection, sqlite3_prepare16_v2, text, std::forward<Values>(values) ...);
        }

        /** Evaluates a prepared statement.
         * This method can be called one or more times to evaluate the statement.
         * @returns true when there are more rows to iterate through and false when there are no more
         * @throws SQLite::Exception or a derived class
         */
        bool step() const
        {
            // This is to signal when the user has reached the end but
            // calls step again. The offical SQLite 3 API specifies that "sqlite3_step"
            // should not be called again after done before a reset. We just want it to
            // return false signaling done and be a NOP.
            if (m_done) return false;

            const int result =  sqlite3_step(getHandle());

            if (result == SQLITE_ROW) return true;
            if (result == SQLITE_DONE) {
                m_done = true;
                return false;
            }

            throwLastError();
            return false;
        }

        int execute() const
        {
            bool done = !step();
            // This variable is only used for the assert.
            // Doing this so there is no warnings on release builds.
            ((void)done);
            assert(done);

            return sqlite3_changes(sqlite3_db_handle(getHandle()));
        }

        /** Binds an integer value to a parameters in an SQL prepared statement.
         * @param index of the SQL parameter to be set
         * @param The integer value to bind to the parameter.
         **/
        void bind(const int index, const int value) const
        {
            if (SQLITE_OK != sqlite3_bind_int(getHandle(), index, value))
            {
                throwLastError();
            }
        }

        void bind(const int index, const double value) const
        {
            if (SQLITE_OK != sqlite3_bind_double(getHandle(), index, value))
            {
                throwLastError();
            }
        }

        void bind(const int index, const void * const value, const int size, BindType type = BindType::Transient) const
        {
            if (SQLITE_OK != sqlite3_bind_blob(getHandle(), index, value, size, type == BindType::Transient? SQLITE_TRANSIENT : SQLITE_STATIC))
            {
                throwLastError();
            }
        }

        void bind(const int index, const Blob &value) const
        {
            if (SQLITE_OK != sqlite3_bind_blob(getHandle(), index, value.data(), value.size(), SQLITE_TRANSIENT))
            {
                throwLastError();
            }
        }

        void bind(const int index, const char * const value, const int size = -1, BindType type = BindType::Transient) const
        {
            if (SQLITE_OK != sqlite3_bind_text(getHandle(), index, value, size, type == BindType::Transient? SQLITE_TRANSIENT : SQLITE_STATIC))
            {
                throwLastError();
            }
        }

        void bind(const int index, const char16_t * const value, const int size = -1, BindType type = BindType::Transient) const
        {
            if (SQLITE_OK != sqlite3_bind_text16(getHandle(), index, value, size, type == BindType::Transient? SQLITE_TRANSIENT : SQLITE_STATIC))
            {
                throwLastError();
            }
        }

        void bind(const int index, const std::string &value) const
        {
            bind(index, value.c_str(), value.size());
        }

        void bind(const int index, const std::u16string &value) const
        {
            bind(index, value.c_str(), value.size() * sizeof(char16_t));
        }

        void bind(const int index, std::string &&value) const
        {
            if (SQLITE_OK != sqlite3_bind_text(getHandle(), index, value.c_str(), value.size(), SQLITE_TRANSIENT))
            {
                throwLastError();
            }
        }

        void bind(const int index, std::u16string &&value) const
        {
            if (SQLITE_OK != sqlite3_bind_text16(getHandle(), index, value.c_str(), value.size() * sizeof(char16_t), SQLITE_TRANSIENT))
            {
                throwLastError();
            }
        }

        template <typename T>
        void bindByName(const char *name, T &&value)
        {
            const int index = sqlite3_bind_parameter_index(getHandle(), name);
            bind(index, value);
        }

        template <typename ... Values>
        void bindAll(Values && ... values) const
        {
            internalBind(1, std::forward<Values>(values) ...);
        }

        template <typename ... Values>
        void reset(Values && ... values) const
        {
            if (SQLITE_OK != sqlite3_reset(getHandle()))
            {
                throwLastError();
            }

            bindAll(values ...);
            m_done = false;
        }

        private:
        using StatementHandle = std::unique_ptr<sqlite3_stmt, decltype(&sqlite3_finalize)>;
        StatementHandle m_handle;

        mutable bool m_done;

        template <typename F, typename C, typename ... Values>
        void internalPrepare(
            const DBConnection & connection,
            F prepare,
            const C * const text,
            Values && ... values)
        {
            assert(connection);

            sqlite3_stmt *statement;
            if (SQLITE_OK != prepare(connection.getHandle(), text, -1, &statement, nullptr))
            {
                const int errcode = sqlite3_extended_errcode(connection.getHandle());
                const std::string message = sqlite3_errmsg(connection.getHandle());
                sqlite3_finalize(statement);
                throwErrorCode(errcode, message);
            }

            m_handle.reset(statement);
            bindAll(std::forward<Values>(values) ...);
        }

        void internalBind(int) const noexcept
        {}

        template <typename First, typename ... Rest>
        void internalBind(const int index, First && first, Rest && ... rest) const
        {
            bind(index, std::forward<First>(first));
            internalBind(index + 1, std::forward<Rest>(rest) ...);
        }

        void throwLastError() const
        {
            throwErrorCode(sqlite3_db_handle(getHandle()));
        }

        Statement(const Statement &other) = delete;
        Statement& operator=(Statement &other) = delete;

    };

    /** Helps when iterating over rows in a "SELECT" statement.
     */
    class RowIterator
    {
        public:
        RowIterator() noexcept = default;

        RowIterator(const Statement &statement) noexcept
        {
            if (statement.step())
            {
                m_statement = &statement;
            }
        }

        RowIterator & operator++() noexcept
        {
            if (!m_statement->step())
            {
                m_statement = nullptr;
            }

            return *this;
        }

        bool operator!=(const RowIterator &other) const noexcept
        {
            return m_statement != other.m_statement;
        }

        Row operator*() const noexcept
        {
            return Row(m_statement->getHandle());
        }

        private:
        const Statement *m_statement = nullptr;

    };

    inline RowIterator begin(const Statement &statement) noexcept
    {
        return RowIterator(statement);
    }

    inline RowIterator end(const Statement &statement) noexcept
    {
        // statement argument is unused because the end is a nullptr.
        // This will stop warnings
        (void)statement;

        return RowIterator();
    }

    template <typename C, typename ... Values>
    inline int Execute(
        const DBConnection & connection,
        const C * const text,
        Values && ... values)
    {
        return Statement(connection, text, std::forward<Values>(values) ...).execute();
    }

    template <typename ... Values>
    inline int Execute(
        const DBConnection & connection,
        const std::string &text,
        Values && ... values)
    {
        return Statement(connection, text.c_str(), std::forward<Values>(values) ...).execute();
    }

    template <typename ... Values>
    inline int Execute(
        const DBConnection & connection,
        const std::u16string text,
        Values && ... values)
    {
        return Statement(connection, text.c_str(), std::forward<Values>(values) ...).execute();
    }

    template <typename Call>
    inline int internal_execute_callback(
        void *data,
        int numColumns,
        char **colData,
        char **colNames)
    {
        Call *userCallback = static_cast<Call *>(data);

        std::vector<std::string> columnData;
        std::vector<std::string> columnName;
        for (int i = 0; i < numColumns; i++) {
            columnData.push_back(colData[i]? colData[i]: "");
            columnName.push_back(colNames[i]? colNames[i]: "");
        }

        (*userCallback)(columnData, columnName);
        return 0;
    }

    template <typename F, typename ... Args>
    inline void ExecuteCallback(
        const DBConnection &connection,
        const std::string &sql,
        F && callback,
        Args && ... args)
    {
// TODO: Decide whether to go with lambda or with bind
//        auto userCallback = [&](const std::vector<std::string> &colValues, const std::vector<std::string> &colNames) {
//            callback(colValues, colNames, std::forward<Args>(args)...);
//        };

        auto userCallback =
            std::bind(
               std::forward<F>(callback),
               std::placeholders::_1,
               std::placeholders::_2,
               std::forward<Args>(args)...);

        typedef decltype(userCallback) Call;

        char *errmsgPtr = nullptr;
        sqlite3_exec(connection.getHandle(), sql.c_str(), internal_execute_callback<Call>, (void *)&userCallback, nullptr);
        delete errmsgPtr;

        connection.throwLastError();
    }
}

#endif
