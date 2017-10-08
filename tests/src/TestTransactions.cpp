#include "catch.hpp"
#include "SQLiteXX.h"

#include <string>

TEST_CASE("Test Uses of Transactions", "[Transaction]") {
   sqlite::dbconnection connection = sqlite::dbconnection::memory();

   {
      sqlite::deferred_transaction transaction(connection);

      REQUIRE(sqlite::execute(connection, "CREATE TABLE test (id INTEGER PRIMARY KEY, value TEXT)") == 0);
      REQUIRE(sqlite::execute(connection, "INSERT INTO test VALUES (NULL, \"first\")") == 1);

      REQUIRE_NOTHROW(transaction.commit());
   }

   {
      sqlite::deferred_transaction transaction(connection);

      REQUIRE(sqlite::execute(connection, "INSERT INTO test VALUES (NULL, \"second\")") == 1);
   }

   SECTION("Check the results (should only be one row of results)") {
      sqlite::statement query(connection, "SELECT * FROM test");
      int numRows = 0;
      for (sqlite::row r : query) {
         REQUIRE(r.get_int(0) == 1);
         REQUIRE(r.get_string(1) == std::string("first"));
         numRows++;
      }
      REQUIRE(numRows == 1);
   }
}

TEST_CASE("Test double transaction commit", "[Transaction]") {
   sqlite::dbconnection connection = sqlite::dbconnection::memory();

   sqlite::deferred_transaction transaction(connection);

   REQUIRE(sqlite::execute(connection, "CREATE TABLE test (id INTEGER PRIMARY KEY, value TEXT)") == 0);
   REQUIRE(sqlite::execute(connection, "INSERT INTO test VALUES (NULL, \"first\")") == 1);

   REQUIRE_NOTHROW(transaction.commit());
   REQUIRE_THROWS_AS(transaction.commit(), sqlite::exception);
}

TEST_CASE("Test using different transaction types", "[Transaction]") {
   sqlite::dbconnection connection = sqlite::dbconnection::memory();

   REQUIRE(sqlite::execute(connection, "CREATE TABLE test (id INTEGER PRIMARY KEY, value TEXT)") == 0);

   {
       sqlite::deferred_transaction transaction(connection);

       REQUIRE(sqlite::execute(connection, "INSERT INTO test VALUES (NULL, \"deferred\")") == 1);

       REQUIRE_NOTHROW(transaction.commit());
       REQUIRE_THROWS_AS(transaction.commit(), sqlite::exception);
   }

   {
       sqlite::immediate_transaction transaction(connection);

       REQUIRE(sqlite::execute(connection, "INSERT INTO test VALUES (NULL, \"immediate\")") == 1);

       REQUIRE_NOTHROW(transaction.commit());
       REQUIRE_THROWS_AS(transaction.commit(), sqlite::exception);
   }

   {
       sqlite::exclusive_transaction transaction(connection);

       REQUIRE(sqlite::execute(connection, "INSERT INTO test VALUES (NULL, \"exclusive\")") == 1);

       REQUIRE_NOTHROW(transaction.commit());
       REQUIRE_THROWS_AS(transaction.commit(), sqlite::exception);
   }

   SECTION("Check the results (should be three rows of results)") {
      sqlite::statement query(connection, "SELECT * FROM test");
      const std::vector<std::string> expectedValues = {
          "deferred",
          "immediate",
          "exclusive"
      };

      int numRows = 0;
      for (sqlite::row r : query) {
         REQUIRE(r.get_int(0) == (numRows + 1));
         REQUIRE(r.get_string(1) == expectedValues[numRows]);
         numRows++;
      }
      REQUIRE(numRows == 3);
   }
}
