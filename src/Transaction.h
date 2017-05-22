#ifndef __SQLITECXX_SQLITE_TRANSACTION_H__
#define __SQLITECXX_SQLITE_TRANSACTION_H__

#include "DBConnection.h"
#include "Statement.h"

#include <sqlite3.h>


namespace SQLite
{
    enum class TransactionType : int {
        Deferred,
        Immediate,
        Exclusive
    };


    /** RAII encapsulation of the SQLite Transactions
    */
    class Transaction
    {
        public:
        const TransactionType type;

        /** Begins the SQLite transaction.
         * @param[in] connection the database connection to begin the transaction on
         */
        Transaction(DBConnection &connection, const TransactionType type);

        /** Safely rollback the transaction if it has not been commited.
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

    class DeferredTransaction : public Transaction
    {
        public:
        DeferredTransaction(DBConnection &connection) :
            Transaction(connection, TransactionType::Deferred)
        {}
    };

    class ImmediateTransaction : public Transaction
    {
        public:
        ImmediateTransaction(DBConnection &connection) :
            Transaction(connection, TransactionType::Immediate)
        {}
    };

    class ExclusiveTransaction : public Transaction
    {
        public:
        ExclusiveTransaction(DBConnection &connection) :
            Transaction(connection, TransactionType::Exclusive)
        {}
    };
}


#endif
