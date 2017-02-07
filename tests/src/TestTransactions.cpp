#include "catch/catch.hpp"
#include "SQLiteXX.h"

#include <string>

TEST_CASE("Test Uses of Transactions", "[Transaction]") {
   SQLite::DBConnection connection = SQLite::DBConnection::memory();

   {
      SQLite::Transaction transaction(connection);

      REQUIRE(Execute(connection, "CREATE TABLE test (id INTEGER PRIMARY KEY, value TEXT)") == 0);
      REQUIRE(Execute(connection, "INSERT INTO test VALUES (NULL, \"first\")") == 1);

      REQUIRE_NOTHROW(transaction.commit());
   }

   {
      SQLite::Transaction transaction(connection);

      REQUIRE(Execute(connection, "INSERT INTO test VALUES (NULL, \"second\")") == 1);
   }

   SECTION("Check the results (should only be one row of results)") {
      SQLite::Statement query(connection, "SELECT * FROM test");
      int numRows = 0;
      for (SQLite::Row r : query) {
         REQUIRE(r.getInt(0) == 1);
         REQUIRE(r.getString(1) == std::string("first"));
         numRows++;
      }
      REQUIRE(numRows == 1);
   }
}

TEST_CASE("Test double transaction commit", "[Transaction]") {
   SQLite::DBConnection connection = SQLite::DBConnection::memory();

   SQLite::Transaction transaction(connection);

   REQUIRE(Execute(connection, "CREATE TABLE test (id INTEGER PRIMARY KEY, value TEXT)") == 0);
   REQUIRE(Execute(connection, "INSERT INTO test VALUES (NULL, \"first\")") == 1);

   REQUIRE_NOTHROW(transaction.commit());
   REQUIRE_THROWS_AS(transaction.commit(), SQLite::Exception);
}

TEST_CASE("Test using different transaction types", "[Transaction]") {
   SQLite::DBConnection connection = SQLite::DBConnection::memory();

   REQUIRE(Execute(connection, "CREATE TABLE test (id INTEGER PRIMARY KEY, value TEXT)") == 0);

   {
       SQLite::Transaction transaction(connection, SQLite::Transaction::DEFERRED);

       REQUIRE(Execute(connection, "INSERT INTO test VALUES (NULL, \"deferred\")") == 1);

       REQUIRE_NOTHROW(transaction.commit());
       REQUIRE_THROWS_AS(transaction.commit(), SQLite::Exception);
   }

   {
       SQLite::Transaction transaction(connection, SQLite::Transaction::IMMEDIATE);

       REQUIRE(Execute(connection, "INSERT INTO test VALUES (NULL, \"immediate\")") == 1);

       REQUIRE_NOTHROW(transaction.commit());
       REQUIRE_THROWS_AS(transaction.commit(), SQLite::Exception);
   }

   {
       SQLite::Transaction transaction(connection, SQLite::Transaction::EXCLUSIVE);

       REQUIRE(Execute(connection, "INSERT INTO test VALUES (NULL, \"exclusive\")") == 1);

       REQUIRE_NOTHROW(transaction.commit());
       REQUIRE_THROWS_AS(transaction.commit(), SQLite::Exception);
   }

   SECTION("Check the results (should be three rows of results)") {
      SQLite::Statement query(connection, "SELECT * FROM test");
      const std::vector<std::string> expectedValues = {
          "deferred",
          "immediate",
          "exclusive"
      };

      int numRows = 0;
      for (SQLite::Row r : query) {
         REQUIRE(r.getInt(0) == (numRows + 1));
         REQUIRE(r.getString(1) == expectedValues[numRows]);
         numRows++;
      }
      REQUIRE(numRows == 3);
   }
}
