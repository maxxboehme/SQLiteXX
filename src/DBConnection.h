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


namespace sqlite
{
    /** Class that represents a connection to a database.
     * The class dbconnection is a wrapper around the "sqlite3" structure.
     */
    class dbconnection
    {
        public:

        /** Default constructor.
         */
        dbconnection() noexcept;

        /** Copy constructor.
         * @param[in] other another dbconnection object to use as source to initialize object with.
         */
        dbconnection(const dbconnection& other) noexcept;

        /** Copy constructor.
         * Constructs a dbconnection object with a copy of the contents of other using move semantics.
         * @param[in] other another dbconnection object to use as source to initialize object with.
         */
        dbconnection(dbconnection&& other) noexcept;

        /** Open the provided database UTF-8 filename.
         * @param[in] filename UTF-8 path/uri to the database database file
         * @param[in] mode     file opening options specified by combination of openmode flags
         * @param[in] timeout  amount of milliseconds to wait before returning sqlite::busy_exception when a table is locked
         */
        dbconnection(
            const std::string& filename,
            openmode mode = openmode::read_write | openmode::create,
            const std::chrono::milliseconds timeout = DEFAULT_TIMEOUT);

        /** Open the provided database UTF-8 filename.
         * @param[in] filename UTF-8 path/uri to the database database file
         * @param[in] timeout  amount of milliseconds to wait before returning sqlite::busy_exception when a table is locked
         */
        dbconnection(const std::string& filename, const std::chrono::milliseconds timeout);

        /** Open the provided database UTF-16 filename.
         * @param[in] filename UTF-16 path/uri to the database database file
         * @param[in] timeout  Amount of milliseconds to wait before returning sqlite::busy_exception when a table is locked
         */
        dbconnection(const std::u16string& filename, const std::chrono::milliseconds timeout = DEFAULT_TIMEOUT);

        /** Copy assignment operator.
         * Replaces the contents with those of other.
         * @param[in] other another dbconnection object to use as source to initialize object with.
         * @returns *this
         */
        dbconnection& operator=(const dbconnection& other) noexcept;

        /** Move assignment operator.
         * Replaces the contents with those of other using move semantics.
         * @param[in] other another dbconnection object to use as source to initialize object with.
         * @returns *this
         */
        dbconnection& operator=(dbconnection&& other) noexcept;

        /** Create a purely in memory database.
         * @returns a purely in memory sqlite::dbconnection
         */
        static dbconnection memory();

        /** Create a purely in memory database with UTF-16 as the native byte order.
         * @returns a purely in memory sqlite::dbconnection
         */
        static dbconnection wide_memory();

        /** Returns a mutex that serializes access to the database.
         * @returns A mutex object for the database connection.
         */
        sqlite::mutex mutex();

        /** Specifies if the dbconnection has a open database connection.
         * @returns Returns true if the dbconnection has a open database connection associated
         *          associated with it.
         */
        explicit operator bool() const noexcept;

        /** Returns pointer to the underlying "sqlite3" object.
         */
        sqlite3* handle() const noexcept;

        /** Open an SQLite database file as specified by the filename argument.
         * @param[in] filename path to SQLite file
         * @param[in] mode     specifies the privileges to use when opening the database.
         */
        void open(const std::string& filename, openmode mode = openmode::read_write | openmode::create);

        /** Open an SQLite database file as specified by the filname argument.
         * The database file will have UTF-16 native byte order.
         * @param[in] filename path to SQLite file
         */
        void open(const std::u16string& filename);

        /** Returns the rowid of the most recent successful "INSERT" into
         * a rowid table or virtual table on database connection.
         * @returns rowid of the most recent successful "INSERT" into the database, or 0 if there was none.
         */
        long long row_id() const noexcept;

        /** Used to add SQL functions or redefine the behavior of existing SQL functions.
         * @tparam F The function type to use to create the function.
         * @param[in] name             the name of the function to be used in an SQL query
         * @param[in] function         the implementation to the function
         * @param[in] is_deterministic specifies if the function will always return the same result given the same inputs within a single SQL statement.
         * @param[in] encoding         specifies the text encoding the SQL function prefers for its parameters.
         * @param[in] nargs            the number of arguments that the SQL function takes. -1 means the SQL function can take any number of arguments.
         */
        template <typename F>
        void create_general_function(
            const std::string& name,
            F&& function,
            int is_deterministic = false,
            const textencoding encoding = textencoding::utf8,
            int nargs = -1)
        {
            using FunctionType = typename SQLiteFunctionTraits<F>::f_type;
            FunctionType *userFunction = new FunctionType(function);

            int flags = static_cast<int>(encoding);
            if (is_deterministic) {
                flags |= SQLITE_DETERMINISTIC;
            }

            int errorcode = sqlite3_create_function_v2(
                handle(),
                name.c_str(),
                nargs,
                static_cast<int>(flags),
                (void*)userFunction,
                &internal_general_scalar_function<FunctionType>,
                nullptr,
                nullptr,
                &internal_delete<FunctionType>);

            throw_error_code(errorcode, "");
        }

        /** Used to add SQL functions or redefine the behavior of existing SQL functions.
         * @tparam F The function type to use to create the function.
         * @param[in] name             the name of the function to be used in an SQL query
         * @param[in] function         the implementation to the function
         * @param[in] is_deterministic specifies if the function will always return the same result given the same inputs within a single SQL statement.
         * @param[in] encoding         specifies the test encoding the SQL function prefers for its parameters
         */
        template <typename F>
        void create_function(
            const std::string& name,
            F&& function,
            bool is_deterministic = false,
            const textencoding encoding = textencoding::utf8)
        {
            using FunctionType = typename function_traits<F>::f_type;
            FunctionType *userFunction = new FunctionType(function);

            int flags = static_cast<int>(encoding);
            if (is_deterministic) {
                flags |= SQLITE_DETERMINISTIC;
            }

            int errorcode = sqlite3_create_function_v2(
                handle(),
                name.c_str(),
                function_traits<F>::nargs,
                flags,
                (void*)userFunction,
                &internal_scalar_function<FunctionType>,
                nullptr,
                nullptr,
                &internal_delete<FunctionType>);

            throw_error_code(errorcode, "");
        }

        /** Used to add  SQL aggregate functions or redefine the behavior of existing SQL aggregate functions.
         * @tparam A The class to use as the aggregate function.
         * @param[in] name             the name of the aggregate function to be used in an SQL query
         * @param[in] is_deterministic specifies if the function will always return the same result given the same inputs within a single SQL statement.
         * @param[in] encoding         specifies the test encoding the SQL function prefers for its parameters
         */
        template <typename A>
        void create_aggregate(
            const std::string& name,
            bool is_deterministic = false,
            const textencoding encoding = textencoding::utf8)
        {
            using StepFunctionType = function_traits<decltype(&A::step)>;
            aggregate_wrapper<A> *wrapper = new aggregate_wrapper<A>;

            int flags = static_cast<int>(encoding);
            if (is_deterministic) {
                flags |= SQLITE_DETERMINISTIC;
            }

            int errorcode = sqlite3_create_function_v2(
                handle(),
                name.c_str(),
                StepFunctionType::nargs,
                static_cast<int>(flags),
                (void*)wrapper,
                nullptr,
                &internal_step<aggregate_wrapper<A> >,
                &internal_final<aggregate_wrapper<A> >,
                &internal_dispose<aggregate_wrapper<A> >);

            throw_error_code(errorcode, "");
        }

        /** Used to add an SQL collation or redefine the behavior of existing SQL collations.
         * The function created should not throw an exception, if so the results is unknown.
         *
         * @tparam F The function type to use to create the function.
         * @param[in] name             the name of the function to be used in an SQL query
         * @param[in] function         the implementation to the function
         * @param[in] encoding         specifies the test encoding the SQL function prefers for its parameters
         */
        template <typename F>
        void create_collation(
            const std::string& name,
            F&& function,
            const textencoding encoding = textencoding::utf8)
        {
            using CollationType = typename collation_traits<F>::f_type;
            CollationType *userFunction = new CollationType(function);

            int flags = static_cast<int>(encoding);

            int errorcode = sqlite3_create_collation_v2(
                handle(),
                name.c_str(),
                flags,
                (void*)userFunction,
                &internal_collation_function<CollationType>,
                &internal_delete<CollationType>);

            throw_error_code(errorcode, "");
        }


        template <typename F>
        void profile(F&& callback, void* const context = nullptr)
        {
            sqlite3_profile(handle(), callback, context);
        }

        private:
        using connection_handle = std::shared_ptr<sqlite3>;
        connection_handle m_handle;

        static const std::chrono::minutes DEFAULT_TIMEOUT;
    };
}

#endif
