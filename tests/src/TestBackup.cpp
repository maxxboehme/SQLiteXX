#include "catch.hpp"
#include <SQLiteXX.h>

TEST_CASE("Initialization with non empty database", "[Backup]") {
    SQLite::DBConnection src = SQLite::DBConnection::memory();
    Execute(src, "CREATE TABLE test (id INTEGER PRIMARY KEY, value TEXT)");

    REQUIRE(Execute(src, "INSERT INTO test VALUES (1, \"one\")") == 1);
    REQUIRE(Execute(src, "INSERT INTO test VALUES (2, \"two\")") == 1);

    SECTION("Backup to self") {
        REQUIRE_THROWS_AS(SQLite::Backup(src, src), SQLite::Exception);
    }
}

TEST_CASE("Backup process", "[Backup]") {
    SQLite::DBConnection src = SQLite::DBConnection::memory();
    Execute(src, "CREATE TABLE test (id INTEGER PRIMARY KEY, integerValue TEXT, doubleValue REAL)");
    REQUIRE(Execute(src, "INSERT INTO test VALUES (1, \"one\", 1.0)") == 1);
    REQUIRE(Execute(src, "INSERT INTO test VALUES (2, \"two\", 2.0)") == 1);

    SQLite::DBConnection dest = SQLite::DBConnection::memory();

    SECTION("Backup one page at a time") {
        SQLite::Backup backup(src, dest);

        REQUIRE(backup.step(1) == true);
        REQUIRE(backup.getTotalPageCount() == 2);
        REQUIRE(backup.getRemainingPageCount() == 1);
        REQUIRE(backup.step(1) == false);
        REQUIRE(backup.getRemainingPageCount() == 0);

        SQLite::Statement query(dest, "SELECT * FROM test ORDER BY id ASC");
        REQUIRE(query.step() == true);
        REQUIRE(query.getInt(0) == 1);
        REQUIRE(query.getString(1) == std::string("one"));
        REQUIRE(query.getDouble(2) == 1.0);
        REQUIRE(query.step() == true);
        REQUIRE(query.getInt(0) == 2);
        REQUIRE(query.getString(1) == std::string("two"));
        REQUIRE(query.getDouble(2) == 2.0);
        REQUIRE(query.step() == false);
    }

    SECTION("Backup all at once") {
        SQLite::Backup backup(src, dest);

        REQUIRE(backup.step() == false);
        REQUIRE(backup.getTotalPageCount() == 2);
        REQUIRE(backup.getRemainingPageCount() == 0);

        SQLite::Statement query(dest, "SELECT * FROM test ORDER BY id ASC");
        REQUIRE(query.step() == true);
        REQUIRE(query.getInt(0) == 1);
        REQUIRE(query.getString(1) == std::string("one"));
        REQUIRE(query.getDouble(2) == 1.0);
        REQUIRE(query.step() == true);
        REQUIRE(query.getInt(0) == 2);
        REQUIRE(query.getString(1) == std::string("two"));
        REQUIRE(query.getDouble(2) == 2.0);
        REQUIRE(query.step() == false);
    }

    SECTION("Save to disk") {
        remove("SaveToDiskResults.db");
        SQLite::SaveToDisk(src, "SaveToDiskResults.db");

        SQLite::DBConnection connection("SaveToDiskResults.db");

        SQLite::Statement query(connection, "SELECT * FROM test ORDER BY id ASC");
        REQUIRE(query.step() == true);
        REQUIRE(query.getInt(0) == 1);
        REQUIRE(query.getString(1) == std::string("one"));
        REQUIRE(query.getDouble(2) == 1.0);
        REQUIRE(query.step() == true);
        REQUIRE(query.getInt(0) == 2);
        REQUIRE(query.getString(1) == std::string("two"));
        REQUIRE(query.getDouble(2) == 2.0);
        REQUIRE(query.step() == false);
    }
}
