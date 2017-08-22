/** @file */

#ifndef __SQLITEXX_SQLITE_STATEMENT_H__
#define __SQLITEXX_SQLITE_STATEMENT_H__

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
    /** Base class used to help with reading "sqlite3_stmt" information.
     * This class is meant to be inherited from.
     */
    template <typename T>
    class Reader
    {
        public:

        virtual ~Reader() = default;

        /** Returns the specified column value as an integer.
         * @param[in] column position of the column to return
         * @returns value of column as integer
         */
        int getInt(const int column) const noexcept
        {
            return sqlite3_column_int(static_cast<T const *>(this)->getHandle(), column);
        }

        /** Returns the specified column value as an integer.
         * @param[in] name name of the column to return
         * @returns value of column as integer
         */
        int getInt(const std::string& name) const noexcept
        {
            const int column = getColumnIndex(name);
            return getInt(column);
        }

        /** Returns the specified column value as a 64-bit integer.
         * @param[in] column position of the column to return
         * @returns value of column as integer
         */
        int64_t getInt64(const int column) const noexcept
        {
            return sqlite3_column_int64(static_cast<T const *>(this)->getHandle(), column);
        }

        /** Returns the specified column value as a 64-bit integer.
         * @param[in] name name of the column to return
         * @returns value of column as integer
         */
        int64_t getInt64(const std::string& name) const noexcept
        {
            const int column = getColumnIndex(name);
            return getInt64(column);
        }

        /** Returns the specified column value as an unsigned integer.
         * @param[in] column position of the column to return
         * @returns value of column as integer
         */
        unsigned int getUInt(const int column) const noexcept
        {
            return static_cast<unsigned int>(getInt64(column));
        }

        /** Returns the specified column value as an unsigned integer.
         * @param[in] name name of the column to return
         * @returns value of column as integer
         */
        unsigned int getUInt(const std::string& name) const noexcept
        {
            const int column = getColumnIndex(name);
            return static_cast<unsigned int>(getInt64(column));
        }

        /** Returns the specified column value as a double.
         * @param[in] column position of the column to return
         * @returns value of column as integer
         */
        double getDouble(const int column) const noexcept
        {
            return sqlite3_column_double(static_cast<T const *>(this)->getHandle(), column);
        }

        /** Returns the specified column value as a double.
         * @param[in] name name of the column to return
         * @returns value of column as integer
         */
        double getDouble(const std::string& name) const noexcept
        {
            const int column = getColumnIndex(name);
            return getDouble(column);
        }

        /** Returns the specified column value as a Blob object.
         * @param[in] column position of the column to return
         * @returns value of column as integer
         */
        const Blob getBlob(const int column) const noexcept
        {
            const void *blob = sqlite3_column_blob(static_cast<T const *>(this)->getHandle(), column);
            return Blob(blob, getBytes(column));
        }

        /** Returns the specified column value as a Blob object.
         * @param[in] name name of the column to return
         * @returns value of column as integer
         */
        const Blob getBlob(const std::string& name) const noexcept
        {
            const int column = getColumnIndex(name);
            return getBlob(column);
        }

        /** Returns the specified column value as a string.
         * @param[in] column position of the column to return
         * @returns value of column as integer
         */
        const std::string getString(const int column) const noexcept
        {
            const char *txt = getText(column);
            return std::string(txt, getTextLength(column));
        }

        /** Returns the specified column value as a string.
         * @param[in] name name of the column to return
         * @returns value of column as integer
         */
        const std::string getString(const std::string& name) const noexcept
        {
            const int column = getColumnIndex(name);
            return getString(column);
        }

        /** Returns the specified column value as a UTF-16 string.
         * @param[in] column position of the column to return
         * @returns value of column as integer
         */
        const std::u16string getU16String(const int column) const noexcept
        {
            const char16_t *txt = getText16(column);
            return std::u16string(txt, getText16Length(column));
        }

        /** Returns the specified column value as a UTF-16 string.
         * @param[in] name name of the column to return
         * @returns value of column as integer
         */
        const std::u16string getU16String(const std::string& name) const noexcept
        {
            const int column = getColumnIndex(name);
            return getU16String(column);
        }

        /** Returns the specified column value as a Value object.
         * @param[in] column position of the column to return
         * @returns value of column as integer
         */
        Value getValue(const int column) const noexcept
        {
            return Value(sqlite3_column_value(static_cast<T const *>(this)->getHandle(), column));
        }

        /** Returns the specified column value as a Value object.
         * @param[in] name name of the column to return
         * @returns value of column as integer
         */
        Value getValue(const std::string& name) const
        {
            const int column = getColumnIndex(name);
            return getValue(column);
        }


        /** Returns the size in bytes of the column value.
         * @param[in] column position of the column to return
         * @returns the size in bytes of the column value
         */
        int getBytes(const int column) const noexcept
        {
            return sqlite3_column_bytes(static_cast<T const *>(this)->getHandle(), column);
        }

        /** Returns the size in bytes of the column value.
         * @param[in] name name of the column to return
         * @returns the size in bytes of the column value
         */
        int getBytes(const std::string& name) const noexcept
        {
            const int column = getColumnIndex(name);
            return getBytes(column);
        }

        /** Returns the type of the specified column.
         * @param[in] column position of the column to return
         * @returns The SQLiteXX::Type value for a column
         */
        Type getType(const int column) const noexcept
        {
            return static_cast<Type>(sqlite3_column_type(static_cast<T const *>(this)->getHandle(), column));
        }

        /** Returns the type of the specified column.
         * @param[in] name name of the column to return
         * @returns The SQLiteXX::Type value for a column
         */
        Type getType(const std::string& name) const noexcept
        {
            const int column = getColumnIndex(name);
            return getType(column);
        }

        /** Returns the number of columns in the result set returned by the prepared statement.
         * If this method returns 0, that means the prepared statment returns no data (for example an UPDATE).
         * @returns The number of columns in the result set.
         */
        int getColumnCount() const noexcept
        {
            return sqlite3_column_count(static_cast<T const *>(this)->getHandle());
        }

        /** Returns the name assigned to a particular column.
         * @param[in] index the position of the column
         * @returns The name of the specified column.
         */
        const char* getColumnName(const int index) const noexcept
        {
            return sqlite3_column_name(static_cast<T const *>(this)->getHandle(), index);
        }

        /** Returns the name assigned to a particular column.
         * @param[in] index the position of the column
         * @returns The name of the specified column.
         */
        const char16_t* getColumnWideName(const int index) const noexcept
        {
            return sqlite3_column_name16(static_cast<T const *>(this)->getHandle(), index);
        }

        /** Returns the position of a column with the specified name.
         * @returns The position of the column
         * @throws SQLiteXXException if no column with specified name was found
         */
        int getColumnIndex(const std::string& name) const
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


    /** Represents a returned row when stepping through a "SELECT" statement.
     */
    class Row : public Reader<Row>
    {
        public:

        /** Constructs a Row object from a sqlite3_stmt.
         */
        Row(sqlite3_stmt* const statement) noexcept :
            m_statement(statement)
         {}

        /** Returns pointer to the underlying "sqlite3_stmt" object.
         */
        sqlite3_stmt* getHandle() const noexcept
        {
            return m_statement;
        }

        /** Access specified element of a row.
         * @param[in] column position of the column to return
         * @returns Value object representing requested element
         */
        Value operator[](int column) const
        {
            return getValue(column);
        }

        private:
        sqlite3_stmt* m_statement = nullptr;
    };


    /** Represents a single SQL statement that has been compiled into binary
     * form and is ready to be evaluated, aka "sqlite3_stmt".
     */
    class Statement : public Reader<Statement>
    {
        public:
        /** Default constructor.
         * Constructs an un-prepared Statment object.
         */
        Statement() noexcept;

        /** Creates, prepares, and binds values into an SQL statement.
         * @param[in] connection a database connection to execute the statement on
         * @param[in] text       the SQL query
         * @param[in] values     possible values to bind to SQL query if containing bind parameters
         */
        template <typename ... Values>
        Statement(
            const DBConnection& connection,
            const std::string& text,
            Values&& ... values) :
        m_handle(nullptr, sqlite3_finalize),
        m_done(false)
        {
            prepare(connection, text, std::forward<Values>(values) ...);
        }

        /** Creates, prepares, and binds values into an SQL statement.
         *
         * @param[in] connection a database connection to execute the statement on
         * @param[in] text       UTF-16 encoded SQL query
         * @param[in] values     possible values to bind to SQL query if containing bind parameters
         */
        template <typename ... Values>
        Statement(
            const DBConnection& connection,
            const std::u16string& text,
            Values&& ... values) :
        m_handle(nullptr, sqlite3_finalize),
        m_done(false)
        {
            prepare(connection, text, std::forward<Values>(values) ...);
        }

        /** Used to specify if the Statement object has a prepared SQLite statement.
         * @returns The returns true if the Statement object has been assigned a SQLite prepared statement.
         */
        operator bool() const noexcept;

        /** Returns pointer to the underlying "sqlite3_stmt" object.
         * The returned sqlite3_stmt object pointer will be automatically deleted on
         * the destruction of the parent Statement object.
         **/
        sqlite3_stmt* getHandle() const noexcept;

        /** Turn an SQL query into byte code.
         * @param[in] connection a successfully opened database connection
         * @param[in] text       the statement to be compiled, encoded as UTF-8
         * @param[in] values     possible bindings
         */
        template <typename ... Values>
        void prepare(
            DBConnection const& connection,
            const std::string& text,
            Values&& ... values)
        {
            internalPrepare(connection, sqlite3_prepare_v2, text.c_str(), std::forward<Values>(values) ...);
        }

        /** Turn an SQL query into byte code.
         * @param[in] connection a successfully opened database connection
         * @param[in] text       the statement to be compiled, encoded as UTF-16
         * @param[in] values     possible bindings
         */
        template <typename ... Values>
        void prepare(
            const DBConnection& connection,
            const std::u16string& text,
            Values&& ... values)
        {
            internalPrepare(connection, sqlite3_prepare16_v2, text.c_str(), std::forward<Values>(values) ...);
        }

        /** Evaluates a prepared statement.
         * This method can be called one or more times to evaluate the statement.
         * @returns true when there are more rows to iterate through and false when there are no more
         * @throws SQLite::Exception or a derived class
         */
        bool step() const;

        /** Executes a prepared statement and will return the number of changes to the database.
         * @returns The number of changes to the database.
         **/
        int execute() const;

        /** Binds an integer value to a parameter in an SQL prepared statement.
         * @param[in] index specifies the index of the SQL parameter to be set
         * @param[in] value the value to bind to the parameter.
         **/
        void bind(const int index, const int value) const;

        /** Binds an double value to a parameter in an SQL prepared statement.
         * @param[in] index specifies the index of the SQL parameter to be set
         * @param[in] value the value to bind to the parameter.
         **/
        void bind(const int index, const double value) const;

        /** Binds an blob value to a parameter in an SQL prepared statement.
         * @param[in] index specifies the index of the SQL parameter to be set
         * @param[in] value the value to bind to the parameter.
         * @param[in] size  in bytes the size of the blob object
         * @param[in] type  the way to bind this parameter.
         **/
        void bind(const int index, const void* const value, const int size, BindType type = BindType::Transient) const;

        /** Binds an Blob value to a parameter in an SQL prepared statement.
         * @param[in] index specifies the index of the SQL parameter to be set
         * @param[in] value the value to bind to the parameter.
         **/
        void bind(const int index, const Blob& value) const;

        /** Binds an string value to a parameter in an SQL prepared statement.
         * @param[in] index specifies the index of the SQL parameter to be set
         * @param[in] value the value to bind to the parameter.
         * @param[in] size  the number of bytes of the value.
         * @param[in] type  the way to bind this parameter.
         **/
        void bind(const int index, const char* const value, const int size = -1, BindType type = BindType::Transient) const;

        /** Binds an UTF-16 string value to a parameter in an SQL prepared statement.
         * @param[in] index specifies the index of the SQL parameter to be set
         * @param[in] value the value to bind to the parameter.
         * @param[in] size  the number of bytes of the value.
         * @param[in] type  the way to bind this parameter.
         **/
        void bind(const int index, const char16_t* const value, const int size = -1, BindType type = BindType::Transient) const;

        /** Binds an string value to a parameter in an SQL prepared statement.
         * @param[in] index specifies the index of the SQL parameter to be set
         * @param[in] value the value to bind to the parameter.
         **/
        void bind(const int index, const std::string& value) const;

        /** Binds an UTF-16 string value to a parameter in an SQL prepared statement.
         * @param[in] index specifies the index of the SQL parameter to be set
         * @param[in] value the value to bind to the parameter.
         **/
        void bind(const int index, const std::u16string& value) const;

        /** Binds an value to a parameter in an SQL prepared statement.
         * @param[in] name  specifies the name of the SQL parameter to be set
         * @param[in] value the value to bind to the parameter.
         **/
        template <typename T>
        void bindByName(const std::string& name, T&& value)
        {
            const int index = sqlite3_bind_parameter_index(getHandle(), name.c_str());
            bind(index, value);
        }

        /** Binds values to parameters in an SQL prepared statement.
         * @param[in] values the values to bind to SQL parameters
         **/
        template <typename ... Values>
        void bindAll(Values&& ... values) const
        {
            internalBind(1, std::forward<Values>(values) ...);
        }

        /** Resets all SQL parameters to NULL.
         * @param[in] values Possible values to to bind to SQL parameters.
         */
        template <typename ... Values>
        void clearBindings(Values&& ... values) const
        {
            if (SQLITE_OK != sqlite3_clear_bindings(getHandle()))
            {
                throwLastError();
            }

            bindAll(values ...);
        }


        /** Resets a prepared statement object back to its initial state.
         * Any SQL statment parameters that had values bound to them using the bind method return their values.
         * Use clearBindings to reset the bindings.
         */
        void reset() const
        {
            if (SQLITE_OK != sqlite3_reset(getHandle()))
            {
                throwLastError();
            }
            m_done = false;
        }

        private:
        using StatementHandle = std::unique_ptr<sqlite3_stmt, decltype(&sqlite3_finalize)>;
        StatementHandle m_handle;

        mutable bool m_done;

        template <typename F, typename C, typename ... Values>
        void internalPrepare(
            const DBConnection& connection,
            F prepare,
            const C * const text,
            Values&& ... values)
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
        void internalBind(const int index, First&& first, Rest&& ... rest) const
        {
            bind(index, std::forward<First>(first));
            internalBind(index + 1, std::forward<Rest>(rest) ...);
        }

        void throwLastError() const;

        Statement(const Statement& other) = delete;
        Statement& operator=(Statement& other) = delete;

    };

    /** Helps when iterating over rows in a "SELECT" statement.
     * RowIterator is a [InputIterator](http://en.cppreference.com/w/cpp/concept/InputIterator) and can read data from the pointed to SQLite row.
     */
    class RowIterator
    {
        public:
        /** Default constructor.
         */
        RowIterator() noexcept = default;

        /** Construct a RowIterator object from a Statment object.
         * A RowIterator should only be constructect on an Statment object that is doing a SQL "SELECT" query.
         * @param statement the statement object to construct the iterator from.
         */
        RowIterator(const Statement& statement) noexcept;

        /** Increment iterator to the next Row object of the Statement.
         */
        RowIterator& operator++() noexcept;

        /** Comparison operation.
         */
        bool operator!=(const RowIterator& other) const noexcept;

        /** Dereference operation.
         * Dereferencing a RowIterator object will return a Row object.
         */
        Row operator*() const noexcept;

        private:
        const Statement* m_statement = nullptr;

    };

    /** Returns an iterator to the first row of a Statement.
     * @param[in] statement the statement to get the first row of
     * @returns The first row of an executed SQL statement.
     */
    RowIterator begin(const Statement& statement) noexcept;

    /** Returns an iterator to the end.
     * @param[in] statement the statement to get the end of
     * @returns The value that signifies that there are no more rows to iterate over.
     */
    RowIterator end(const Statement& statement) noexcept;

    /** Executes an SQL query on a database connection.
     *
     * @param[in] connection a database connection to execute the statement on
     * @param[in] text       the SQL query
     * @param[in] values     possible values to bind to SQL query if containing bind parameters
     */
    template <typename ... Values>
    inline int Execute(
        const DBConnection& connection,
        const std::string& text,
        Values&& ... values)
    {
        return Statement(connection, text.c_str(), std::forward<Values>(values) ...).execute();
    }

    /** Executes an SQL query on a database connection.
     *
     * @param[in] connection a database connection to execute the statement on
     * @param[in] text       UTF-16 SQL query
     * @param[in] values     possible values to bind to SQL query if containing bind parameters
     */
    template <typename ... Values>
    inline int Execute(
        const DBConnection& connection,
        const std::u16string& text,
        Values&& ... values)
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
        const DBConnection& connection,
        const std::string& sql,
        F&& callback,
        Args&& ... args)
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
