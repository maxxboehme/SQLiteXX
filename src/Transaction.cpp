#include "Transaction.h"


namespace sqlite
{
    static const char* kCommit = "COMMIT";
    static const char* kRollback = "ROLLBACK";
    static const char* kBegginings[3] = {"BEGIN DEFERRED", "BEGIN IMMEDIATE", "BEGIN EXCLUSIVE"};

    transaction::transaction(dbconnection &connection, const transactiontype type) :
        type(type),
        m_connection(connection),
        m_commited(false)
    {
        const char* begin = kBegginings[static_cast<int>(type)];
        sqlite::execute(m_connection, begin);
    }

    transaction::~transaction() noexcept
    {
        if (!m_commited)
        {
            try {
                sqlite::execute(m_connection, kRollback);
            } catch (sqlite::exception&) {
                // Don't throw exception in destructor. Already rolling back if
                // issue occurred.
            }
        }
    }

    void transaction::commit()
    {
        sqlite::execute(m_connection, kCommit);
        m_commited = true;
    }
}
