#include "Exception.h"

namespace sqlite
{
    void throw_error_code(sqlite3 *connection)
    {
        const int errcode = sqlite3_extended_errcode(connection);
        if (errcode == SQLITE_OK) return;
        if (errcode == SQLITE_DONE) return;

        switch(errcode)
        {
            case SQLITE_BUSY:
                throw busy_exception(connection);
                break;
            default:
                throw exception(connection);
                break;
        }
    }

    void throw_error_code(const int errcode, const std::string& message)
    {
        if (errcode == SQLITE_OK) return;
        if (errcode == SQLITE_DONE) return;

        switch(errcode)
        {
            case SQLITE_BUSY:
                throw busy_exception(message);
                break;
            default:
                throw exception(errcode, message);
                break;
        }
    }

}
