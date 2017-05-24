#ifndef __SQLITECXX_SQLITE_DBCONNECTION_H__
#define __SQLITECXX_SQLITE_DBCONNECTION_H__

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
    /** Class that represents a connection to the database.
     * DBConnection is a wrapper around the "sqlite3" structure.
     */
    class DBConnection
    {
        public:

        DBConnection() noexcept;

        DBConnection(const DBConnection &other) noexcept;

        DBConnection(DBConnection &&other) noexcept;

        DBConnection& operator=(const DBConnection &other) noexcept;

        DBConnection& operator=(DBConnection &&other) noexcept;

        /** Open the provided database UTF-8 filename.
         * @param[in] filename UTF-8 path/uri to the database database file
         * @param[in] mode     file opening options specified by combination of OpenMode flags
         * @param[in] timeout  amount of milliseconds to wait before returning SQLite::BusyException when a table is locked
         */
        DBConnection(
            const std::string &filename,
            OpenMode mode = OpenMode::ReadWrite | OpenMode::Create,
            const std::chrono::milliseconds timeout = DEFAULT_TIMEOUT);

        /** Open the provided database UTF-8 filename.
         * @param[in] filename UTF-8 path/uri to the database database file
         * @param[in] timeout  amount of milliseconds to wait before returning SQLite::BusyException when a table is locked
         */
        DBConnection(const std::string &filename, const std::chrono::milliseconds timeout);

        /** Open the provided database UTF-16 filename.
         * @param[in] filename UTF-16 path/uri to the database database file
         * @param[in] timeout  Amount of milliseconds to wait before returning SQLite::BusyException when a table is locked
         */
        DBConnection(const std::u16string &filename, const std::chrono::milliseconds timeout = DEFAULT_TIMEOUT);

        /** Create a purely in memory database.
         * @returns a purely in memory SQLite::DBConnection
         */
        static DBConnection memory();

        /** Create a purely in memory database with UTF-16 as the native byte order.
         * @returns a purely in memory SQLite::DBConnection
         */
        static DBConnection wideMemory();

        /** Returns a Mutex that serializes access to the database.
         */
        Mutex getMutex();

        explicit operator bool() const noexcept;

        sqlite3* getHandle() const noexcept;

        /** Open an SQLite database file as specified by the filename argument.
         * @param filename path to SQLite file
         */
        void open(const std::string &filename, OpenMode mode = OpenMode::ReadWrite | OpenMode::Create);

        /** Open an SQLite database file as specified by the filname argument.
         * The database file will have UTF-16 native byte order.
         * @param filename path to SQLite file
         */
        void open(const std::u16string &filename);

        /** Returns the rowid of the most recent successful "INSERT" into
         * a rowid table or virtual table on database connection.
         * @returns rowid of the most recent successful "INSERT" into the database, or 0 if there was none.
         */
        long long rowId() const noexcept;

        /*
         * @param name The name of the function to be used in an SQL query
         * @param function
         * @param flags
         * @param nargs
         */
        template <typename F>
        void createGeneralFunction(
            const std::string &name,
            F &&function,
            const TextEncoding flags = SQLite::TextEncoding::UTF8,
            int nargs = -1)
        {
            using FunctionType = typename SQLiteFunctionTraits<F>::f_type;

            FunctionType *userFunction = new FunctionType(function);

            sqlite3_create_function_v2(
                getHandle(),
                name.c_str(),
                nargs,
                static_cast<int>(flags),
                (void*)userFunction,
                &internal_general_scalar_function<FunctionType>,
                nullptr,
                nullptr,
                &internal_delete<FunctionType>);
        }

        /*
         * @param name The name of the function to be used in an SQL query
         * @param function
         * @param flags
         */
        template <typename F>
        void createFunction(
            const std::string &name,
            F &&function,
            const TextEncoding flags = SQLite::TextEncoding::UTF8)
        {
            using FunctionType = typename function_traits<F>::f_type;

            FunctionType *userFunction = new FunctionType(function);

            sqlite3_create_function_v2(
                getHandle(),
                name.c_str(),
                function_traits<F>::nargs,
                static_cast<int>(flags),
                (void*)userFunction,
                &internal_scalar_function<FunctionType>,
                nullptr,
                nullptr,
                &internal_delete<FunctionType>);
        }

        /*
         * @param name The name of the function to be used in an SQL query
         * @param flags
         */
        template <typename A>
        void createAggregate(
            const std::string &name,
            const TextEncoding flags = SQLite::TextEncoding::UTF8)
        {
            using StepFunctionType = function_traits<decltype(&A::step)>;

            aggregate_wrapper<A> *wrapper = new aggregate_wrapper<A>;

            sqlite3_create_function_v2(
                getHandle(),
                name.c_str(),
                StepFunctionType::nargs,
                static_cast<int>(flags),
                (void*)wrapper,
                nullptr,
                &internal_step<aggregate_wrapper<A> >,
                &internal_final<aggregate_wrapper<A> >,
                &internal_dispose<aggregate_wrapper<A> >);
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
