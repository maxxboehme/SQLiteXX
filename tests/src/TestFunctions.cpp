#include "SQLiteXX.h"
#include "catch/catch.hpp"


int testGeneralMultiply(const std::vector<SQLite::Value> &values) {
    int product = 1;
    for (size_t i = 0; i < values.size(); ++i) {
        product *= values[i].getInt();
    }

    return product;
}

TEST_CASE("Create General Scalar Functions", "[Functions]") {
    SQLite::DBConnection connection = SQLite::DBConnection::memory();

    REQUIRE(Execute(connection, "CREATE TABLE test (num INT)") == 0);
    REQUIRE(Execute(connection, "INSERT INTO test VALUES (1)") == 1);
    REQUIRE(Execute(connection, "INSERT INTO test VALUES (2)") == 1);
    REQUIRE(Execute(connection, "INSERT INTO test VALUES (3)") == 1);
    REQUIRE(Execute(connection, "INSERT INTO test VALUES (123)") == 1);

    std::vector<std::pair<int, int>> expectedValues = {
        std::make_pair(1, 1),
        std::make_pair(2, 4),
        std::make_pair(3, 9),
        std::make_pair(123, 15129)
    };

    SECTION("Create function with function reference") {
        REQUIRE_NOTHROW(
                connection.createGeneralFunction(
                "multiply",
                testGeneralMultiply,
                SQLite::TextEncoding::UTF8 | SQLite::FunctionType::Deterministic));

        int i = 0;
        for (auto row : SQLite::Statement(connection, "SELECT num, multiply(num, num) FROM test")) {
            REQUIRE(expectedValues[i].first == row.getInt(0));
            REQUIRE(expectedValues[i].second == row.getInt(1));
            ++i;
        }
    }

    SECTION("Create function with lambda") {
        REQUIRE_NOTHROW(
            connection.createGeneralFunction(
                "multiply",
                [](const std::vector<SQLite::Value> &values) -> int {
                    int product = 1;
                    for (size_t i = 0; i < values.size(); ++i) {
                        product *= values[i].getInt();
                    }
                    return product;
                },
                SQLite::TextEncoding::UTF8 | SQLite::FunctionType::Deterministic));

        int i = 0;
        for (auto row : SQLite::Statement(connection, "SELECT num, multiply(num, num) FROM test")) {
            REQUIRE(expectedValues[i].first == row.getInt(0));
            REQUIRE(expectedValues[i].second == row.getInt(1));
            ++i;
        }
    }

    SECTION("Create function specific number of arguments and using more") {
        REQUIRE_NOTHROW(
            connection.createGeneralFunction(
                "multiply",
                [](const std::vector<SQLite::Value> &values) -> int {
                    return values[0].getInt() * values[1].getInt();
                },
                SQLite::TextEncoding::UTF8 | SQLite::FunctionType::Deterministic,
                2));

        REQUIRE_THROWS_AS(SQLite::Statement(connection, "SELECT num, multiply(num, num, num) FROM test"), SQLite::Exception);
    }

    SECTION("Create function specific number of arguments and using less") {
        REQUIRE_NOTHROW(
            connection.createGeneralFunction(
                "multiply",
                [](const std::vector<SQLite::Value> &values) -> int {
                    return values[0].getInt() * values[1].getInt();
                },
                SQLite::TextEncoding::UTF8 | SQLite::FunctionType::Deterministic,
                2));

        REQUIRE_THROWS_AS(SQLite::Statement(connection, "SELECT num, multiply(num) FROM test"), SQLite::Exception);
    }
}

int testMultiply(int x, int y) {
    return x * y;
}

TEST_CASE("Create Scalar Function", "[Functions]") {
    SQLite::DBConnection connection = SQLite::DBConnection::memory();

    REQUIRE(Execute(connection, "CREATE TABLE test (num INT)") == 0);
    REQUIRE(Execute(connection, "INSERT INTO test VALUES (1)") == 1);
    REQUIRE(Execute(connection, "INSERT INTO test VALUES (2)") == 1);
    REQUIRE(Execute(connection, "INSERT INTO test VALUES (3)") == 1);
    REQUIRE(Execute(connection, "INSERT INTO test VALUES (123)") == 1);

    std::vector<std::pair<int, int>> expectedValues = {
        std::make_pair(1, 1),
        std::make_pair(2, 4),
        std::make_pair(3, 9),
        std::make_pair(123, 15129)
    };

    SECTION("Create function with function reference") {
        REQUIRE_NOTHROW(
                connection.createFunction(
                "multiply",
                testMultiply,
                SQLite::TextEncoding::UTF8 | SQLite::FunctionType::Deterministic));

        int i = 0;
        for (auto row : SQLite::Statement(connection, "SELECT num, multiply(num, num) FROM test")) {
            REQUIRE(expectedValues[i].first == row.getInt(0));
            REQUIRE(expectedValues[i].second == row.getInt(1));
            ++i;
        }
    }

    SECTION("Create function with lambda") {
        REQUIRE_NOTHROW(
            connection.createFunction(
                "multiply",
                [](int x, int y) -> int {
                    return x * y;
                },
                SQLite::TextEncoding::UTF8 | SQLite::FunctionType::Deterministic));

        int i = 0;
        for (auto row : SQLite::Statement(connection, "SELECT num, multiply(num, num) FROM test")) {
            REQUIRE(expectedValues[i].first == row.getInt(0));
            REQUIRE(expectedValues[i].second == row.getInt(1));
            ++i;
        }
    }

    SECTION("Create function specific number of arguments and using more") {
        REQUIRE_NOTHROW(
            connection.createFunction(
                "multiply",
                [](int x, int y) -> int {
                    return x * y;
                },
                SQLite::TextEncoding::UTF8 | SQLite::FunctionType::Deterministic));

        REQUIRE_THROWS_AS(SQLite::Statement(connection, "SELECT num, multiply(num, num, num) FROM test"), SQLite::Exception);
    }

    SECTION("Create function specific number of arguments and using less") {
        REQUIRE_NOTHROW(
            connection.createFunction(
                "multiply",
                [](int x, int y) -> int {
                    return x * y;
                },
                SQLite::TextEncoding::UTF8 | SQLite::FunctionType::Deterministic));

        REQUIRE_THROWS_AS(SQLite::Statement(connection, "SELECT num, multiply(num) FROM test"), SQLite::Exception);
    }
}

class MySum {
    public:
    MySum() :
        m_sum(0)
    {}

    void step(int val) {
        m_sum += val;
    }

    double finalize() {
        return m_sum;
    }

    private:
    int m_sum;
};

TEST_CASE("Create Aggregate Function", "[Functions]") {
    SQLite::DBConnection connection = SQLite::DBConnection::memory();

    REQUIRE(Execute(connection, "CREATE TABLE test (num INT)") == 0);
    REQUIRE(Execute(connection, "INSERT INTO test VALUES (1)") == 1);
    REQUIRE(Execute(connection, "INSERT INTO test VALUES (2)") == 1);
    REQUIRE(Execute(connection, "INSERT INTO test VALUES (3)") == 1);
    REQUIRE(Execute(connection, "INSERT INTO test VALUES (123)") == 1);

    std::vector<int> expectedValues = {
        129
    };

    REQUIRE_NOTHROW(connection.createAggregate<MySum>("MySum"));

    int i = 0;
    for (auto row : SQLite::Statement(connection, "SELECT MySum(num) FROM test")) {
        REQUIRE(i == 0);
        REQUIRE(expectedValues[i] == row.getInt(0));
        ++i;
    }

    int j = 0;
    for (auto row : SQLite::Statement(connection, "SELECT MySum(num) FROM test")) {
        REQUIRE(j == 0);
        REQUIRE(expectedValues[j] == row.getInt(0));
        ++j;
    }
}
