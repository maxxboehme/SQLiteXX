#include "catch.hpp"
#include "SQLiteXX.h"

#include <cstring>

TEST_CASE("Implicit conversion", "[Value]") {
    sqlite::dbconnection connection = sqlite::dbconnection::memory();

    REQUIRE(sqlite::execute(connection, "CREATE TABLE test (id INTEGER PRIMARY KEY, msg TEXT, int INTEGER, double REAL, binary BLOB, empty TEXT)") == 0);
    REQUIRE(connection.row_id() == 0);

    // Create a first row (autoid: 1) with all kind of data and a null value
    sqlite::statement insert(connection, "INSERT INTO test VALUES (NULL, \"first\", -123, 0.123, ?, NULL)");
    // Bind the blob value to the first parameter of the SQL query
    const char  buffer[] = {'b', 'l', '\0', 'b'}; // "bl\0b" : 4 char, with a null byte inside
    const int size = sizeof(buffer);
    const sqlite::blob blob(&buffer, size);
    insert.bind(1, blob);
    REQUIRE(insert.execute() == 1);

    sqlite::statement query(connection, "SELECT * FROM test");
    REQUIRE(query.column_count() == 6);
    query.step();

    {
        const int rowIDInteger = query.get_value(0);
        REQUIRE(rowIDInteger == 1);

        const sqlite3_int64 rowIDSQLiteInt64 = query.get_value(0);
        REQUIRE(rowIDSQLiteInt64 == 1);

        const int64_t rowIDInt64 = query.get_value(0);
        REQUIRE(rowIDInt64 == 1);

        const long long rowIDLongLong = query.get_value(0);
        REQUIRE(rowIDLongLong == 1);

        const unsigned int rowIDUnsignedInteger = query.get_value(0);
        REQUIRE(rowIDUnsignedInteger == 1);

        const std::string string = query.get_value(1);
        REQUIRE(string == "first");

        const std::u16string wideString = query.get_value(1);
        REQUIRE(wideString == u"first");

        const int integer = query.get_value(2);
        REQUIRE(integer == -123);

        const double real = query.get_value(3);
        REQUIRE(real == 0.123);

        const sqlite::blob sqlBlob = query.get_value(4);
        REQUIRE(sqlBlob.size() == 4);
        REQUIRE(memcmp("bl\0b", sqlBlob.data(), sqlBlob.size()) == 0);

        const sqlite::blob empty = query.get_value(5);
        REQUIRE(empty.data() == nullptr);
        REQUIRE(empty.size() == 0);
    }

    {
        REQUIRE(query.get_value(0).type() == sqlite::datatype::integer);
        REQUIRE(query.get_value(1).type() == sqlite::datatype::text);
        REQUIRE(query.get_value(2).type() == sqlite::datatype::integer);
        REQUIRE(query.get_value(3).type() == sqlite::datatype::floating);
        REQUIRE(query.get_value(4).type() == sqlite::datatype::blob);
        REQUIRE(query.get_value(5).type() == sqlite::datatype::null);
    }
}

TEST_CASE("Explicit conversion", "[Value]") {
    sqlite::dbconnection connection = sqlite::dbconnection::memory();

    REQUIRE(sqlite::execute(connection, "CREATE TABLE test (id INTEGER PRIMARY KEY, msg TEXT, int INTEGER, double REAL, binary BLOB, empty TEXT)") == 0);
    REQUIRE(connection.row_id() == 0);

    // Create a first row (autoid: 1) with all kind of data and a null value
    sqlite::statement insert(connection, "INSERT INTO test VALUES (NULL, \"first\", -123, 0.123, ?, NULL)");
    // Bind the blob value to the first parameter of the SQL query
    const char  buffer[] = {'b', 'l', '\0', 'b'}; // "bl\0b" : 4 char, with a null byte inside
    const int size = sizeof(buffer);
    const sqlite::blob blob(&buffer, size);
    insert.bind(1, blob);
    REQUIRE(insert.execute() == 1);

    sqlite::statement query(connection, "SELECT * FROM test");
    REQUIRE(query.column_count() == 6);
    query.step();

    {
        const sqlite::value firstColumnValue = query.get_value(0);
        REQUIRE(firstColumnValue.as_int() == 1);
        REQUIRE(firstColumnValue.as_int64() == 1);
        REQUIRE(firstColumnValue.as_uint() == 1u);
        REQUIRE(firstColumnValue.as_double() == 1.0);
        REQUIRE(firstColumnValue.as_string() == "1");
        REQUIRE(firstColumnValue.as_u16string() == u"1");

        const sqlite::value secondColumnValue = query.get_value(1);
        REQUIRE(secondColumnValue.as_int() == 0);
        REQUIRE(secondColumnValue.as_int64() == 0);
        REQUIRE(secondColumnValue.as_uint() == 0u);
        REQUIRE(secondColumnValue.as_double() == 0.0);
        REQUIRE(secondColumnValue.as_string() == "first");
        REQUIRE(secondColumnValue.as_u16string() == u"first");

        const sqlite::value thirdColumnValue = query.get_value(2);
        REQUIRE(thirdColumnValue.as_int() == -123);
        REQUIRE(thirdColumnValue.as_int64() == -123);
        // Will returned overflown representation
        REQUIRE(thirdColumnValue.as_uint() == (std::numeric_limits<unsigned int>::max() - 122));
        REQUIRE(thirdColumnValue.as_double() == -123.0);
        REQUIRE(thirdColumnValue.as_string() == "-123");
        REQUIRE(thirdColumnValue.as_u16string() == u"-123");

        const sqlite::value fourthColumnValue = query.get_value(3);
        REQUIRE(fourthColumnValue.as_int() == 0);
        REQUIRE(fourthColumnValue.as_int64() == 0);
        REQUIRE(fourthColumnValue.as_uint() == 0u);
        REQUIRE(fourthColumnValue.as_double() == 0.123);
        REQUIRE(fourthColumnValue.as_string() == "0.123");
        REQUIRE(fourthColumnValue.as_u16string() == u"0.123");
        REQUIRE(fourthColumnValue.as_u16string().size() == 5);

        const sqlite::value fifthColumnValue = query.get_value(4);
        REQUIRE(fifthColumnValue.as_int() == 0);
        REQUIRE(fifthColumnValue.as_int64() == 0);
        REQUIRE(fifthColumnValue.as_uint() == 0u);
        REQUIRE(fifthColumnValue.as_double() == 0.0);
        REQUIRE(fifthColumnValue.as_string() == std::string("bl\0b", 4));
        // What the blob object that as_blob returns will depend on
        // if you called getString or getU16String as sqlite will
        // convert the data in the back and when calling blob will return
        // that representation of it.
        sqlite::blob sqlBlob = fifthColumnValue.as_blob();
        REQUIRE(sqlBlob.size() == 4);
        REQUIRE(memcmp("bl\0b", sqlBlob.data(), sqlBlob.size()) == 0);

        REQUIRE(fifthColumnValue.as_u16string() == std::u16string(u"bl\0b", 4));
        sqlBlob = fifthColumnValue.as_blob();
        REQUIRE(sqlBlob.size() == 8);
        REQUIRE(memcmp(u"bl\0b", sqlBlob.data(), sqlBlob.size()) == 0);

        const sqlite::value sixthColumnValue = query.get_value(5);
        REQUIRE(sixthColumnValue.as_int() == 0);
        REQUIRE(sixthColumnValue.as_int64() == 0);
        REQUIRE(sixthColumnValue.as_uint() == 0u);
        REQUIRE(sixthColumnValue.as_double() == 0.0);
        REQUIRE(sixthColumnValue.as_string() == "");
        REQUIRE(sixthColumnValue.as_u16string() == u"");
        const sqlite::blob empty = sixthColumnValue.as_blob();
        REQUIRE(empty.data() == nullptr);
        REQUIRE(empty.size() == 0);
    }

    {
        REQUIRE(query.get_value(0).type() == sqlite::datatype::integer);
        REQUIRE(query.get_value(1).type() == sqlite::datatype::text);
        REQUIRE(query.get_value(2).type() == sqlite::datatype::integer);
        REQUIRE(query.get_value(3).type() == sqlite::datatype::floating);
        REQUIRE(query.get_value(4).type() == sqlite::datatype::blob);
        REQUIRE(query.get_value(5).type() == sqlite::datatype::null);
    }
}

TEST_CASE("Converting between string types", "[Value]") {
    sqlite::dbconnection connection = sqlite::dbconnection::memory();

    REQUIRE(sqlite::execute(connection, "CREATE TABLE test (msg TEXT)") == 0);
    REQUIRE(sqlite::execute(connection, "INSERT INTO test VALUES (\"first\")") == 1);

    sqlite::statement query(connection, "SELECT * FROM test");
    REQUIRE(query.column_count() == 1);
    query.step();

    const sqlite::value value = query.get_value(0);
    {
        std::string string = value.as_string();
        REQUIRE(string.size() == 5);
        REQUIRE(string == "first");

        std::u16string string16 = value.as_u16string();
        REQUIRE(string16.size() == 5);
        REQUIRE(string16 == u"first");

        string = value.as_string();
        REQUIRE(string.size() == 5);
        REQUIRE(string == "first");

        string16 = value.as_u16string();
        REQUIRE(string16.size() == 5);
        REQUIRE(string16 == u"first");
    }
}

TEST_CASE("Value Test Assignment Operators", "[Value]") {
    sqlite::dbconnection connection = sqlite::dbconnection::memory();

    REQUIRE(sqlite::execute(connection, "CREATE TABLE test (txt1 TEXT, txt2 TEXT)") == 0);
    REQUIRE(sqlite::execute(connection, "INSERT INTO test VALUES (\"first\", \"second\")") == 1);

    sqlite::statement query(connection, "SELECT * FROM test");
    REQUIRE(query.column_count() == 2);
    query.step();

    SECTION("L-value assignment operator") {
        sqlite::value value = query.get_value(0);
        REQUIRE(value.as_string() == std::string("first"));

        sqlite::value value2 = query.get_value(1);
        REQUIRE(value2.as_string() == std::string("second"));

        // Testing assignment
        value = value2;
        REQUIRE(value.as_string() == std::string("second"));
    }

    SECTION("R-value assignment operator") {
        sqlite::value value = query.get_value(0);
        REQUIRE(value.as_string() == std::string("first"));

        // Testing assignment
        value = query.get_value(1);
        REQUIRE(value.as_string() == std::string("second"));
    }
}


