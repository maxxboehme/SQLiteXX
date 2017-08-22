/** @file */

#ifndef __SQLITEXX_SQLITE_DBCONNECTION_H__
#define __SQLITEXX_SQLITE_DBCONNECTION_H__

#include "Exception.h"
#include "Functions.h"
#include "Mutex.h"
#include "Open.h"

#include <sqlite3.h>

#include <cassert>
#include <chrono>
#include <memory>
#include <thread>


namespace SQLite
{
    /** Class that represents a connection to a database.
     * DBConnection is a wrapper around the "sqlite3" structure.
     */
    class DBConnection
    {
        public:

        /** Default constructor.
         */
        DBConnection() noexcept;

        /** Copy constructor.
         * @param[in] other another DBConnection object to use as source to initialize object with.
         */
        DBConnection(const DBConnection& other) noexcept;

        /** Copy constructor.
         * Constructs a DBConnection object with a copy of the contents of other using move semantics.
         * @param[in] other another DBConnection object to use as source to initialize object with.
         */
        DBConnection(DBConnection&& other) noexcept;

        /** Open the provided database UTF-8 filename.
         * @param[in] filename UTF-8 path/uri to the database database file
         * @param[in] mode     file opening options specified by combination of OpenMode flags
         * @param[in] timeout  amount of milliseconds to wait before returning SQLite::BusyException when a table is locked
         */
        DBConnection(
            const std::string& filename,
            OpenMode mode = OpenMode::ReadWrite | OpenMode::Create,
            const std::chrono::milliseconds timeout = DEFAULT_TIMEOUT);

        /** Open the provided database UTF-8 filename.
         * @param[in] filename UTF-8 path/uri to the database database file
         * @param[in] timeout  amount of milliseconds to wait before returning SQLite::BusyException when a table is locked
         */
        DBConnection(const std::string& filename, const std::chrono::milliseconds timeout);

        /** Open the provided database UTF-16 filename.
         * @param[in] filename UTF-16 path/uri to the database database file
         * @param[in] timeout  Amount of milliseconds to wait before returning SQLite::BusyException when a table is locked
         */
        DBConnection(const std::u16string& filename, const std::chrono::milliseconds timeout = DEFAULT_TIMEOUT);

        /** Copy assignment operator.
         * Replaces the contents with those of other.
         * @param[in] other another DBConnection object to use as source to initialize object with.
         * @returns *this
         */
        DBConnection& operator=(const DBConnection& other) noexcept;

        /** Move assignment operator.
         * Replaces the contents with those of other using move semantics.
         * @param[in] other another DBConnection object to use as source to initialize object with.
         * @returns *this
         */
        DBConnection& operator=(DBConnection&& other) noexcept;

        /** Create a purely in memory database.
         * @returns a purely in memory SQLite::DBConnection
         */
        static DBConnection memory();

        /** Create a purely in memory database with UTF-16 as the native byte order.
         * @returns a purely in memory SQLite::DBConnection
         */
        static DBConnection wideMemory();

        /** Returns a Mutex that serializes access to the database.
         * @returns A Mutex object for the database connection.
         */
        Mutex getMutex();

        /** Specifies if the DBConnection has a open database connection.
         * @returns Returns true if the DBConnection has a open database connection associated
         *          associated with it.
         */
        explicit operator bool() const noexcept;

        /** Returns pointer to the underlying "sqlite3" object.
         */
        sqlite3* getHandle() const noexcept;

        /** Open an SQLite database file as specified by the filename argument.
         * @param[in] filename path to SQLite file
         * @param[in] mode     specifies the privileges to use when opening the database.
         */
        void open(const std::string& filename, OpenMode mode = OpenMode::ReadWrite | OpenMode::Create);

        /** Open an SQLite database file as specified by the filname argument.
         * The database file will have UTF-16 native byte order.
         * @param[in] filename path to SQLite file
         */
        void open(const std::u16string& filename);

        /** Returns the rowid of the most recent successful "INSERT" into
         * a rowid table or virtual table on database connection.
         * @returns rowid of the most recent successful "INSERT" into the database, or 0 if there was none.
         */
        long long rowId() const noexcept;

        /** Used to add SQL functions or redefine the behavior of existing SQL functions.
         * @param[in] name            The name of the function to be used in an SQL query
         * @param[in] function        the implementation to the function
         * @param[in] isDeterministic specifies if the function will always return the same result given the same inputs within a single SQL statement.
         * @param[in] textEncoding    specifies the text encoding the SQL function prefers for its parameters.
         * @param[in] nargs           the number of arguments that the SQL function takes. -1 means the SQL function can take any number of arguments.
         */
        template <typename F>
        void createGeneralFunction(
            const std::string& name,
            F&& function,
            int isDeterministic = false,
            const TextEncoding textEncoding = SQLite::TextEncoding::UTF8,
            int nargs = -1)
        {
            using FunctionType = typename SQLiteFunctionTraits<F>::f_type;
            FunctionType *userFunction = new FunctionType(function);

            int flags = static_cast<int>(textEncoding);
            if (isDeterministic) {
                flags |= SQLITE_DETERMINISTIC;
            }

            int errorcode = sqlite3_create_function_v2(
                getHandle(),
                name.c_str(),
                nargs,
                static_cast<int>(flags),
                (void*)userFunction,
                &internal_general_scalar_function<FunctionType>,
                nullptr,
                nullptr,
                &internal_delete<FunctionType>);

            throwErrorCode(errorcode, "");
        }

        /** Used to add SQL functions or redefine the behavior of existing SQL functions.
         * @param[in] name            The name of the function to be used in an SQL query
         * @param[in] function        the implementation to the function
         * @param[in] isDeterministic specifies if the function will always return the same result given the same inputs within a single SQL statement.
         * @param[in] textEncoding    specifies the test encoding the SQL function prefers for its parameters
         */
        template <typename F>
        void createFunction(
            const std::string& name,
            F&& function,
            bool isDeterministic = false,
            const TextEncoding textEncoding = SQLite::TextEncoding::UTF8)
        {
            using FunctionType = typename function_traits<F>::f_type;
            FunctionType *userFunction = new FunctionType(function);

            int flags = static_cast<int>(textEncoding);
            if (isDeterministic) {
                flags |= SQLITE_DETERMINISTIC;
            }

            int errorcode = sqlite3_create_function_v2(
                getHandle(),
                name.c_str(),
                function_traits<F>::nargs,
                flags,
                (void*)userFunction,
                &internal_scalar_function<FunctionType>,
                nullptr,
                nullptr,
                &internal_delete<FunctionType>);

            throwErrorCode(errorcode, "");
        }

        /** Used to add  SQL aggregate functions or redefine the behavior of existing SQL aggregate functions.
         * @tparam A The class to use as the aggregate function.
         * @param[in] name The name of the aggregate function to be used in an SQL query
         * @param[in] isDeterministic specifies if the function will always return the same result given the same inputs within a single SQL statement.
         * @param[in] textEncoding    specifies the test encoding the SQL function prefers for its parameters
         */
        template <typename A>
        void createAggregate(
            const std::string& name,
            bool isDeterministic = false,
            const TextEncoding textEncoding = SQLite::TextEncoding::UTF8)
        {
            using StepFunctionType = function_traits<decltype(&A::step)>;
            aggregate_wrapper<A> *wrapper = new aggregate_wrapper<A>;

            int flags = static_cast<int>(textEncoding);
            if (isDeterministic) {
                flags |= SQLITE_DETERMINISTIC;
            }

            int errorcode = sqlite3_create_function_v2(
                getHandle(),
                name.c_str(),
                StepFunctionType::nargs,
                static_cast<int>(flags),
                (void*)wrapper,
                nullptr,
                &internal_step<aggregate_wrapper<A> >,
                &internal_final<aggregate_wrapper<A> >,
                &internal_dispose<aggregate_wrapper<A> >);

            throwErrorCode(errorcode, "");
        }

        template <typename F>
        void profile(F callback, void * const context = nullptr)
        {
            sqlite3_profile(getHandle(), callback, context);
        }

        private:
        using ConnectionHandle = std::shared_ptr<sqlite3>;
        ConnectionHandle m_handle;

        static const std::chrono::minutes DEFAULT_TIMEOUT;
    };
}

#endif
