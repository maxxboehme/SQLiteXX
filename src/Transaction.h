#ifndef __SQLITECXX_SQLITE_TRANSACTION_H__
#define __SQLITECXX_SQLITE_TRANSACTION_H__

#include <sqlite3.h>

#include "DBConnection.h"
#include "Statement.h"

namespace SQLite
{
    /** RAII encapsulation of the SQLite Transactions
    */
    class Transaction
    {
        public:

        enum Type {
            DEFERRED,
            IMMEDIATE,
            EXCLUSIVE
        };

        /** Begins the SQLite transaction.
         * @param connection [in] the database connection to begin the transaction on
         */
        Transaction(DBConnection &connection, const Type type = DEFERRED) :
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
        ~Transaction() noexcept
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

        const std::map<Type, std::string> m_transactionBeginnings = {
            {Type::DEFERRED, "BEGIN DEFERRED"},
            {Type::IMMEDIATE, "BEGIN IMMEDIATE"},
            {Type::EXCLUSIVE, "BEGIN EXCLUSIVE"}
        };

        const std::string COMMIT = "COMMIT";
        const std::string ROLLBACK = "ROLLBACK";


   };
}

#endif
