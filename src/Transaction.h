/** @file */

#ifndef __SQLITEXX_SQLITE_TRANSACTION_H__
#define __SQLITEXX_SQLITE_TRANSACTION_H__

#include "DBConnection.h"
#include "Statement.h"

#include <sqlite3.h>


namespace sqlite
{

    /** Used to specify the different types of SQLite transactions.
     */
    enum class transactiontype: int {
        deferred,  ///< means that no locks are acquired on the database until the database is first accessed.
        immediate, ///< means that no database connection will be able to write to the database or do a BEGIN IMMEDIATE/EXCLUSIVE.
        exclusive  ///< means that no other database connection except for read_uncommitted connection will be able to read/write to the database.
    };


    /** RAII encapsulation of the SQLite Transactions
    */
    class transaction
    {
        public:
        const transactiontype type;

        /** Implements a strictly scope-based SQLite transaction.
         * @param[in] connection the database connection to begin the transaction on
         * @param[in] type the transaction type to be used.
         */
        transaction(dbconnection& connection, const transactiontype type);

        /** Destructor.
         * Safely rollback the transaction if it has not been commited.
        */
        virtual ~transaction() noexcept;

        /** Commit the transaction
        */
        virtual void commit();

        private:

        dbconnection m_connection;
        bool m_commited;

        transaction(const transaction&) = delete;
        transaction& operator=(const transaction&) = delete;
    };

    /** RAII encapsulation of the SQLite deferred transaction.
    */
    class deferred_transaction : public transaction
    {
        public:
        /** Implements a strictly scope-based SQLite deferred transaction.
         * @param[in] connection the database connection to begin the transaction on
         */
        deferred_transaction(dbconnection& connection) :
            transaction(connection, transactiontype::deferred)
        {}
    };

    /** RAII encapsulation of the SQLite immediate transaction.
    */
    class immediate_transaction : public transaction
    {
        public:
        /** Implements a strictly scope-based SQLite immediate transaction.
         * @param[in] connection the database connection to begin the transaction on
         */
        immediate_transaction(dbconnection& connection) :
            transaction(connection, transactiontype::immediate)
        {}
    };

    /** RAII encapsulation of the SQLite exclusive transaction.
    */
    class exclusive_transaction : public transaction
    {
        public:
        /** Implements a strictly scope-based SQLite exclusive transaction.
         * @param[in] connection the database connection to begin the transaction on
         */
        exclusive_transaction(dbconnection& connection) :
            transaction(connection, transactiontype::exclusive)
        {}
    };
}


#endif
