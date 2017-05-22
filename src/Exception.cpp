#include "Exception.h"

namespace SQLite
{
    void throwErrorCode(sqlite3 *connection)
    {
        const int errcode = sqlite3_extended_errcode(connection);
        if (errcode == SQLITE_OK) return;
        if (errcode == SQLITE_DONE) return;

        switch(errcode)
        {
            case SQLITE_BUSY:
                throw BusyException(connection);
                break;
            default:
                throw Exception(connection);
                break;
        }
    }

    void throwErrorCode(const int errcode, const std::string& message)
    {
        if (errcode == SQLITE_OK) return;
        if (errcode == SQLITE_DONE) return;

        switch(errcode)
        {
            case SQLITE_BUSY:
                throw BusyException(message);
                break;
            default:
                throw Exception(errcode, message);
                break;
        }
    }

}
