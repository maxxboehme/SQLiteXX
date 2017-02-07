#include "catch/catch.hpp"
#include "SQLiteXX.h"

#include <cstring>

TEST_CASE("Column conversion", "[Value]") {
    SQLite::DBConnection connection = SQLite::DBConnection::memory();

    REQUIRE(Execute(connection, "CREATE TABLE test (id INTEGER PRIMARY KEY, msg TEXT, int INTEGER, double REAL, binary BLOB, empty TEXT)") == 0);
    REQUIRE(connection.rowId() == 0);

    // Create a first row (autoid: 1) with all kind of data and a null value
    SQLite::Statement insert(connection, "INSERT INTO test VALUES (NULL, \"first\", -123, 0.123, ?, NULL)");
    // Bind the blob value to the first parameter of the SQL query
    const char  buffer[] = {'b', 'l', '\0', 'b'}; // "bl\0b" : 4 char, with a null byte inside
    const int   size = sizeof(buffer); // size = 4
    const void* blob = &buffer;
    insert.bind(1, blob, size);
    REQUIRE(insert.execute() == 1);

    SQLite::Statement query(connection, "SELECT * FROM test");
    REQUIRE(query.getColumnCount() == 6);
    query.step();

    {
        const int rowIDInteger = query.getValue(0);
        REQUIRE(rowIDInteger == 1);

        const sqlite3_int64 rowIDSQLiteInt64 = query.getValue(0);
        REQUIRE(rowIDSQLiteInt64 == 1);

        const int64_t rowIDInt64 = query.getValue(0);
        REQUIRE(rowIDInt64 == 1);

        const long long rowIDLongLong = query.getValue(0);
        REQUIRE(rowIDLongLong == 1);

        const unsigned int rowIDUnsignedInteger = query.getValue(0);
        REQUIRE(rowIDUnsignedInteger == 1);

        const char* txt = query.getValue(1);
        REQUIRE(std::string(txt) == std::string("first"));

        const char16_t* wtxt = query.getValue(1);
        REQUIRE(std::u16string(wtxt) == std::u16string(u"first"));

        const std::string string = query.getValue(1);
        REQUIRE(string == "first");

        const std::u16string wideString = query.getValue(1);
        REQUIRE(wideString == u"first");

        const int integer = query.getValue(2);
        REQUIRE(integer == -123);

        const double real = query.getValue(3);
        REQUIRE(real == 0.123);

        const void *sqlBlob = query.getValue(4);
        REQUIRE(memcmp("bl\0b", static_cast<const char*>(sqlBlob), size) == 0);

        const void *empty = query.getValue(5);
        REQUIRE(empty == nullptr);
    }

    {
        REQUIRE(query.getType(0) == SQLite::Type::Integer);
        REQUIRE(query.getType(1) == SQLite::Type::Text);
        REQUIRE(query.getType(2) == SQLite::Type::Integer);
        REQUIRE(query.getType(3) == SQLite::Type::Float);
        REQUIRE(query.getType(4) == SQLite::Type::Blob);
        REQUIRE(query.getType(5) == SQLite::Type::Null);
    }
}
