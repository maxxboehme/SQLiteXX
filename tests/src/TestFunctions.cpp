#include "SQLiteXX.h"
#include "catch.hpp"


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
                true));

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
                true));

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
                true,
                SQLite::TextEncoding::UTF8,
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
                true,
                SQLite::TextEncoding::UTF8,
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
                true));

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
                true));

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
                true));

        REQUIRE_THROWS_AS(SQLite::Statement(connection, "SELECT num, multiply(num, num, num) FROM test"), SQLite::Exception);
    }

    SECTION("Create function specific number of arguments and using less") {
        REQUIRE_NOTHROW(
            connection.createFunction(
                "multiply",
                [](int x, int y) -> int {
                    return x * y;
                },
                true));

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

class BadAllocAggregate {
    public:
    BadAllocAggregate() :
        m_sum(0)
    {}

    void step(int val) {
        m_sum += val;
        throw std::bad_alloc();
    }

    double finalize() {
        return m_sum;
    }

    private:
    int m_sum;
};

class SQLiteExceptionAggregate {
    public:
    SQLiteExceptionAggregate() :
        m_sum(0)
    {}

    void step(int val) {
        m_sum += val;
        throw SQLite::Exception(1, "test Exception");
    }

    double finalize() {
        return m_sum;
    }

    private:
    int m_sum;
};

class StdExceptionAggregate {
    public:
    StdExceptionAggregate() :
        m_sum(0)
    {}

    void step(int val) {
        m_sum += val;
        throw std::exception();
    }

    double finalize() {
        return m_sum;
    }

    private:
    int m_sum;
};

class RandomExceptionAggregate {
    public:
    RandomExceptionAggregate() :
        m_sum(0)
    {}

    void step(int val) {
        m_sum += val;
        throw "test exception";
    }

    double finalize() {
        return m_sum;
    }

    private:
    int m_sum;
};

class BadAllocAggregateFinalize {
    public:
    BadAllocAggregateFinalize() :
        m_sum(0)
    {}

    void step(int val) {
        m_sum += val;
    }

    double finalize() {
        throw std::bad_alloc();
        return m_sum;
    }

    private:
    int m_sum;
};

class SQLiteExceptionAggregateFinalize {
    public:
    SQLiteExceptionAggregateFinalize() :
        m_sum(0)
    {}

    void step(int val) {
        m_sum += val;
    }

    double finalize() {
        throw SQLite::Exception(1, "test Exception");
        return m_sum;
    }

    private:
    int m_sum;
};

class StdExceptionAggregateFinalize {
    public:
    StdExceptionAggregateFinalize() :
        m_sum(0)
    {}

    void step(int val) {
        m_sum += val;
    }

    double finalize() {
        throw std::exception();
        return m_sum;
    }

    private:
    int m_sum;
};

class RandomExceptionAggregateFinalize {
    public:
    RandomExceptionAggregateFinalize() :
        m_sum(0)
    {}

    void step(int val) {
        m_sum += val;
    }

    double finalize() {
        throw "test exception";
        return m_sum;
    }

    private:
    int m_sum;
};

TEST_CASE("Throwing Exceptions from functions", "[Functions]") {
    SQLite::DBConnection connection = SQLite::DBConnection::memory();

    REQUIRE(Execute(connection, "CREATE TABLE test (num INT)") == 0);
    REQUIRE(Execute(connection, "INSERT INTO test VALUES (1)") == 1);
    REQUIRE(Execute(connection, "INSERT INTO test VALUES (2)") == 1);
    REQUIRE(Execute(connection, "INSERT INTO test VALUES (3)") == 1);
    REQUIRE(Execute(connection, "INSERT INTO test VALUES (123)") == 1);

    SECTION("Generic Scalar function Exceptions") {
        connection.createGeneralFunction(
                "badAlloc",
                [](const std::vector<SQLite::Value>&) -> int {
                    throw std::bad_alloc();
                    return 1;
                });

        connection.createGeneralFunction(
                "SQLiteException",
                [](const std::vector<SQLite::Value>&) -> int {
                    throw SQLite::Exception(1, "test Exception");
                    return 1;
                });

        connection.createGeneralFunction(
                "stdException",
                [](const std::vector<SQLite::Value>&) -> int {
                    throw std::exception();
                    return 1;
                });

        connection.createGeneralFunction(
                "randomException",
                [](const std::vector<SQLite::Value>&) -> int {
                    throw "test Exception";
                    return 1;
                });


        REQUIRE_THROWS_AS(SQLite::Statement(connection, "SELECT badAlloc(num) FROM test").step(), SQLite::Exception);
        REQUIRE_THROWS_AS(SQLite::Statement(connection, "SELECT SQLiteException(num) FROM test").step(), SQLite::Exception);
        REQUIRE_THROWS_AS(SQLite::Statement(connection, "SELECT stdException(num) FROM test").step(), SQLite::Exception);
        REQUIRE_THROWS_AS(SQLite::Statement(connection, "SELECT randomException(num) FROM test").step(), SQLite::Exception);
    }

    SECTION("Scalar function Exceptions") {
        connection.createFunction(
                "badAlloc",
                [](int) -> int {
                    throw std::bad_alloc();
                    return 1;
                });

        connection.createFunction(
                "SQLiteException",
                [](int) -> int {
                    throw SQLite::Exception(1, "test Exception");
                    return 1;
                });

        connection.createFunction(
                "stdException",
                [](int) -> int {
                    throw std::exception();
                    return 1;
                });

        connection.createFunction(
                "randomException",
                [](int) -> int {
                    throw "test Exception";
                    return 1;
                });


        REQUIRE_THROWS_AS(SQLite::Statement(connection, "SELECT badAlloc(num) FROM test").step(), SQLite::Exception);
        REQUIRE_THROWS_AS(SQLite::Statement(connection, "SELECT SQLiteException(num) FROM test").step(), SQLite::Exception);
        REQUIRE_THROWS_AS(SQLite::Statement(connection, "SELECT stdException(num) FROM test").step(), SQLite::Exception);
        REQUIRE_THROWS_AS(SQLite::Statement(connection, "SELECT randomException(num) FROM test").step(), SQLite::Exception);
    }

    SECTION("Aggregate function exceptions in step") {
        REQUIRE_NOTHROW(connection.createAggregate<BadAllocAggregate>("badAlloc"));
        REQUIRE_NOTHROW(connection.createAggregate<SQLiteExceptionAggregate>("SQLiteException"));
        REQUIRE_NOTHROW(connection.createAggregate<StdExceptionAggregate>("stdException"));
        REQUIRE_NOTHROW(connection.createAggregate<RandomExceptionAggregate>("randomException"));

        REQUIRE_THROWS_AS(SQLite::Statement(connection, "SELECT badAlloc(num) FROM test").step(), SQLite::Exception);
        REQUIRE_THROWS_AS(SQLite::Statement(connection, "SELECT SQLiteException(num) FROM test").step(), SQLite::Exception);
        REQUIRE_THROWS_AS(SQLite::Statement(connection, "SELECT stdException(num) FROM test").step(), SQLite::Exception);
        REQUIRE_THROWS_AS(SQLite::Statement(connection, "SELECT randomException(num) FROM test").step(), SQLite::Exception);
    }

    SECTION("Aggregate function exceptions in finalize") {
        REQUIRE_NOTHROW(connection.createAggregate<BadAllocAggregateFinalize>("badAlloc"));
        REQUIRE_NOTHROW(connection.createAggregate<SQLiteExceptionAggregateFinalize>("SQLiteException"));
        REQUIRE_NOTHROW(connection.createAggregate<StdExceptionAggregateFinalize>("stdException"));
        REQUIRE_NOTHROW(connection.createAggregate<RandomExceptionAggregateFinalize>("randomException"));

        REQUIRE_THROWS_AS(SQLite::Statement(connection, "SELECT badAlloc(num) FROM test").step(), SQLite::Exception);
        REQUIRE_THROWS_AS(SQLite::Statement(connection, "SELECT SQLiteException(num) FROM test").step(), SQLite::Exception);
        REQUIRE_THROWS_AS(SQLite::Statement(connection, "SELECT stdException(num) FROM test").step(), SQLite::Exception);
        REQUIRE_THROWS_AS(SQLite::Statement(connection, "SELECT randomException(num) FROM test").step(), SQLite::Exception);
    }
}
