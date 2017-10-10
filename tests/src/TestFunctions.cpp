#include "SQLiteXX.h"
#include "catch.hpp"


int testGeneralMultiply(const std::vector<sqlite::value> &values) {
    int product = 1;
    for (size_t i = 0; i < values.size(); ++i) {
        product *= values[i].as_int();
    }

    return product;
}

TEST_CASE("Create General Scalar Functions", "[Functions]") {
    sqlite::dbconnection connection = sqlite::dbconnection::memory();

    REQUIRE(sqlite::execute(connection, "CREATE TABLE test (num INT)") == 0);
    REQUIRE(sqlite::execute(connection, "INSERT INTO test VALUES (1)") == 1);
    REQUIRE(sqlite::execute(connection, "INSERT INTO test VALUES (2)") == 1);
    REQUIRE(sqlite::execute(connection, "INSERT INTO test VALUES (3)") == 1);
    REQUIRE(sqlite::execute(connection, "INSERT INTO test VALUES (123)") == 1);

    std::vector<std::pair<int, int>> expectedValues = {
        std::make_pair(1, 1),
        std::make_pair(2, 4),
        std::make_pair(3, 9),
        std::make_pair(123, 15129)
    };

    SECTION("Create function with function reference") {
        REQUIRE_NOTHROW(
                connection.create_general_function(
                "multiply",
                testGeneralMultiply,
                true));

        int i = 0;
        for (auto row : sqlite::statement(connection, "SELECT num, multiply(num, num) FROM test")) {
            REQUIRE(expectedValues[i].first == row.get_int(0));
            REQUIRE(expectedValues[i].second == row.get_int(1));
            ++i;
        }
    }

    SECTION("Create function with lambda") {
        REQUIRE_NOTHROW(
            connection.create_general_function(
                "multiply",
                [](const std::vector<sqlite::value> &values) -> int {
                    int product = 1;
                    for (size_t i = 0; i < values.size(); ++i) {
                        product *= values[i].as_int();
                    }
                    return product;
                },
                true));

        int i = 0;
        for (auto row : sqlite::statement(connection, "SELECT num, multiply(num, num) FROM test")) {
            REQUIRE(expectedValues[i].first == row.get_int(0));
            REQUIRE(expectedValues[i].second == row.get_int(1));
            ++i;
        }
    }

    SECTION("Create function specific number of arguments and using more") {
        REQUIRE_NOTHROW(
            connection.create_general_function(
                "multiply",
                [](const std::vector<sqlite::value> &values) -> int {
                    return values[0].as_int() * values[1].as_int();
                },
                true,
                sqlite::textencoding::utf8,
                2));

        REQUIRE_THROWS_AS(sqlite::statement(connection, "SELECT num, multiply(num, num, num) FROM test"), sqlite::exception);
    }

    SECTION("Create function specific number of arguments and using less") {
        REQUIRE_NOTHROW(
            connection.create_general_function(
                "multiply",
                [](const std::vector<sqlite::value> &values) -> int {
                    return values[0].as_int() * values[1].as_int();
                },
                true,
                sqlite::textencoding::utf8,
                2));

        REQUIRE_THROWS_AS(sqlite::statement(connection, "SELECT num, multiply(num) FROM test"), sqlite::exception);
    }
}

int testMultiply(int x, int y) {
    return x * y;
}

TEST_CASE("Create Scalar Function", "[Functions]") {
    sqlite::dbconnection connection = sqlite::dbconnection::memory();

    REQUIRE(sqlite::execute(connection, "CREATE TABLE test (num INT)") == 0);
    REQUIRE(sqlite::execute(connection, "INSERT INTO test VALUES (1)") == 1);
    REQUIRE(sqlite::execute(connection, "INSERT INTO test VALUES (2)") == 1);
    REQUIRE(sqlite::execute(connection, "INSERT INTO test VALUES (3)") == 1);
    REQUIRE(sqlite::execute(connection, "INSERT INTO test VALUES (123)") == 1);

    std::vector<std::pair<int, int>> expectedValues = {
        std::make_pair(1, 1),
        std::make_pair(2, 4),
        std::make_pair(3, 9),
        std::make_pair(123, 15129)
    };

    SECTION("Create function with function reference") {
        REQUIRE_NOTHROW(
                connection.create_function(
                "multiply",
                testMultiply,
                true));

        int i = 0;
        for (auto row : sqlite::statement(connection, "SELECT num, multiply(num, num) FROM test")) {
            REQUIRE(expectedValues[i].first == row.get_int(0));
            REQUIRE(expectedValues[i].second == row.get_int(1));
            ++i;
        }
    }

    SECTION("Create function with lambda") {
        REQUIRE_NOTHROW(
            connection.create_function(
                "multiply",
                [](int x, int y) -> int {
                    return x * y;
                },
                true));

        int i = 0;
        for (auto row : sqlite::statement(connection, "SELECT num, multiply(num, num) FROM test")) {
            REQUIRE(expectedValues[i].first == row.get_int(0));
            REQUIRE(expectedValues[i].second == row.get_int(1));
            ++i;
        }
    }

    SECTION("Create function specific number of arguments and using more") {
        REQUIRE_NOTHROW(
            connection.create_function(
                "multiply",
                [](int x, int y) -> int {
                    return x * y;
                },
                true));

        REQUIRE_THROWS_AS(sqlite::statement(connection, "SELECT num, multiply(num, num, num) FROM test"), sqlite::exception);
    }

    SECTION("Create function specific number of arguments and using less") {
        REQUIRE_NOTHROW(
            connection.create_function(
                "multiply",
                [](int x, int y) -> int {
                    return x * y;
                },
                true));

        REQUIRE_THROWS_AS(sqlite::statement(connection, "SELECT num, multiply(num) FROM test"), sqlite::exception);
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
    sqlite::dbconnection connection = sqlite::dbconnection::memory();

    REQUIRE(sqlite::execute(connection, "CREATE TABLE test (num INT)") == 0);
    REQUIRE(sqlite::execute(connection, "INSERT INTO test VALUES (1)") == 1);
    REQUIRE(sqlite::execute(connection, "INSERT INTO test VALUES (2)") == 1);
    REQUIRE(sqlite::execute(connection, "INSERT INTO test VALUES (3)") == 1);
    REQUIRE(sqlite::execute(connection, "INSERT INTO test VALUES (123)") == 1);

    std::vector<int> expectedValues = {
        129
    };

    REQUIRE_NOTHROW(connection.create_aggregate<MySum>("MySum", true));

    int i = 0;
    for (auto row : sqlite::statement(connection, "SELECT MySum(num) FROM test")) {
        REQUIRE(i == 0);
        REQUIRE(expectedValues[i] == row.get_int(0));
        ++i;
    }

    int j = 0;
    for (auto row : sqlite::statement(connection, "SELECT MySum(num) FROM test")) {
        REQUIRE(j == 0);
        REQUIRE(expectedValues[j] == row.get_int(0));
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
        throw sqlite::exception(1, "test Exception");
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
        throw sqlite::exception(1, "test Exception");
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
    sqlite::dbconnection connection = sqlite::dbconnection::memory();

    REQUIRE(sqlite::execute(connection, "CREATE TABLE test (num INT)") == 0);
    REQUIRE(sqlite::execute(connection, "INSERT INTO test VALUES (1)") == 1);
    REQUIRE(sqlite::execute(connection, "INSERT INTO test VALUES (2)") == 1);
    REQUIRE(sqlite::execute(connection, "INSERT INTO test VALUES (3)") == 1);
    REQUIRE(sqlite::execute(connection, "INSERT INTO test VALUES (123)") == 1);

    SECTION("Generic Scalar function Exceptions") {
        connection.create_general_function(
                "badAlloc",
                [](const std::vector<sqlite::value>&) -> int {
                    throw std::bad_alloc();
                    return 1;
                });

        connection.create_general_function(
                "SQLiteException",
                [](const std::vector<sqlite::value>&) -> int {
                    throw sqlite::exception(1, "test Exception");
                    return 1;
                });

        connection.create_general_function(
                "stdException",
                [](const std::vector<sqlite::value>&) -> int {
                    throw std::exception();
                    return 1;
                });

        connection.create_general_function(
                "randomException",
                [](const std::vector<sqlite::value>&) -> int {
                    throw "test Exception";
                    return 1;
                });


        REQUIRE_THROWS_AS(sqlite::statement(connection, "SELECT badAlloc(num) FROM test").step(), sqlite::exception);
        REQUIRE_THROWS_AS(sqlite::statement(connection, "SELECT SQLiteException(num) FROM test").step(), sqlite::exception);
        REQUIRE_THROWS_AS(sqlite::statement(connection, "SELECT stdException(num) FROM test").step(), sqlite::exception);
        REQUIRE_THROWS_AS(sqlite::statement(connection, "SELECT randomException(num) FROM test").step(), sqlite::exception);
    }

    SECTION("Scalar function Exceptions") {
        connection.create_function(
                "badAlloc",
                [](int) -> int {
                    throw std::bad_alloc();
                    return 1;
                });

        connection.create_function(
                "SQLiteException",
                [](int) -> int {
                    throw sqlite::exception(1, "test Exception");
                    return 1;
                });

        connection.create_function(
                "stdException",
                [](int) -> int {
                    throw std::exception();
                    return 1;
                });

        connection.create_function(
                "randomException",
                [](int) -> int {
                    throw "test Exception";
                    return 1;
                });


        REQUIRE_THROWS_AS(sqlite::statement(connection, "SELECT badAlloc(num) FROM test").step(), sqlite::exception);
        REQUIRE_THROWS_AS(sqlite::statement(connection, "SELECT SQLiteException(num) FROM test").step(), sqlite::exception);
        REQUIRE_THROWS_AS(sqlite::statement(connection, "SELECT stdException(num) FROM test").step(), sqlite::exception);
        REQUIRE_THROWS_AS(sqlite::statement(connection, "SELECT randomException(num) FROM test").step(), sqlite::exception);
    }

    SECTION("Aggregate function exceptions in step") {
        REQUIRE_NOTHROW(connection.create_aggregate<BadAllocAggregate>("badAlloc"));
        REQUIRE_NOTHROW(connection.create_aggregate<SQLiteExceptionAggregate>("SQLiteException"));
        REQUIRE_NOTHROW(connection.create_aggregate<StdExceptionAggregate>("stdException"));
        REQUIRE_NOTHROW(connection.create_aggregate<RandomExceptionAggregate>("randomException"));

        REQUIRE_THROWS_AS(sqlite::statement(connection, "SELECT badAlloc(num) FROM test").step(), sqlite::exception);
        REQUIRE_THROWS_AS(sqlite::statement(connection, "SELECT SQLiteException(num) FROM test").step(), sqlite::exception);
        REQUIRE_THROWS_AS(sqlite::statement(connection, "SELECT stdException(num) FROM test").step(), sqlite::exception);
        REQUIRE_THROWS_AS(sqlite::statement(connection, "SELECT randomException(num) FROM test").step(), sqlite::exception);
    }

    SECTION("Aggregate function exceptions in finalize") {
        REQUIRE_NOTHROW(connection.create_aggregate<BadAllocAggregateFinalize>("badAlloc"));
        REQUIRE_NOTHROW(connection.create_aggregate<SQLiteExceptionAggregateFinalize>("SQLiteException"));
        REQUIRE_NOTHROW(connection.create_aggregate<StdExceptionAggregateFinalize>("stdException"));
        REQUIRE_NOTHROW(connection.create_aggregate<RandomExceptionAggregateFinalize>("randomException"));

        REQUIRE_THROWS_AS(sqlite::statement(connection, "SELECT badAlloc(num) FROM test").step(), sqlite::exception);
        REQUIRE_THROWS_AS(sqlite::statement(connection, "SELECT SQLiteException(num) FROM test").step(), sqlite::exception);
        REQUIRE_THROWS_AS(sqlite::statement(connection, "SELECT stdException(num) FROM test").step(), sqlite::exception);
        REQUIRE_THROWS_AS(sqlite::statement(connection, "SELECT randomException(num) FROM test").step(), sqlite::exception);
    }
}

static int test_collation(const std::string& s1, const std::string& s2) {
    return -1 * s1.compare(s2);
}

TEST_CASE("Create Collation Function", "[Functions]") {
    sqlite::dbconnection connection = sqlite::dbconnection::memory();

    REQUIRE(sqlite::execute(connection, "CREATE TABLE test (string TEXT)") == 0);
    REQUIRE(sqlite::execute(connection, "INSERT INTO test VALUES (\"a\")") == 1);
    REQUIRE(sqlite::execute(connection, "INSERT INTO test VALUES (\"b\")") == 1);
    REQUIRE(sqlite::execute(connection, "INSERT INTO test VALUES (\"c\")") == 1);
    REQUIRE(sqlite::execute(connection, "INSERT INTO test VALUES (\"d\")") == 1);

    std::vector<std::string> expectedValues = {"d", "c", "b", "a"};

    SECTION("Create collation with function reference") {
        REQUIRE_NOTHROW(
                connection.create_collation(
                "reverse",
                test_collation));

        int i = 0;
        for (auto row : sqlite::statement(connection, "SELECT string FROM test ORDER BY string COLLATE reverse")) {
            REQUIRE(expectedValues[i] == row.get_string(0));
            ++i;
        }
    }

    SECTION("Create collation with lambda") {
        REQUIRE_NOTHROW(
            connection.create_collation(
                "reverse",
                [](const std::string& s1, const std::string& s2) -> int {
                    return -1 * s1.compare(s2);
                }));

        int i = 0;
        for (auto row : sqlite::statement(connection, "SELECT string FROM test ORDER BY string COLLATE reverse")) {
            REQUIRE(expectedValues[i] == row.get_string(0));
            ++i;
        }
    }
}
