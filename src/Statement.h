#ifndef __SQLITECXX_SQLITE_STATEMENT_H__
#define __SQLITECXX_SQLITE_STATEMENT_H__

#include "Blob.h"
#include "DBConnection.h"
#include "SQLiteEnums.h"
#include "Value.h"

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
    template <typename T>
    class Reader
    {
        public:

        virtual ~Reader() {}

        int getInt(const int column) const noexcept
        {
            return sqlite3_column_int(static_cast<T const *>(this)->getHandle(), column);
        }

        int getInt(const std::string &name) const noexcept
        {
            const int column = getColumnIndex(name);
            return getInt(column);
        }

        int64_t getInt64(const int column) const noexcept
        {
            return sqlite3_column_int64(static_cast<T const *>(this)->getHandle(), column);
        }

        int64_t getInt64(const std::string &name) const noexcept
        {
            const int column = getColumnIndex(name);
            return getInt64(column);
        }

        unsigned int getUInt(const int column) const noexcept
        {
            return static_cast<unsigned int>(getInt64(column));
        }

        unsigned int getUInt(const std::string &name) const noexcept
        {
            const int column = getColumnIndex(name);
            return static_cast<unsigned int>(getInt64(column));
        }

        double getDouble(const int column) const noexcept
        {
            return sqlite3_column_double(static_cast<T const *>(this)->getHandle(), column);
        }

        double getDouble(const std::string &name) const noexcept
        {
            const int column = getColumnIndex(name);
            return getDouble(column);
        }

        const Blob getBlob(const int column) const noexcept
        {
            const void *blob = sqlite3_column_blob(static_cast<T const *>(this)->getHandle(), column);
            return Blob(blob, getBytes(column));
        }

        const Blob getBlob(const std::string &name) const noexcept
        {
            const int column = getColumnIndex(name);
            return getBlob(column);
        }

        const std::string getString(const int column) const noexcept
        {
            const char *txt = getText(column);
            return std::string(txt, getTextLength(column));
        }

        const std::string getString(const std::string &name) const noexcept
        {
            const int column = getColumnIndex(name);
            return getString(column);
        }

        const std::u16string getU16String(const int column) const noexcept
        {
            const char16_t *txt = getText16(column);
            return std::u16string(txt, getText16Length(column));
        }

        const std::u16string getU16String(const std::string &name) const noexcept
        {
            const int column = getColumnIndex(name);
            return getU16String(column);
        }

        int getBytes(const int column) const noexcept
        {
            return sqlite3_column_bytes(static_cast<T const *>(this)->getHandle(), column);
        }

        int getBytes(const std::string &name) const noexcept
        {
            const int column = getColumnIndex(name);
            return getBytes(column);
        }

        Type getType(const int column) const noexcept
        {
            return static_cast<Type>(sqlite3_column_type(static_cast<T const *>(this)->getHandle(), column));
        }

        Type getType(const std::string &name) const noexcept
        {
            const int column = getColumnIndex(name);
            return getType(column);
        }

        Value getValue(const int column) const noexcept
        {
            return Value(sqlite3_column_value(static_cast<T const *>(this)->getHandle(), column));
        }

        Value getValue(const std::string &name) const
        {
            const int column = getColumnIndex(name);
            return getValue(column);
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

        /**
         * @throws SQLiteXXException if no column with specified name was found
         */
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
                throw SQLiteXXException("No column was found with that name");
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
        public:

        /** Returns pointer to the underlying "sqlite3_stmt" object.
         * */
        sqlite3_stmt* getHandle() const noexcept
        {
            return m_statement;
        }

        Row(sqlite3_stmt * const statement) noexcept :
            m_statement(statement)
         {}

        Value operator[](int column) const
        {
            return getValue(column);
        }

        private:
        sqlite3_stmt * m_statement = nullptr;
    };


    /** Represents a single SQL statement that has been compiled into binary
     * form and is ready to be evaluated, aka "sqlite3_stmt".
     */
    class Statement : public Reader<Statement>
    {
        public:
        Statement() noexcept;

        template <typename ... Values>
        Statement(
            const DBConnection &connection,
            const std::string &text,
            Values && ... values) :
        m_handle(nullptr, sqlite3_finalize),
        m_done(false)
        {
            prepare(connection, text, std::forward<Values>(values) ...);
        }

        template <typename ... Values>
        Statement(
            const DBConnection &connection,
            const std::u16string &text,
            Values && ... values) :
        m_handle(nullptr, sqlite3_finalize),
        m_done(false)
        {
            prepare(connection, text, std::forward<Values>(values) ...);
        }

        operator bool() const noexcept;

        /** Returns pointer to the underlying "sqlite3_stmt" object.
         * */
        sqlite3_stmt* getHandle() const noexcept;

        /** Turn an SQL query into byte code.
         * @param connection a successfully opened database connection
         * @param text the statement to be compiled, encoded as UTF-8
         * @param values possible bindings
         */
        template <typename ... Values>
        void prepare(
            DBConnection const &connection,
            const std::string &text,
            Values && ... values)
        {
            internalPrepare(connection, sqlite3_prepare_v2, text.c_str(), std::forward<Values>(values) ...);
        }

        /** Turn an SQL query into byte code.
         * @param connection a successfully opened database connection
         * @param text the statement to be compiled, encoded as UTF-16
         * @param values possible bindings
         */
        template <typename ... Values>
        void prepare(
            const DBConnection &connection,
            const std::u16string &text,
            Values && ... values)
        {
            internalPrepare(connection, sqlite3_prepare16_v2, text.c_str(), std::forward<Values>(values) ...);
        }

        /** Evaluates a prepared statement.
         * This method can be called one or more times to evaluate the statement.
         * @returns true when there are more rows to iterate through and false when there are no more
         * @throws SQLite::Exception or a derived class
         */
        bool step() const;

        int execute() const;

        /** Binds an integer value to a parameters in an SQL prepared statement.
         * @param index of the SQL parameter to be set
         * @param The integer value to bind to the parameter.
         **/
        void bind(const int index, const int value) const;
        void bind(const int index, const double value) const;
        void bind(const int index, const void * const value, const int size, BindType type = BindType::Transient) const;
        void bind(const int index, const Blob &value) const;
        void bind(const int index, const char * const value, const int size = -1, BindType type = BindType::Transient) const;
        void bind(const int index, const char16_t * const value, const int size = -1, BindType type = BindType::Transient) const;
        void bind(const int index, const std::string &value) const;
        void bind(const int index, const std::u16string &value) const;
        void bind(const int index, std::string &&value) const;
        void bind(const int index, std::u16string &&value) const;

        template <typename T>
        void bindByName(const std::string &name, T &&value)
        {
            const int index = sqlite3_bind_parameter_index(getHandle(), name.c_str());
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

        void throwLastError() const;

        Statement(const Statement &other) = delete;
        Statement& operator=(Statement &other) = delete;

    };

    /** Helps when iterating over rows in a "SELECT" statement.
     */
    class RowIterator
    {
        public:
        RowIterator() noexcept = default;

        RowIterator(const Statement &statement) noexcept;

        RowIterator& operator++() noexcept;

        bool operator!=(const RowIterator &other) const noexcept;

        Row operator*() const noexcept;

        private:
        const Statement* m_statement = nullptr;

    };

    RowIterator begin(const Statement &statement) noexcept;

    RowIterator end(const Statement &statement) noexcept;

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
        const DBConnection &connection,
        const std::u16string &text,
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
        // TODO: Decide whether to go with lambda or with bind.
        // Using variadic templates in a lambda requires C++14 but could be more performant.
        // auto userCallback = [&](const std::vector<std::string> &colValues, const std::vector<std::string> &colNames) {
        //     callback(colValues, colNames, std::forward<Args>(args)...);
        // };

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

        throwErrorCode(connection.getHandle());
    }
}

#endif
