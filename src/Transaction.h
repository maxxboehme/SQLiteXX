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
    template <TransactionType T>
    class Transaction
    {
        public:
        const TransactionType type;

        /** Begins the SQLite transaction.
         * @param[in] connection the database connection to begin the transaction on
         */
        Transaction(DBConnection &connection) :
            type(T),
            m_connection(connection),
            m_commited(false)
        {
            std::string begin = m_transactionBeginnings.find(type)->second;
            Execute(m_connection, begin);
        }

        /** Commit the transaction
        */
        void commit()
        {
            Execute(m_connection, COMMIT);
            m_commited = true;
        }

        /** Safely rollback the transaction if it has not been commited.
        */
        virtual ~Transaction() noexcept
        {
            if (!m_commited)
            {
                try {
                    Execute(m_connection, ROLLBACK);
                } catch (SQLite::Exception&) {
                    // Don't throw exception in destructor. Already rolling back if
                    // issue occurred
                }
            }
        }

        private:

        DBConnection m_connection;
        bool m_commited;

        Transaction(const Transaction&) = delete;
        Transaction& operator=(const Transaction&) = delete;

        const std::map<TransactionType, std::string> m_transactionBeginnings = {
            {TransactionType::Deferred, "BEGIN DEFERRED"},
            {TransactionType::Immediate, "BEGIN IMMEDIATE"},
            {TransactionType::Exclusive, "BEGIN EXCLUSIVE"}
        };

        const std::string COMMIT = "COMMIT";
        const std::string ROLLBACK = "ROLLBACK";
    };

    using DeferredTransaction  = Transaction<TransactionType::Deferred>;
    using ImmediateTransaction = Transaction<TransactionType::Immediate>;
    using ExclusiveTransaction = Transaction<TransactionType::Exclusive>;
//    typedef Transaction<TransactionType::Deferred>  DeferredTransaction;
//    typedef Transaction<TransactionType::Immediate> ImmediateTransaction;
//    typedef Transaction<TransactionType::Exclusive> ExclusiveTransaction;
}

#endif
