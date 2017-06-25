#include "catch.hpp"
#include "SQLiteXX.h"

#include <cstring>

TEST_CASE("Implicit conversion", "[Value]") {
    SQLite::DBConnection connection = SQLite::DBConnection::memory();

    REQUIRE(Execute(connection, "CREATE TABLE test (id INTEGER PRIMARY KEY, msg TEXT, int INTEGER, double REAL, binary BLOB, empty TEXT)") == 0);
    REQUIRE(connection.rowId() == 0);

    // Create a first row (autoid: 1) with all kind of data and a null value
    SQLite::Statement insert(connection, "INSERT INTO test VALUES (NULL, \"first\", -123, 0.123, ?, NULL)");
    // Bind the blob value to the first parameter of the SQL query
    const char  buffer[] = {'b', 'l', '\0', 'b'}; // "bl\0b" : 4 char, with a null byte inside
    const int size = sizeof(buffer);
    const SQLite::Blob blob(&buffer, size);
    insert.bind(1, blob);
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

        const std::string string = query.getValue(1);
        REQUIRE(string == "first");

        const std::u16string wideString = query.getValue(1);
        REQUIRE(wideString == u"first");

        const int integer = query.getValue(2);
        REQUIRE(integer == -123);

        const double real = query.getValue(3);
        REQUIRE(real == 0.123);

        const SQLite::Blob sqlBlob = query.getValue(4);
        REQUIRE(sqlBlob.size() == 4);
        REQUIRE(memcmp("bl\0b", sqlBlob.data(), sqlBlob.size()) == 0);

        const SQLite::Blob empty = query.getValue(5);
        // Note: For some reason in release mode this does not work
        // with Catch.
        // REQUIRE(empty.data() == nullptr);
        REQUIRE(empty.size() == 0);
    }

    {
        REQUIRE(query.getValue(0).getType() == SQLite::Type::Integer);
        REQUIRE(query.getValue(1).getType() == SQLite::Type::Text);
        REQUIRE(query.getValue(2).getType() == SQLite::Type::Integer);
        REQUIRE(query.getValue(3).getType() == SQLite::Type::Float);
        REQUIRE(query.getValue(4).getType() == SQLite::Type::Blob);
        REQUIRE(query.getValue(5).getType() == SQLite::Type::Null);
    }
}

TEST_CASE("Explicit conversion", "[Value]") {
    SQLite::DBConnection connection = SQLite::DBConnection::memory();

    REQUIRE(Execute(connection, "CREATE TABLE test (id INTEGER PRIMARY KEY, msg TEXT, int INTEGER, double REAL, binary BLOB, empty TEXT)") == 0);
    REQUIRE(connection.rowId() == 0);

    // Create a first row (autoid: 1) with all kind of data and a null value
    SQLite::Statement insert(connection, "INSERT INTO test VALUES (NULL, \"first\", -123, 0.123, ?, NULL)");
    // Bind the blob value to the first parameter of the SQL query
    const char  buffer[] = {'b', 'l', '\0', 'b'}; // "bl\0b" : 4 char, with a null byte inside
    const int size = sizeof(buffer);
    const SQLite::Blob blob(&buffer, size);
    insert.bind(1, blob);
    REQUIRE(insert.execute() == 1);

    SQLite::Statement query(connection, "SELECT * FROM test");
    REQUIRE(query.getColumnCount() == 6);
    query.step();

    {
        const SQLite::Value firstColumnValue = query.getValue(0);
        REQUIRE(firstColumnValue.getInt() == 1);
        REQUIRE(firstColumnValue.getInt64() == 1);
        REQUIRE(firstColumnValue.getUInt() == 1u);
        REQUIRE(firstColumnValue.getDouble() == 1.0);
        REQUIRE(firstColumnValue.getString() == "1");
        REQUIRE(firstColumnValue.getU16String() == u"1");

        const SQLite::Value secondColumnValue = query.getValue(1);
        REQUIRE(secondColumnValue.getInt() == 0);
        REQUIRE(secondColumnValue.getInt64() == 0);
        REQUIRE(secondColumnValue.getUInt() == 0u);
        REQUIRE(secondColumnValue.getDouble() == 0.0);
        REQUIRE(secondColumnValue.getString() == "first");
        REQUIRE(secondColumnValue.getU16String() == u"first");

        const SQLite::Value thirdColumnValue = query.getValue(2);
        REQUIRE(thirdColumnValue.getInt() == -123);
        REQUIRE(thirdColumnValue.getInt64() == -123);
        // Will returned overflown representation
        REQUIRE(thirdColumnValue.getUInt() == (std::numeric_limits<unsigned int>::max() - 122));
        REQUIRE(thirdColumnValue.getDouble() == -123.0);
        REQUIRE(thirdColumnValue.getString() == "-123");
        REQUIRE(thirdColumnValue.getU16String() == u"-123");

        const SQLite::Value fourthColumnValue = query.getValue(3);
        REQUIRE(fourthColumnValue.getInt() == 0);
        REQUIRE(fourthColumnValue.getInt64() == 0);
        REQUIRE(fourthColumnValue.getUInt() == 0u);
        REQUIRE(fourthColumnValue.getDouble() == 0.123);
        REQUIRE(fourthColumnValue.getString() == "0.123");
        REQUIRE(fourthColumnValue.getU16String() == u"0.123");
        REQUIRE(fourthColumnValue.getU16String().size() == 5);

        const SQLite::Value fifthColumnValue = query.getValue(4);
        REQUIRE(fifthColumnValue.getInt() == 0);
        REQUIRE(fifthColumnValue.getInt64() == 0);
        REQUIRE(fifthColumnValue.getUInt() == 0u);
        REQUIRE(fifthColumnValue.getDouble() == 0.0);
        REQUIRE(fifthColumnValue.getString() == std::string("bl\0b", 4));
        // What the Blob object that getBlob returns will depend on
        // if you called getString or getU16String as sqlite will
        // convert the data in the back and when calling blob will return
        // that representation of it.
        SQLite::Blob sqlBlob = fifthColumnValue.getBlob();
        REQUIRE(sqlBlob.size() == 4);
        REQUIRE(memcmp("bl\0b", sqlBlob.data(), sqlBlob.size()) == 0);

        REQUIRE(fifthColumnValue.getU16String() == std::u16string(u"bl\0b", 4));
        sqlBlob = fifthColumnValue.getBlob();
        REQUIRE(sqlBlob.size() == 8);
        REQUIRE(memcmp(u"bl\0b", sqlBlob.data(), sqlBlob.size()) == 0);

        const SQLite::Value sixthColumnValue = query.getValue(5);
        REQUIRE(sixthColumnValue.getInt() == 0);
        REQUIRE(sixthColumnValue.getInt64() == 0);
        REQUIRE(sixthColumnValue.getUInt() == 0u);
        REQUIRE(sixthColumnValue.getDouble() == 0.0);
        REQUIRE(sixthColumnValue.getString() == "");
        REQUIRE(sixthColumnValue.getU16String() == u"");
        const SQLite::Blob empty = sixthColumnValue.getBlob();
        // Note: For some reason in release mode this does not work
        // with catch.
        // REQUIRE(empty.data() == nullptr);
        REQUIRE(empty.size() == 0);
    }

    {
        REQUIRE(query.getValue(0).getType() == SQLite::Type::Integer);
        REQUIRE(query.getValue(1).getType() == SQLite::Type::Text);
        REQUIRE(query.getValue(2).getType() == SQLite::Type::Integer);
        REQUIRE(query.getValue(3).getType() == SQLite::Type::Float);
        REQUIRE(query.getValue(4).getType() == SQLite::Type::Blob);
        REQUIRE(query.getValue(5).getType() == SQLite::Type::Null);
    }
}

TEST_CASE("Converting between string types", "[Value]") {
    SQLite::DBConnection connection = SQLite::DBConnection::memory();

    REQUIRE(Execute(connection, "CREATE TABLE test (msg TEXT)") == 0);
    REQUIRE(Execute(connection, "INSERT INTO test VALUES (\"first\")") == 1);

    SQLite::Statement query(connection, "SELECT * FROM test");
    REQUIRE(query.getColumnCount() == 1);
    query.step();

    const SQLite::Value value = query.getValue(0);
    {
        std::string string = value.getString();
        REQUIRE(string.size() == 5);
        REQUIRE(string == "first");

        std::u16string string16 = value.getU16String();
        REQUIRE(string16.size() == 5);
        REQUIRE(string16 == u"first");

        string = value.getString();
        REQUIRE(string.size() == 5);
        REQUIRE(string == "first");

        string16 = value.getU16String();
        REQUIRE(string16.size() == 5);
        REQUIRE(string16 == u"first");
    }
}

TEST_CASE("Value Test Assignment Operators", "[Value]") {
    SQLite::DBConnection connection = SQLite::DBConnection::memory();

    REQUIRE(Execute(connection, "CREATE TABLE test (txt1 TEXT, txt2 TEXT)") == 0);
    REQUIRE(Execute(connection, "INSERT INTO test VALUES (\"first\", \"second\")") == 1);

    SQLite::Statement query(connection, "SELECT * FROM test");
    REQUIRE(query.getColumnCount() == 2);
    query.step();

    SECTION("L-value assignment operator") {
        SQLite::Value value = query.getValue(0);
        REQUIRE(value.getString() == std::string("first"));

        SQLite::Value value2 = query.getValue(1);
        REQUIRE(value2.getString() == std::string("second"));

        // Testing assignment
        value = value2;
        REQUIRE(value.getString() == std::string("second"));
    }

    SECTION("R-value assignment operator") {
        SQLite::Value value = query.getValue(0);
        REQUIRE(value.getString() == std::string("first"));

        // Testing assignment
        value = query.getValue(1);
        REQUIRE(value.getString() == std::string("second"));
    }
}


