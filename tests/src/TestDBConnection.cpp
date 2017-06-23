#include "catch/catch.hpp"
#include "SQLiteXX.h"

#include <string>
#include <utility>
#include <vector>

TEST_CASE("Combinations of OpenMode flags with new database", "[DBConnection]") {
    remove("testDBConnection.db");

    SECTION("Open file as read only but file does not exist") {
        REQUIRE_THROWS_AS(SQLite::DBConnection("testDBConnection.db", SQLite::OpenMode::ReadOnly), SQLite::Exception);
    }

    SECTION("Open and create file with ReadOnly") {
        REQUIRE_THROWS_AS(SQLite::DBConnection("testDBConnection.db", SQLite::OpenMode::ReadOnly | SQLite::OpenMode::Create), SQLite::Exception);
    }

    SECTION("Open and create file with ReadWrite") {
        REQUIRE_NOTHROW(SQLite::DBConnection("testDBConnection.db", SQLite::OpenMode::ReadWrite | SQLite::OpenMode::Create));
    }

    SECTION("Open, create, and write to file") {
        SQLite::DBConnection connection("testDBConnection.db", SQLite::OpenMode::ReadWrite | SQLite::OpenMode::Create);

        REQUIRE_NOTHROW(Execute(connection, "CREATE TABLE test (id INTEGER PRIMARY KEY, value TEXT)"));
        REQUIRE_NOTHROW(Execute(connection, "INSERT INTO test VALUES (NULL, \"one\")"));

        SQLite::Statement query(connection, "SELECT * FROM test");
        REQUIRE(query.getColumnCount() == 2);
    }
}

TEST_CASE("Combinations of OpenMode flags with already existing database", "[DBConnection]") {
    remove("testDBConnection.db");
    {
        SQLite::DBConnection connection("testDBConnection.db");

        REQUIRE_NOTHROW(Execute(connection, "CREATE TABLE test (id INTEGER PRIMARY KEY, value TEXT)"));
        REQUIRE_NOTHROW(Execute(connection, "INSERT INTO test VALUES (NULL, \"one\")"));

        SQLite::Statement query(connection, "SELECT * FROM test");
        REQUIRE(query.getColumnCount() == 2);
    }

    SECTION("Open ReadOnly") {
        SQLite::DBConnection connection("testDBConnection.db", SQLite::OpenMode::ReadOnly);

        SQLite::Statement query(connection, "SELECT * FROM test");
        REQUIRE(query.getColumnCount() == 2);

        REQUIRE_THROWS_AS(Execute(connection, "INSERT INTO test VALUES (NULL, \"one\")"), SQLite::Exception);
    }

    SECTION("Open ReadWrite") {
        SQLite::DBConnection connection("testDBConnection.db", SQLite::OpenMode::ReadWrite);

        REQUIRE_NOTHROW(Execute(connection, "INSERT INTO test VALUES (NULL, \"two\")"));

        SQLite::Statement query(connection, "SELECT * FROM test");
        REQUIRE(query.getColumnCount() == 2);
    }

    SECTION("Open ReadWrite and Create") {
        SQLite::DBConnection connection("testDBConnection.db", SQLite::OpenMode::ReadWrite | SQLite::OpenMode::Create);

        REQUIRE_NOTHROW(Execute(connection, "INSERT INTO test VALUES (NULL, \"two\")"));

        SQLite::Statement query(connection, "SELECT * FROM test");
        REQUIRE(query.getColumnCount() == 2);
    }
}

TEST_CASE("Combinations of OpenMode mutex flags", "[DBConnection]") {
    remove("testDBConnection.db");

    SECTION("Opening database with no Mutex") {
        SQLite::DBConnection connection(
            "testDBConnection.db",
            SQLite::OpenMode::ReadWrite | SQLite::OpenMode::Create | SQLite::OpenMode::NoMutex);

        REQUIRE_THROWS_AS(connection.getMutex(), SQLite::SQLiteXXException);
    }
}

TEST_CASE("DBConnection UTF8 Support", "[DBConnection]") {
    remove("testDBConnection_utf16.db");

    SECTION("Memory database") {
        SQLite::DBConnection connection = SQLite::DBConnection::wideMemory();

        REQUIRE_NOTHROW(Execute(connection, "CREATE TABLE test (id INTEGER PRIMARY KEY, value TEXT)"));
        REQUIRE_NOTHROW(Execute(connection, "INSERT INTO test VALUES (NULL, \"one\")"));

        SQLite::Statement query(connection, "SELECT * FROM test");
        REQUIRE(query.getColumnCount() == 2);
    }

    SECTION("File database using open") {
        SQLite::DBConnection connection;
        connection.open(u"testDBConnection_utf16.db");

        REQUIRE_NOTHROW(Execute(connection, "CREATE TABLE test (id INTEGER PRIMARY KEY, value TEXT)"));
        REQUIRE_NOTHROW(Execute(connection, "INSERT INTO test VALUES (NULL, \"one\")"));

        SQLite::Statement query(connection, "SELECT * FROM test");
        REQUIRE(query.getColumnCount() == 2);
    }

    SECTION("File database from contructor") {
        SQLite::DBConnection connection(u"testDBConnection_utf16.db");

        REQUIRE_NOTHROW(Execute(connection, "CREATE TABLE test (id INTEGER PRIMARY KEY, value TEXT)"));
        REQUIRE_NOTHROW(Execute(connection, "INSERT INTO test VALUES (NULL, \"one\")"));

        SQLite::Statement query(connection, "SELECT * FROM test");
        REQUIRE(query.getColumnCount() == 2);
    }
}
