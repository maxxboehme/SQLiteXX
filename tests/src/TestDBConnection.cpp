#include "catch.hpp"
#include "SQLiteXX.h"

#include <string>
#include <utility>
#include <vector>

TEST_CASE("Combinations of openmode flags with new database", "[DBConnection]") {
    remove("testDBConnection.db");
    remove("testDBConnection_utf16.db");

    SECTION("Open file as read only but file does not exist") {
        REQUIRE_THROWS_AS(sqlite::dbconnection("testDBConnection.db", sqlite::openmode::read_only), sqlite::exception);
    }

    SECTION("Open and create file with ReadOnly") {
        REQUIRE_THROWS_AS(sqlite::dbconnection("testDBConnection.db", sqlite::openmode::read_only | sqlite::openmode::create), sqlite::exception);
    }

    SECTION("Open and create file with ReadWrite") {
        REQUIRE_NOTHROW(sqlite::dbconnection("testDBConnection.db", sqlite::openmode::read_write | sqlite::openmode::create));
    }

    SECTION("Open, create, and write to file") {
        sqlite::dbconnection connection("testDBConnection.db", sqlite::openmode::read_write | sqlite::openmode::create);

        REQUIRE_NOTHROW(sqlite::execute(connection, "CREATE TABLE test (id INTEGER PRIMARY KEY, value TEXT)"));
        REQUIRE_NOTHROW(sqlite::execute(connection, "INSERT INTO test VALUES (NULL, \"one\")"));

        sqlite::statement query(connection, "SELECT * FROM test");
        REQUIRE(query.column_count() == 2);
    }
}

TEST_CASE("Combinations of openmode flags with already existing database", "[DBConnection]") {
    remove("testDBConnection.db");
    {
        sqlite::dbconnection connection("testDBConnection.db");

        REQUIRE_NOTHROW(sqlite::execute(connection, "CREATE TABLE test (id INTEGER PRIMARY KEY, value TEXT)"));
        REQUIRE_NOTHROW(sqlite::execute(connection, "INSERT INTO test VALUES (NULL, \"one\")"));

        sqlite::statement query(connection, "SELECT * FROM test");
        REQUIRE(query.column_count() == 2);
    }

    SECTION("Open ReadOnly") {
        sqlite::dbconnection connection("testDBConnection.db", sqlite::openmode::read_only);

        sqlite::statement query(connection, "SELECT * FROM test");
        REQUIRE(query.column_count() == 2);

        REQUIRE_THROWS_AS(sqlite::execute(connection, "INSERT INTO test VALUES (NULL, \"one\")"), sqlite::exception);
    }

    SECTION("Open ReadWrite") {
        sqlite::dbconnection connection("testDBConnection.db", sqlite::openmode::read_write);

        REQUIRE_NOTHROW(sqlite::execute(connection, "INSERT INTO test VALUES (NULL, \"two\")"));

        sqlite::statement query(connection, "SELECT * FROM test");
        REQUIRE(query.column_count() == 2);
    }

    SECTION("Open ReadWrite and Create") {
        sqlite::dbconnection connection("testDBConnection.db", sqlite::openmode::read_write | sqlite::openmode::create);

        REQUIRE_NOTHROW(sqlite::execute(connection, "INSERT INTO test VALUES (NULL, \"two\")"));

        sqlite::statement query(connection, "SELECT * FROM test");
        REQUIRE(query.column_count() == 2);
    }
}

TEST_CASE("Combinations of openmode mutex flags", "[DBConnection]") {
    remove("testDBConnection.db");

    SECTION("Opening database with no Mutex") {
        sqlite::dbconnection connection(
            "testDBConnection.db",
            sqlite::openmode::read_write | sqlite::openmode::create | sqlite::openmode::no_mutex);

        REQUIRE_THROWS_AS(connection.mutex(), sqlite::SQLiteXXException);
    }
}

TEST_CASE("DBConnection UTF8 Support", "[DBConnection]") {
    remove("testDBConnection_utf16.db");

    SECTION("Memory database") {
        sqlite::dbconnection connection = sqlite::dbconnection::wide_memory();

        REQUIRE_NOTHROW(sqlite::execute(connection, "CREATE TABLE test (id INTEGER PRIMARY KEY, value TEXT)"));
        REQUIRE_NOTHROW(sqlite::execute(connection, "INSERT INTO test VALUES (NULL, \"one\")"));

        sqlite::statement query(connection, "SELECT * FROM test");
        REQUIRE(query.column_count() == 2);
    }

    SECTION("File database using open") {
        sqlite::dbconnection connection;
        connection.open(u"testDBConnection_utf16.db");

        REQUIRE_NOTHROW(sqlite::execute(connection, "CREATE TABLE test (id INTEGER PRIMARY KEY, value TEXT)"));
        REQUIRE_NOTHROW(sqlite::execute(connection, "INSERT INTO test VALUES (NULL, \"one\")"));

        sqlite::statement query(connection, "SELECT * FROM test");
        REQUIRE(query.column_count() == 2);
    }

    SECTION("File database from contructor") {
        sqlite::dbconnection connection(u"testDBConnection_utf16.db");

        REQUIRE_NOTHROW(sqlite::execute(connection, "CREATE TABLE test (id INTEGER PRIMARY KEY, value TEXT)"));
        REQUIRE_NOTHROW(sqlite::execute(connection, "INSERT INTO test VALUES (NULL, \"one\")"));

        sqlite::statement query(connection, "SELECT * FROM test");
        REQUIRE(query.column_count() == 2);
    }
}

TEST_CASE("DBConnectionTest Assignment Operators", "[DBConnection]") {
    sqlite::dbconnection connection = sqlite::dbconnection::memory();

    REQUIRE(sqlite::execute(connection, "CREATE TABLE test (txt1 TEXT, txt2 TEXT)") == 0);
    REQUIRE(sqlite::execute(connection, "INSERT INTO test VALUES (\"first\", \"second\")") == 1);

    sqlite::statement selectStatement(connection, "SELECT * FROM test");
    REQUIRE(selectStatement.column_count() == 2);

    SECTION("L-value assignment operator") {
        sqlite::dbconnection testConnection;

        // Testing assignment
        testConnection = connection;

        sqlite::statement query(testConnection, "SELECT * FROM test");
        REQUIRE(query.column_count() == 2);
    }

    SECTION("R-value assignment operator") {
        sqlite::dbconnection testConnection;

        // Testing assignment
        testConnection = sqlite::dbconnection(connection);

        sqlite::statement query(testConnection, "SELECT * FROM test");
        REQUIRE(query.column_count() == 2);
    }
}
