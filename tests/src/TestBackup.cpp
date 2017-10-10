#include "catch.hpp"
#include <SQLiteXX.h>

TEST_CASE("Initialization with non empty database", "[Backup]") {
    sqlite::dbconnection src = sqlite::dbconnection::memory();
    sqlite::execute(src, "CREATE TABLE test (id INTEGER PRIMARY KEY, value TEXT)");

    REQUIRE(sqlite::execute(src, "INSERT INTO test VALUES (1, \"one\")") == 1);
    REQUIRE(sqlite::execute(src, "INSERT INTO test VALUES (2, \"two\")") == 1);

    SECTION("Backup to self") {
        REQUIRE_THROWS_AS(sqlite::backup(src, src), sqlite::exception);
    }

    SECTION("Backup to read-only file") {
        remove("TestBackup.db");
        sqlite::dbconnection create_database("TestBackup.db");

        sqlite::dbconnection destination("TestBackup.db", sqlite::openmode::read_only);
        sqlite::backup backup(src, destination);
        REQUIRE_THROWS_AS(backup.step(), sqlite::exception);
    }
}

TEST_CASE("Backup process", "[Backup]") {
    sqlite::dbconnection src = sqlite::dbconnection::memory();
    sqlite::execute(src, "CREATE TABLE test (id INTEGER PRIMARY KEY, integerValue TEXT, doubleValue REAL)");
    REQUIRE(sqlite::execute(src, "INSERT INTO test VALUES (1, \"one\", 1.0)") == 1);
    REQUIRE(sqlite::execute(src, "INSERT INTO test VALUES (2, \"two\", 2.0)") == 1);

    sqlite::dbconnection dest = sqlite::dbconnection::memory();

    SECTION("Backup one page at a time") {
        sqlite::backup backup(src, dest);

        REQUIRE(backup.step(1) == true);
        REQUIRE(backup.total_page_count() == 2);
        REQUIRE(backup.remaining_page_count() == 1);
        REQUIRE(backup.step(1) == false);
        REQUIRE(backup.remaining_page_count() == 0);

        sqlite::statement query(dest, "SELECT * FROM test ORDER BY id ASC");
        REQUIRE(query.step() == true);
        REQUIRE(query.get_int(0) == 1);
        REQUIRE(query.get_string(1) == std::string("one"));
        REQUIRE(query.get_double(2) == 1.0);
        REQUIRE(query.step() == true);
        REQUIRE(query.get_int(0) == 2);
        REQUIRE(query.get_string(1) == std::string("two"));
        REQUIRE(query.get_double(2) == 2.0);
        REQUIRE(query.step() == false);
    }

    SECTION("Backup all at once") {
        sqlite::backup backup(src, dest);

        REQUIRE(backup.step() == false);
        REQUIRE(backup.total_page_count() == 2);
        REQUIRE(backup.remaining_page_count() == 0);

        sqlite::statement query(dest, "SELECT * FROM test ORDER BY id ASC");
        REQUIRE(query.step() == true);
        REQUIRE(query.get_int(0) == 1);
        REQUIRE(query.get_string(1) == std::string("one"));
        REQUIRE(query.get_double(2) == 1.0);
        REQUIRE(query.step() == true);
        REQUIRE(query.get_int(0) == 2);
        REQUIRE(query.get_string(1) == std::string("two"));
        REQUIRE(query.get_double(2) == 2.0);
        REQUIRE(query.step() == false);
    }

    SECTION("Save to disk") {
        remove("SaveToDiskResults.db");
        sqlite::save(src, "SaveToDiskResults.db");

        sqlite::dbconnection connection("SaveToDiskResults.db");

        sqlite::statement query(connection, "SELECT * FROM test ORDER BY id ASC");
        REQUIRE(query.step() == true);
        REQUIRE(query.get_int(0) == 1);
        REQUIRE(query.get_string(1) == std::string("one"));
        REQUIRE(query.get_double(2) == 1.0);
        REQUIRE(query.step() == true);
        REQUIRE(query.get_int(0) == 2);
        REQUIRE(query.get_string(1) == std::string("two"));
        REQUIRE(query.get_double(2) == 2.0);
        REQUIRE(query.step() == false);
    }
}
