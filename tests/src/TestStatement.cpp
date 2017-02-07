#include "catch/catch.hpp"
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
    // REQUIRE_THROWS_AS(query.step(), SQLite::Exception);

    // Try to insert a new row with the same PRIMARY KEY: "UNIQUE constraint failed: test.id"
    SQLite::Statement insert(connection, "INSERT INTO test VALUES (1, \"exception\", 456, 0.456)");
    REQUIRE_THROWS_AS(insert.step(), SQLite::Exception);
    // reset should throw again
    REQUIRE_THROWS_AS(insert.reset(), SQLite::Exception);

    REQUIRE_THROWS_AS(Execute(connection, "INSERT INTO test VALUES (1, \"exception\", 456, 0.456)"), SQLite::Exception);
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
