/** @file */

#ifndef __SQLITEXX_SQLITE_TRANSACTION_H__
#define __SQLITEXX_SQLITE_TRANSACTION_H__

#include "DBConnection.h"
#include "Statement.h"

#include <sqlite3.h>


namespace SQLite
{

    /** Used to specify the different types of SQLite transactions.
     */
    enum class TransactionType: int {
        Deferred,  ///< means that no locks are acquired on the database until the database is first accessed.
        Immediate, ///< means that no database connection will be able to write to the database or do a BEGIN IMMEDIATE/EXCLUSIVE.
        Exclusive  ///< means that no other database connection except for read_uncommitted connection will be able to read/write to the database.
    };


    /** RAII encapsulation of the SQLite Transactions
    */
    class Transaction
    {
        public:
        const TransactionType type;

        /** Implements a strictly scope-based SQLite transaction.
         * @param[in] connection the database connection to begin the transaction on
         * @param[in] type the transaction type to be used.
         */
        Transaction(DBConnection& connection, const TransactionType type);

        /** Destructor.
         * Safely rollback the transaction if it has not been commited.
        */
        virtual ~Transaction() noexcept;

        /** Commit the transaction
        */
        virtual void commit();

        private:

        DBConnection m_connection;
        bool m_commited;

        Transaction(const Transaction&) = delete;
        Transaction& operator=(const Transaction&) = delete;
    };

    /** RAII encapsulation of the SQLite deferred transaction.
    */
    class DeferredTransaction : public Transaction
    {
        public:
        /** Implements a strictly scope-based SQLite deferred transaction.
         * @param[in] connection the database connection to begin the transaction on
         */
        DeferredTransaction(DBConnection& connection) :
            Transaction(connection, TransactionType::Deferred)
        {}
    };

    /** RAII encapsulation of the SQLite immediate transaction.
    */
    class ImmediateTransaction : public Transaction
    {
        public:
        /** Implements a strictly scope-based SQLite immediate transaction.
         * @param[in] connection the database connection to begin the transaction on
         */
        ImmediateTransaction(DBConnection& connection) :
            Transaction(connection, TransactionType::Immediate)
        {}
    };

    /** RAII encapsulation of the SQLite exclusive transaction.
    */
    class ExclusiveTransaction : public Transaction
    {
        public:
        /** Implements a strictly scope-based SQLite exclusive transaction.
         * @param[in] connection the database connection to begin the transaction on
         */
        ExclusiveTransaction(DBConnection& connection) :
            Transaction(connection, TransactionType::Exclusive)
        {}
    };
}


#endif
