#include <SQLiteXX/SQLiteXX.h>

#include <iostream>

int main(int argc, const char *argv[])
{
    SQLite::DBConnection connection = SQLite::DBConnection::memory();
    SQLite::Execute(connection, "CREATE TABLE test (id INTEGER PRIMARY KEY, msg TEXT, int INTEGER, double REAL, binary BLOB, empty TEXT)");

    SQLite::Statement insert(connection, "INSERT INTO test VALUES (NULL, \"first\", -123, 0.123, ?, NULL)");
    insert.execute();

    SQLite::Statement query(connection, "SELECT * FROM test");
    const int rowIDInteger = query.getValue(0);

    return 0;
}
