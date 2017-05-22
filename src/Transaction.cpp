#include "Transaction.h"


namespace SQLite
{
    static const char* kCommit = "COMMIT";
    static const char* kRollback = "ROLLBACK";
    static const char* kBegginings[3] = {"BEGIN DEFERRED", "BEGIN IMMEDIATE", "BEGIN EXCLUSIVE"};

    Transaction::Transaction(DBConnection &connection, const TransactionType type) :
        type(type),
        m_connection(connection),
        m_commited(false)
    {
        const char* begin = kBegginings[static_cast<int>(type)];
        Execute(m_connection, begin);
    }

    Transaction::~Transaction() noexcept
    {
        if (!m_commited)
        {
            try {
                Execute(m_connection, kRollback);
            } catch (SQLite::Exception&) {
                // Don't throw exception in destructor. Already rolling back if
                // issue occurred
            }
        }
    }

    void Transaction::commit()
    {
        Execute(m_connection, kCommit);
        m_commited = true;
    }
}
