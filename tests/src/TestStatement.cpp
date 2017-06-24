#include "catch.hpp"
#include "SQLiteXX.h"

#include <string>
#include <utility>
#include <vector>

TEST_CASE("Query in Memory Database", "[Statement]") {
    SQLite::DBConnection connection = SQLite::DBConnection::memory();

    SECTION("querying table that does not exist") {
        REQUIRE_THROWS_AS(Execute(connection, "SELECT * FROM test"), SQLite::Exception);
    }

    SQLite::Statement query(connection, "CREATE TABLE test (id INTEGER PRIMARY KEY, value TEXT)");
    REQUIRE_NOTHROW(query.execute());

    SECTION("Create a Statement with no parameter") {
        SQLite::Statement query(connection, "SELECT * FROM test");
        REQUIRE(query.getColumnCount() == 2);
    }

    SECTION("Try to bind to SELECT statement") {
        SQLite::Statement query(connection, "SELECT * FROM test");
        REQUIRE_THROWS_AS(query.bind(-1, 12345), SQLite::Exception);
        REQUIRE_THROWS_AS(query.bind(0, 12345), SQLite::Exception);
        REQUIRE_THROWS_AS(query.bind(1, 12345), SQLite::Exception);
        REQUIRE_THROWS_AS(query.bind(2, 12345), SQLite::Exception);
        REQUIRE_THROWS_AS(query.bind(2, "abc"), SQLite::Exception);
    }

    SECTION("Insert a row") {
        REQUIRE(Execute(connection, "INSERT INTO test VALUES (NULL, \"one\")") == 1);
        REQUIRE(connection.rowId() == 1);
    }
}

TEST_CASE("Executing Steps", "[Statement]") {
    SQLite::DBConnection connection = SQLite::DBConnection::memory();

    REQUIRE(Execute(connection, "CREATE TABLE test (id INTEGER PRIMARY KEY, string TEXT, int INTEGER, double REAL)") == 0);

    REQUIRE(Execute(connection, "INSERT INTO test VALUES (NULL, \"one\", 1234, 0.1234)") == 1);
    REQUIRE(connection.rowId() == 1);

    SQLite::Statement query(connection, "SELECT * FROM test");
    REQUIRE(query.getColumnCount() == 4);

    REQUIRE_NOTHROW(query.step());
    REQUIRE(query.getInt64(0) == 1);
    REQUIRE(query.getString(1) == std::string("one"));
    REQUIRE(query.getInt(2) == 1234);
    REQUIRE(query.getDouble(3) == 0.1234);

    // Step one more time to discover there is nothing more
    REQUIRE(query.step() == false);

    // Step after "the end" throw an exception
    REQUIRE(query.step() == false);

    // Try to insert a new row with the same PRIMARY KEY: "UNIQUE constraint failed: test.id"
    SQLite::Statement insert(connection, "INSERT INTO test VALUES (1, \"exception\", 456, 0.456)");
    REQUIRE_THROWS_AS(insert.step(), SQLite::Exception);
    // reset should throw again
    REQUIRE_THROWS_AS(insert.reset(), SQLite::Exception);

    REQUIRE_THROWS_AS(Execute(connection, "INSERT INTO test VALUES (1, \"exception\", 456, 0.456)"), SQLite::Exception);
}

TEST_CASE("Preparing a statement", "[Statement]") {
    SQLite::DBConnection connection = SQLite::DBConnection::memory();

    REQUIRE(Execute(connection, "CREATE TABLE test (id INTEGER PRIMARY KEY, msg TEXT, int INTEGER, double REAL)") == 0);
    REQUIRE(connection.rowId() == 0);

    SECTION("Explicitly calling prepare") {
        SQLite::Statement insert;
        insert.prepare(connection, "INSERT INTO test VALUES (NULL, \"first\", -123, 0.123)");
        REQUIRE(insert.execute() == 1);
    }

    SECTION("Preparing through constructor") {
        SQLite::Statement insert(connection, "INSERT INTO test VALUES (NULL, \"first\", -123, 0.123)");
        REQUIRE(insert.execute() == 1);
    }

    SECTION("Preparing through execute") {
        REQUIRE_NOTHROW(
            Execute(connection, "INSERT INTO test VALUES (NULL, \"first\", -123, 0.123)"));
    }

    SQLite::Statement query(connection, "SELECT * FROM test");
    REQUIRE(query.getColumnCount() == 4);
    query.step();

    REQUIRE(query.getString(1) == "first");
    REQUIRE(query.getInt(2) == -123);
    REQUIRE(query.getDouble(3) == 0.123);

    {
        REQUIRE(query.getType(0) == SQLite::Type::Integer);
        REQUIRE(query.getType(1) == SQLite::Type::Text);
        REQUIRE(query.getType(2) == SQLite::Type::Integer);
        REQUIRE(query.getType(3) == SQLite::Type::Float);
    }

}

TEST_CASE("Retrieving values from statements", "[Statement]") {
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
        REQUIRE(query.getType(0) == SQLite::Type::Integer);
        REQUIRE(query.getType(1) == SQLite::Type::Text);
        REQUIRE(query.getType(2) == SQLite::Type::Integer);
        REQUIRE(query.getType(3) == SQLite::Type::Float);
        REQUIRE(query.getType(4) == SQLite::Type::Blob);
        REQUIRE(query.getType(5) == SQLite::Type::Null);
    }

    {
        REQUIRE(query.getInt(0) == 1);
        REQUIRE(query.getInt64(0) == 1);
        REQUIRE(query.getUInt(0) == 1u);
        REQUIRE(query.getDouble(0) == 1.0);
        REQUIRE(query.getString(0) == "1");
        REQUIRE(query.getU16String(0) == u"1");

        REQUIRE(query.getInt(1) == 0);
        REQUIRE(query.getInt64(1) == 0);
        REQUIRE(query.getUInt(1) == 0u);
        REQUIRE(query.getDouble(1) == 0.0);
        REQUIRE(query.getString(1) == "first");
        REQUIRE(query.getU16String(1) == u"first");

        REQUIRE(query.getInt(2) == -123);
        REQUIRE(query.getInt64(2) == -123);
        // Will returned overflown representation
        REQUIRE(query.getUInt(2) == (std::numeric_limits<unsigned int>::max() - 122));
        REQUIRE(query.getDouble(2) == -123.0);
        REQUIRE(query.getString(2) == "-123");
        REQUIRE(query.getU16String(2) == u"-123");

        REQUIRE(query.getInt(3) == 0);
        REQUIRE(query.getInt64(3) == 0);
        REQUIRE(query.getUInt(3) == 0u);
        REQUIRE(query.getDouble(3) == 0.123);
        REQUIRE(query.getString(3) == "0.123");
        REQUIRE(query.getU16String(3) == u"0.123");
        REQUIRE(query.getU16String(3).size() == 5);

        const SQLite::Value fifthColumnValue = query.getValue(4);
        REQUIRE(query.getInt(4) == 0);
        REQUIRE(query.getInt64(4) == 0);
        REQUIRE(query.getUInt(4) == 0u);
        REQUIRE(query.getDouble(4) == 0.0);
        REQUIRE(query.getString(4) == std::string("bl\0b", 4));
        // What the Blob object that getBlob returns will depend on
        // if you called getString or getU16String as sqlite will
        // convert the data in the back and when calling blob will return
        // that representation of it.
        SQLite::Blob sqlBlob = query.getBlob(4);
        REQUIRE(sqlBlob.size() == 4);
        REQUIRE(memcmp("bl\0b", sqlBlob.data(), sqlBlob.size()) == 0);

        REQUIRE(query.getU16String(4) == std::u16string(u"bl\0b", 4));
        sqlBlob = query.getBlob(4);
        REQUIRE(sqlBlob.size() == 8);
        REQUIRE(memcmp(u"bl\0b", sqlBlob.data(), sqlBlob.size()) == 0);

        REQUIRE(query.getInt(5) == 0);
        REQUIRE(query.getInt64(5) == 0);
        REQUIRE(query.getUInt(5) == 0u);
        REQUIRE(query.getDouble(5) == 0.0);
        REQUIRE(query.getString(5) == "");
        REQUIRE(query.getU16String(5) == u"");
        const SQLite::Blob empty = query.getBlob(5);
        // Note: For some reason in release mode this does not work
        // with catch.
        // REQUIRE(empty.data() == nullptr);
        REQUIRE(empty.size() == 0);
    }

    {
        REQUIRE(query.getType(0) == SQLite::Type::Integer);
        REQUIRE(query.getType(1) == SQLite::Type::Text);
        REQUIRE(query.getType(2) == SQLite::Type::Integer);
        REQUIRE(query.getType(3) == SQLite::Type::Float);
        // Note that the type returned by query for column four has
        // changed from a blob to text because of the conversions.
        // This is different then what a SQLite::Value does.
        REQUIRE(query.getType(4) == SQLite::Type::Text);
        REQUIRE(query.getType(5) == SQLite::Type::Null);
    }
}

TEST_CASE("Binding to a statement", "[Statement]") {
    SQLite::DBConnection connection = SQLite::DBConnection::memory();

    REQUIRE(Execute(connection, "CREATE TABLE test (id INTEGER PRIMARY KEY, msg TEXT, int INTEGER, double REAL, binary BLOB, empty TEXT)") == 0);
    REQUIRE(connection.rowId() == 0);

    SECTION("Explicitly calling bind") {
        SQLite::Statement insert(connection, "INSERT INTO test VALUES (NULL, ?, ?, ?, ?, NULL)");

        REQUIRE_NOTHROW(insert.bind(1, "first"));
        REQUIRE_NOTHROW(insert.bind(2, -123));
        REQUIRE_NOTHROW(insert.bind(3, 0.123));

        const char buffer[] = {'b', 'l', '\0', 'b'}; // "bl\0b" : 4 char, with a null byte inside
        const int size = sizeof(buffer);
        REQUIRE_NOTHROW(insert.bind(4, SQLite::Blob(&buffer, size)));

        REQUIRE(insert.execute() == 1);
    }

    SECTION("Variadic bind") {
        SQLite::Statement insert(connection, "INSERT INTO test VALUES (NULL, ?, ?, ?, ?, NULL)");

        const char buffer[] = {'b', 'l', '\0', 'b'}; // "bl\0b" : 4 char, with a null byte inside
        const int size = sizeof(buffer);
        REQUIRE_NOTHROW(insert.bindAll("first", -123, 0.123, SQLite::Blob(&buffer, size)));

        REQUIRE(insert.execute() == 1);
    }


    SECTION("Binding through constructor") {
        const char buffer[] = {'b', 'l', '\0', 'b'}; // "bl\0b" : 4 char, with a null byte inside
        const int size = sizeof(buffer);

        SQLite::Statement insert(
            connection,
            "INSERT INTO test VALUES (NULL, ?, ?, ?, ?, NULL)",
            "first",
            -123,
            0.123,
            SQLite::Blob(&buffer, size));

        REQUIRE(insert.execute() == 1);
    }

    SECTION("Binding through execute") {
        const char buffer[] = {'b', 'l', '\0', 'b'}; // "bl\0b" : 4 char, with a null byte inside
        const int size = sizeof(buffer);

        REQUIRE_NOTHROW(
            Execute(
                connection,
                "INSERT INTO test VALUES (NULL, ?, ?, ?, ?, NULL)",
                "first",
                -123,
                0.123,
                SQLite::Blob(&buffer, size)));
    }

    SQLite::Statement query(connection, "SELECT * FROM test");
    REQUIRE(query.getColumnCount() == 6);
    query.step();

    REQUIRE(query.getString(1) == "first");
    REQUIRE(query.getInt(2) == -123);
    REQUIRE(query.getDouble(3) == 0.123);
    const SQLite::Blob sqlBlob = query.getBlob(4);
    REQUIRE(sqlBlob.size() == 4);
    REQUIRE(memcmp("bl\0b", sqlBlob.data(), sqlBlob.size()) == 0);

    {
        REQUIRE(query.getType(0) == SQLite::Type::Integer);
        REQUIRE(query.getType(1) == SQLite::Type::Text);
        REQUIRE(query.getType(2) == SQLite::Type::Integer);
        REQUIRE(query.getType(3) == SQLite::Type::Float);
        REQUIRE(query.getType(4) == SQLite::Type::Blob);
        REQUIRE(query.getType(5) == SQLite::Type::Null);
    }
}

TEST_CASE("Binding using Names", "[Statement]") {
    SQLite::DBConnection connection = SQLite::DBConnection::memory();
    REQUIRE(Execute(connection, "CREATE TABLE test (id INTEGER PRIMARY KEY, string TEXT, int INTEGER, double REAL)") == 0);

    SQLite::Statement insert(connection, "INSERT INTO test VALUES (NULL, @string, @int, @double)");
    // First row with text/int/double
    insert.bindByName("@string", "one");
    insert.bindByName("@int", 1234);
    insert.bindByName("@double", 0.1234);
    REQUIRE(insert.execute() == 1);

    // Compile a SQL query to check the result
    SQLite::Statement query(connection, "SELECT * FROM test");
    REQUIRE(query.getColumnCount() == 4);

    // Check the result
    REQUIRE_NOTHROW(query.step());
    REQUIRE(query.getInt64(0) == 1);
    REQUIRE(query.getString(1) == std::string("one"));
    REQUIRE(query.getInt(2) == 1234);
    REQUIRE(query.getDouble(3) == 0.1234);

    insert.reset();
    const std::string str("two");
    const int integer = 1234;
    const double dub = 0.1234;
    insert.bindByName("@string", str);
    insert.bindByName("@int", integer);
    insert.bindByName("@double", dub);
    REQUIRE(insert.execute() == 1);

    // Check the result
    REQUIRE_NOTHROW(query.step());
    REQUIRE(query.getInt64(0) == 2);
    REQUIRE(query.getString(1) == std::string("two"));
    REQUIRE(query.getInt(2) == 1234);
    REQUIRE(query.getDouble(3) == 0.1234);
}

TEST_CASE("Getting column names", "[Statement]") {
    SQLite::DBConnection connection = SQLite::DBConnection::memory();

    REQUIRE(Execute(connection, "CREATE TABLE test (id INTEGER PRIMARY KEY, string TEXT, int INTEGER, double REAL)") == 0);

    SECTION("get column name from SELECT") {
        SQLite::Statement query(connection, "SELECT id, string, int, double FROM test");
        query.step();

        REQUIRE(query.getColumnName(0) == std::string("id"));
        REQUIRE(query.getColumnName(1) == std::string("string"));
        REQUIRE(query.getColumnName(2) == std::string("int"));
        REQUIRE(query.getColumnName(3) == std::string("double"));
    }

    SECTION("get column name after rename \"as\"") {
        SQLite::Statement query(connection, "SELECT id, string as value FROM test");
        query.step();

        REQUIRE(query.getColumnName(0) == std::string("id"));
        REQUIRE(query.getColumnName(1) == std::string("value"));
    }
}

static void test_callback(const std::vector<std::string> &columnData, const std::vector<std::string> &columnNames, std::vector<std::vector<std::pair<std::string, std::string> > > &allColumnData) {
    std::vector<std::pair<std::string, std::string> > columnDataPairs;
    for (size_t i = 0; i < columnData.size(); ++i) {
        columnDataPairs.push_back(std::make_pair(columnNames[i], columnData[i]));
    }

    allColumnData.push_back(columnDataPairs);
}

TEST_CASE("Using callback function", "[Statement]") {
    SQLite::DBConnection connection = SQLite::DBConnection::memory();

    REQUIRE(Execute(connection, "CREATE TABLE test (id INTEGER PRIMARY KEY, string TEXT, double REAL)") == 0);

    REQUIRE(Execute(connection, "INSERT INTO test VALUES (1, \"one\", 1.0)") == 1);
    REQUIRE(Execute(connection, "INSERT INTO test VALUES (2, \"two\", 2.0)") == 1);
    REQUIRE(Execute(connection, "INSERT INTO test VALUES (3, \"three\", 3.0)") == 1);

    std::vector<std::vector<std::pair<std::string, std::string> > > allColumnData;

    ExecuteCallback(connection, "SELECT * FROM test", test_callback, std::ref(allColumnData));
    REQUIRE(allColumnData.size() == 3);
    REQUIRE(allColumnData[0].size() == 3);
    REQUIRE(allColumnData[1].size() == 3);
    REQUIRE(allColumnData[2].size() == 3);
}

TEST_CASE("Using callback function with lambda", "[Statement]") {
    SQLite::DBConnection connection = SQLite::DBConnection::memory();

    REQUIRE(Execute(connection, "CREATE TABLE test (id INTEGER PRIMARY KEY, string TEXT, double REAL)") == 0);

    REQUIRE(Execute(connection, "INSERT INTO test VALUES (1, \"one\", 1.0)") == 1);
    REQUIRE(Execute(connection, "INSERT INTO test VALUES (2, \"two\", 2.0)") == 1);
    REQUIRE(Execute(connection, "INSERT INTO test VALUES (3, \"three\", 3.0)") == 1);

    std::vector<std::vector<std::pair<std::string, std::string> > > allColumnData;

    ExecuteCallback(
            connection,
            "SELECT * FROM test",
            [&allColumnData](const std::vector<std::string> &columnData, const std::vector<std::string> &columnNames) -> void {
                std::vector<std::pair<std::string, std::string> > columnDataPairs;
                for (size_t i = 0; i < columnData.size(); ++i) {
                    columnDataPairs.push_back(std::make_pair(columnNames[i], columnData[i]));
                }

                allColumnData.push_back(columnDataPairs);
            });

    REQUIRE(allColumnData.size() == 3);
    REQUIRE(allColumnData[0].size() == 3);
    REQUIRE(allColumnData[1].size() == 3);
    REQUIRE(allColumnData[2].size() == 3);
}
