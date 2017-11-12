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

namespace sqlite
{
    /** Base class used to help with reading "sqlite3_stmt" information.
     * This class is meant to be inherited from.
     */
    template <typename T>
    class reader
    {
        public:

        virtual ~reader() = default;

        /** Returns the specified column value as an integer.
         * @param[in] column position of the column to return
         * @returns value of column as integer
         */
        int get_int(const int column) const noexcept
        {
            return sqlite3_column_int(static_cast<T const *>(this)->handle(), column);
        }

        /** Returns the specified column value as an integer.
         * @param[in] name name of the column to return
         * @returns value of column as integer
         */
        int get_int(const std::string& name) const noexcept
        {
            const int column = get_column_index(name);
            return get_int(column);
        }

        /** Returns the specified column value as a 64-bit integer.
         * @param[in] column position of the column to return
         * @returns value of column as integer
         */
        int64_t get_int64(const int column) const noexcept
        {
            return sqlite3_column_int64(static_cast<T const *>(this)->handle(), column);
        }

        /** Returns the specified column value as a 64-bit integer.
         * @param[in] name name of the column to return
         * @returns value of column as integer
         */
        int64_t get_int64(const std::string& name) const noexcept
        {
            const int column = get_column_index(name);
            return get_int64(column);
        }

        /** Returns the specified column value as an unsigned integer.
         * @param[in] column position of the column to return
         * @returns value of column as integer
         */
        unsigned int get_uint(const int column) const noexcept
        {
            return static_cast<unsigned int>(get_int64(column));
        }

        /** Returns the specified column value as an unsigned integer.
         * @param[in] name name of the column to return
         * @returns value of column as integer
         */
        unsigned int get_uint(const std::string& name) const noexcept
        {
            const int column = get_column_index(name);
            return get_uint(column);
        }

        /** Returns the specified column value as a double.
         * @param[in] column position of the column to return
         * @returns value of column as integer
         */
        double get_double(const int column) const noexcept
        {
            return sqlite3_column_double(static_cast<T const *>(this)->handle(), column);
        }

        /** Returns the specified column value as a double.
         * @param[in] name name of the column to return
         * @returns value of column as integer
         */
        double get_double(const std::string& name) const noexcept
        {
            const int column = get_column_index(name);
            return get_double(column);
        }

        /** Returns the specified column value as a blob object.
         * @param[in] column position of the column to return
         * @returns value of column as integer
         */
        const blob get_blob(const int column) const noexcept
        {
            const void *blob = sqlite3_column_blob(static_cast<T const *>(this)->handle(), column);
            return sqlite::blob(blob, get_bytes(column));
        }

        /** Returns the specified column value as a Blob object.
         * @param[in] name name of the column to return
         * @returns value of column as integer
         */
        const blob get_blob(const std::string& name) const noexcept
        {
            const int column = get_column_index(name);
            return get_blob(column);
        }

        /** Returns the specified column value as a string.
         * @param[in] column position of the column to return
         * @returns value of column as integer
         */
        const std::string get_string(const int column) const noexcept
        {
            const char *txt = get_text(column);
            return std::string(txt, get_text_length(column));
        }

        /** Returns the specified column value as a string.
         * @param[in] name name of the column to return
         * @returns value of column as integer
         */
        const std::string get_string(const std::string& name) const noexcept
        {
            const int column = get_column_index(name);
            return get_string(column);
        }

        /** Returns the specified column value as a UTF-16 string.
         * @param[in] column position of the column to return
         * @returns value of column as integer
         */
        const std::u16string get_u16string(const int column) const noexcept
        {
            const char16_t *txt = get_text16(column);
            return std::u16string(txt, get_text16_length(column));
        }

        /** Returns the specified column value as a UTF-16 string.
         * @param[in] name name of the column to return
         * @returns value of column as integer
         */
        const std::u16string get_u16string(const std::string& name) const noexcept
        {
            const int column = get_column_index(name);
            return get_u16string(column);
        }

        /** Returns the specified column value as a value object.
         * @param[in] column position of the column to return
         * @returns value of column as integer
         */
        value get_value(const int column) const noexcept
        {
            return value(sqlite3_column_value(static_cast<T const *>(this)->handle(), column));
        }

        /** Returns the specified column value as a value object.
         * @param[in] name name of the column to return
         * @returns value of column as integer
         */
        value get_value(const std::string& name) const
        {
            const int column = get_column_index(name);
            return get_value(column);
        }


        /** Returns the size in bytes of the column value.
         * @param[in] column position of the column to return
         * @returns the size in bytes of the column value
         */
        int get_bytes(const int column) const noexcept
        {
            return sqlite3_column_bytes(static_cast<T const *>(this)->handle(), column);
        }

        /** Returns the size in bytes of the column value.
         * @param[in] name name of the column to return
         * @returns the size in bytes of the column value
         */
        int get_bytes(const std::string& name) const noexcept
        {
            const int column = get_column_index(name);
            return get_bytes(column);
        }

        /** Returns the type of the specified column.
         * @param[in] column position of the column to return
         * @returns The sqlite::datatype value for a column
         */
        datatype get_type(const int column) const noexcept
        {
            return static_cast<datatype>(sqlite3_column_type(static_cast<T const *>(this)->handle(), column));
        }

        /** Returns the type of the specified column.
         * @param[in] name name of the column to return
         * @returns The sqlite::datatype value for a column
         */
        datatype get_type(const std::string& name) const noexcept
        {
            const int column = get_column_index(name);
            return get_type(column);
        }

        /** Returns the number of columns in the result set returned by the prepared statement.
         * If this method returns 0, that means the prepared statment returns no data (for example an UPDATE).
         * @returns The number of columns in the result set.
         */
        int column_count() const noexcept
        {
            return sqlite3_column_count(static_cast<T const *>(this)->handle());
        }

        /** Returns the name assigned to a particular column.
         * @param[in] index the position of the column
         * @returns The name of the specified column.
         */
        const char* get_column_name(const int index) const noexcept
        {
            return sqlite3_column_name(static_cast<T const *>(this)->handle(), index);
        }

        /** Returns the name assigned to a particular column.
         * @param[in] index the position of the column
         * @returns The name of the specified column.
         */
        const char16_t* get_column_wide_name(const int index) const noexcept
        {
            return sqlite3_column_name16(static_cast<T const *>(this)->handle(), index);
        }

        /** Returns the position of a column with the specified name.
         * @returns The position of the column
         * @throws SQLiteXXException if no column with specified name was found
         */
        int get_column_index(const std::string& name) const
        {
            std::map<std::string, int> columnNamesToIndex;

            const int columnCount = column_count();
            for (int i = 0; i < columnCount; ++i) {
                const char* columnName = sqlite3_column_name(static_cast<T const *>(this)->handle(), i);
                columnNamesToIndex[columnName] = i;
            }

            const std::map<std::string, int>::const_iterator index = columnNamesToIndex.find(name);
            if (index == columnNamesToIndex.end())
            {
                throw SQLiteXXException("No column was found with that name");
            }
            return index->second;
        }

        private:

        const char* get_text(const int column) const noexcept
        {
            return reinterpret_cast<char const *>(sqlite3_column_text(
                    static_cast<T const *>(this)->handle(), column));
        }

        const char16_t* get_text16(const int column) const noexcept
        {
            return reinterpret_cast<char16_t const *>(sqlite3_column_text16(
                    static_cast<T const *>(this)->handle(), column));
        }

        int get_text_length(const int column) const noexcept
        {
            return sqlite3_column_bytes(static_cast<T const *>(this)->handle(), column);
        }

        int get_text16_length(const int column) const noexcept
        {
            return sqlite3_column_bytes16(static_cast<T const *>(this)->handle(), column) / sizeof(char16_t);
        }
    };


    /** Represents a returned row when stepping through a "SELECT" statement.
     */
    class row : public reader<row>
    {
        public:

        /** Constructs a row object from a sqlite3_stmt.
         */
        row(sqlite3_stmt* const statement) noexcept :
            m_statement(statement)
         {}

        /** Returns pointer to the underlying "sqlite3_stmt" object.
         */
        sqlite3_stmt* handle() const noexcept
        {
            return m_statement;
        }

        /** Access specified element of a row.
         * @param[in] column position of the column to return
         * @returns value object representing requested element
         */
        value operator[](int column) const
        {
            return get_value(column);
        }

        /** Access specified element of a row.
         * @param[in] name column name of the column to return
         * @returns value object representing requested element
         */
        value operator[](const std::string& name) const
        {
            return get_value(name);
        }

        private:
        sqlite3_stmt* m_statement = nullptr;
    };


    /** Represents a single SQL statement that has been compiled into binary
     * form and is ready to be evaluated, aka "sqlite3_stmt".
     */
    class statement : public reader<statement>
    {
        public:
        /** Default constructor.
         * Constructs an un-prepared statement object.
         */
        statement() noexcept;

        /** Creates, prepares, and binds values into an SQL statement.
         * @param[in] connection a database connection to execute the statement on
         * @param[in] text       the SQL query
         * @param[in] values     possible values to bind to SQL query if containing bind parameters
         */
        template <typename ... Values>
        statement(
            const dbconnection& connection,
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
        statement(
            const dbconnection& connection,
            const std::u16string& text,
            Values&& ... values) :
        m_handle(nullptr, sqlite3_finalize),
        m_done(false)
        {
            prepare(connection, text, std::forward<Values>(values) ...);
        }

        /** Used to specify if the statement object has a prepared SQLite statement.
         * @returns The returns true if the statement object has been assigned a SQLite prepared statement.
         */
        operator bool() const noexcept;

        /** Returns pointer to the underlying "sqlite3_stmt" object.
         * The returned sqlite3_stmt object pointer will be automatically deleted on
         * the destruction of the parent statement object.
         **/
        sqlite3_stmt* handle() const noexcept;

        /** Turn an SQL query into byte code.
         * @param[in] connection a successfully opened database connection
         * @param[in] text       the statement to be compiled, encoded as UTF-8
         * @param[in] values     possible bindings
         */
        template <typename ... Values>
        void prepare(
            dbconnection const& connection,
            const std::string& text,
            Values&& ... values)
        {
            internal_prepare(connection, sqlite3_prepare_v2, text.c_str(), std::forward<Values>(values) ...);
        }

        /** Turn an SQL query into byte code.
         * @param[in] connection a successfully opened database connection
         * @param[in] text       the statement to be compiled, encoded as UTF-16
         * @param[in] values     possible bindings
         */
        template <typename ... Values>
        void prepare(
            const dbconnection& connection,
            const std::u16string& text,
            Values&& ... values)
        {
            internal_prepare(connection, sqlite3_prepare16_v2, text.c_str(), std::forward<Values>(values) ...);
        }

        /** Evaluates a prepared statement.
         * This method can be called one or more times to evaluate the statement.
         * @returns true when there are more rows to iterate through and false when there are no more
         * @throws sqlite::exception or a derived class
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
        void bind(const int index, const void* const value, const int size, bindtype type = bindtype::transiently) const;

        /** Binds an blob value to a parameter in an SQL prepared statement.
         * @param[in] index specifies the index of the SQL parameter to be set
         * @param[in] value the value to bind to the parameter.
         **/
        void bind(const int index, const blob& value) const;

        /** Binds an string value to a parameter in an SQL prepared statement.
         * @param[in] index specifies the index of the SQL parameter to be set
         * @param[in] value the value to bind to the parameter.
         * @param[in] size  the number of bytes of the value.
         * @param[in] type  the way to bind this parameter.
         **/
        void bind(const int index, const char* const value, const int size = -1, bindtype type = bindtype::transiently) const;

        /** Binds an UTF-16 string value to a parameter in an SQL prepared statement.
         * @param[in] index specifies the index of the SQL parameter to be set
         * @param[in] value the value to bind to the parameter.
         * @param[in] size  the number of bytes of the value.
         * @param[in] type  the way to bind this parameter.
         **/
        void bind(const int index, const char16_t* const value, const int size = -1, bindtype type = bindtype::transiently) const;

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
        void bind_name(const std::string& name, T&& value)
        {
            const int index = sqlite3_bind_parameter_index(handle(), name.c_str());
            bind(index, value);
        }

        /** Binds values to parameters in an SQL prepared statement.
         * @param[in] values the values to bind to SQL parameters
         **/
        template <typename ... Values>
        void bind_all(Values&& ... values) const
        {
            internal_bind(1, std::forward<Values>(values) ...);
        }

        /** Resets all SQL parameters to NULL.
         * @param[in] values Possible values to to bind to SQL parameters.
         */
        template <typename ... Values>
        void clear_bindings(Values&& ... values) const
        {
            if (SQLITE_OK != sqlite3_clear_bindings(handle()))
            {
                throw_last_error();
            }

            bind_all(values ...);
        }


        /** Resets a prepared statement object back to its initial state.
         * Any SQL statment parameters that had values bound to them using the bind method return their values.
         * Use clearBindings to reset the bindings.
         */
        void reset() const
        {
            if (SQLITE_OK != sqlite3_reset(handle()))
            {
                throw_last_error();
            }
            m_done = false;
        }

        private:
        using statement_handle = std::unique_ptr<sqlite3_stmt, decltype(&sqlite3_finalize)>;
        statement_handle m_handle;

        mutable bool m_done;

        template <typename F, typename C, typename ... Values>
        void internal_prepare(
            const dbconnection& connection,
            F prepare,
            const C * const text,
            Values&& ... values)
        {
            assert(connection);

            sqlite3_stmt *statement;
            if (SQLITE_OK != prepare(connection.handle(), text, -1, &statement, nullptr))
            {
                const int errcode = sqlite3_extended_errcode(connection.handle());
                const std::string message = sqlite3_errmsg(connection.handle());
                sqlite3_finalize(statement);
                throw_error_code(errcode, message);
            }

            m_handle.reset(statement);
            bind_all(std::forward<Values>(values) ...);
        }

        void internal_bind(int) const noexcept
        {}

        template <typename First, typename ... Rest>
        void internal_bind(const int index, First&& first, Rest&& ... rest) const
        {
            bind(index, std::forward<First>(first));
            internal_bind(index + 1, std::forward<Rest>(rest) ...);
        }

        void throw_last_error() const;

        statement(const statement& other) = delete;
        statement& operator=(statement& other) = delete;

    };

    /** Helps when iterating over rows in a "SELECT" statement.
     * row_iterator is a [InputIterator](http://en.cppreference.com/w/cpp/concept/InputIterator) and can read data from the pointed to SQLite row.
     */
    class row_iterator
    {
        public:
        /** Default constructor.
         */
        row_iterator() noexcept = default;

        /** Construct a row_iterator object from a statement object.
         * A row_iterator should only be constructect on an statement object that is doing a SQL "SELECT" query.
         * @param statement the statement object to construct the iterator from.
         */
        row_iterator(const statement& statement) noexcept;

        /** Increment iterator to the next row object of the statement.
         */
        row_iterator& operator++() noexcept;

        /** Comparison operation.
         */
        bool operator!=(const row_iterator& other) const noexcept;

        /** Dereference operation.
         * Dereferencing a row_iterator object will return a row object.
         */
        row operator*() const noexcept;

        private:
        const statement* m_statement = nullptr;

    };

    /** Returns an iterator to the first row of a statement.
     * @param[in] statement the statement to get the first row of
     * @returns The first row of an executed SQL statement.
     */
    row_iterator begin(const statement& statement) noexcept;

    /** Returns an iterator to the end.
     * @param[in] statement the statement to get the end of
     * @returns The value that signifies that there are no more rows to iterate over.
     */
    row_iterator end(const statement& statement) noexcept;

    /** Executes an SQL query on a database connection.
     *
     * @param[in] connection a database connection to execute the statement on
     * @param[in] text       the SQL query
     * @param[in] values     possible values to bind to SQL query if containing bind parameters
     */
    template <typename ... Values>
    inline int execute(
        const dbconnection& connection,
        const std::string& text,
        Values&& ... values)
    {
        return statement(connection, text.c_str(), std::forward<Values>(values) ...).execute();
    }

    /** Executes an SQL query on a database connection.
     *
     * @param[in] connection a database connection to execute the statement on
     * @param[in] text       UTF-16 SQL query
     * @param[in] values     possible values to bind to SQL query if containing bind parameters
     */
    template <typename ... Values>
    inline int execute(
        const dbconnection& connection,
        const std::u16string& text,
        Values&& ... values)
    {
        return statement(connection, text.c_str(), std::forward<Values>(values) ...).execute();
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
    inline void execute_callback(
        const dbconnection& connection,
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
        sqlite3_exec(connection.handle(), sql.c_str(), internal_execute_callback<Call>, (void *)&userCallback, nullptr);
        delete errmsgPtr;

        throw_error_code(connection.handle());
    }
}

#endif
