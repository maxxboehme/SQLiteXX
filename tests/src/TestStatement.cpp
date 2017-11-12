#include "catch.hpp"
#include "SQLiteXX.h"

#include <string>
#include <utility>
#include <vector>

TEST_CASE("Query in Memory Database", "[Statement]") {
    sqlite::dbconnection connection = sqlite::dbconnection::memory();

    SECTION("querying table that does not exist") {
        REQUIRE_THROWS_AS(sqlite::execute(connection, "SELECT * FROM test"), sqlite::exception);
    }

    sqlite::statement createTable(connection, "CREATE TABLE test (id INTEGER PRIMARY KEY, value TEXT)");
    REQUIRE_NOTHROW(createTable.execute());

    SECTION("Create a Statement with no parameter") {
        sqlite::statement query(connection, "SELECT * FROM test");
        REQUIRE(query.column_count() == 2);
    }

    SECTION("Try to bind to SELECT statement") {
        sqlite::statement query(connection, "SELECT * FROM test");
        REQUIRE_THROWS_AS(query.bind(-1, 12345), sqlite::exception);
        REQUIRE_THROWS_AS(query.bind(0, 12345), sqlite::exception);
        REQUIRE_THROWS_AS(query.bind(1, 12345), sqlite::exception);
        REQUIRE_THROWS_AS(query.bind(2, 12345), sqlite::exception);
        REQUIRE_THROWS_AS(query.bind(2, "abc"), sqlite::exception);
        REQUIRE_THROWS_AS(query.bind(2, u"abc"), sqlite::exception);
    }

    SECTION("Insert a row") {
        REQUIRE(sqlite::execute(connection, "INSERT INTO test VALUES (NULL, \"one\")") == 1);
        REQUIRE(connection.row_id() == 1);
    }
}

TEST_CASE("Executing Steps", "[Statement]") {
    sqlite::dbconnection connection = sqlite::dbconnection::memory();

    REQUIRE(sqlite::execute(connection, "CREATE TABLE test (id INTEGER PRIMARY KEY, string TEXT, int INTEGER, double REAL)") == 0);

    REQUIRE(sqlite::execute(connection, "INSERT INTO test VALUES (NULL, \"one\", 1234, 0.1234)") == 1);
    REQUIRE(connection.row_id() == 1);

    sqlite::statement query(connection, "SELECT * FROM test");
    REQUIRE(query.column_count() == 4);

    REQUIRE_NOTHROW(query.step());
    REQUIRE(query.get_int64(0) == 1);
    REQUIRE(query.get_string(1) == std::string("one"));
    REQUIRE(query.get_int(2) == 1234);
    REQUIRE(query.get_double(3) == 0.1234);

    // Step one more time to discover there is nothing more
    REQUIRE(query.step() == false);

    // Step after "the end" throw an exception
    REQUIRE(query.step() == false);

    // Try to insert a new row with the same PRIMARY KEY: "UNIQUE constraint failed: test.id"
    sqlite::statement insert(connection, "INSERT INTO test VALUES (1, \"exception\", 456, 0.456)");
    REQUIRE_THROWS_AS(insert.step(), sqlite::exception);
    // reset should throw again
    REQUIRE_THROWS_AS(insert.reset(), sqlite::exception);

    REQUIRE_THROWS_AS(sqlite::execute(connection, "INSERT INTO test VALUES (1, \"exception\", 456, 0.456)"), sqlite::exception);
}

TEST_CASE("Preparing a statement", "[Statement]") {
    sqlite::dbconnection connection = sqlite::dbconnection::memory();

    REQUIRE(sqlite::execute(connection, "CREATE TABLE test (id INTEGER PRIMARY KEY, msg TEXT, int INTEGER, double REAL)") == 0);
    REQUIRE(connection.row_id() == 0);

    SECTION("Explicitly calling prepare") {
        sqlite::statement insert;
        insert.prepare(connection, "INSERT INTO test VALUES (NULL, \"first\", -123, 0.123)");
        REQUIRE(insert.execute() == 1);
    }

    SECTION("Preparing through constructor") {
        sqlite::statement insert(connection, "INSERT INTO test VALUES (NULL, \"first\", -123, 0.123)");
        REQUIRE(insert.execute() == 1);
    }

    SECTION("Preparing through execute") {
        REQUIRE_NOTHROW(
            sqlite::execute(connection, "INSERT INTO test VALUES (NULL, \"first\", -123, 0.123)"));
    }

    sqlite::statement query(connection, "SELECT * FROM test");
    REQUIRE(query.column_count() == 4);
    query.step();

    REQUIRE(query.get_string(1) == "first");
    REQUIRE(query.get_int(2) == -123);
    REQUIRE(query.get_double(3) == 0.123);

    {
        REQUIRE(query.get_type(0) == sqlite::datatype::integer);
        REQUIRE(query.get_type(1) == sqlite::datatype::text);
        REQUIRE(query.get_type(2) == sqlite::datatype::integer);
        REQUIRE(query.get_type(3) == sqlite::datatype::floating);
    }

}

TEST_CASE("Retrieving values from statements", "[Statement]") {
    sqlite::dbconnection connection = sqlite::dbconnection::memory();

    REQUIRE(sqlite::execute(connection, "CREATE TABLE test (id INTEGER PRIMARY KEY, msg TEXT, int INTEGER, double REAL, binary BLOB, empty TEXT)") == 0);
    REQUIRE(connection.row_id() == 0);

    // Create a first row (autoid: 1) with all kind of data and a null value
    sqlite::statement insert(connection, "INSERT INTO test VALUES (NULL, \"first\", -123, 0.123, ?, NULL)");
    // Bind the blob value to the first parameter of the SQL query
    const char  buffer[] = {'b', 'l', '\0', 'b'}; // "bl\0b" : 4 char, with a null byte inside
    const int size = sizeof(buffer);
    const sqlite::blob blob(&buffer, size);
    insert.bind(1, blob);
    REQUIRE(insert.execute() == 1);

    sqlite::statement query(connection, "SELECT * FROM test");
    REQUIRE(query.column_count() == 6);
    query.step();

    {
        REQUIRE(query.get_type(0) == sqlite::datatype::integer);
        REQUIRE(query.get_type(1) == sqlite::datatype::text);
        REQUIRE(query.get_type(2) == sqlite::datatype::integer);
        REQUIRE(query.get_type(3) == sqlite::datatype::floating);
        REQUIRE(query.get_type(4) == sqlite::datatype::blob);
        REQUIRE(query.get_type(5) == sqlite::datatype::null);
    }

    {
        REQUIRE(query.get_int(0) == 1);
        REQUIRE(query.get_int64(0) == 1);
        REQUIRE(query.get_uint(0) == 1u);
        REQUIRE(query.get_double(0) == 1.0);
        REQUIRE(query.get_string(0) == "1");
        REQUIRE(query.get_u16string(0) == u"1");

        REQUIRE(query.get_int(1) == 0);
        REQUIRE(query.get_int64(1) == 0);
        REQUIRE(query.get_uint(1) == 0u);
        REQUIRE(query.get_double(1) == 0.0);
        REQUIRE(query.get_string(1) == "first");
        REQUIRE(query.get_u16string(1) == u"first");

        REQUIRE(query.get_int(2) == -123);
        REQUIRE(query.get_int64(2) == -123);
        // Will returned overflown representation
        REQUIRE(query.get_uint(2) == (std::numeric_limits<unsigned int>::max() - 122));
        REQUIRE(query.get_double(2) == -123.0);
        REQUIRE(query.get_string(2) == "-123");
        REQUIRE(query.get_u16string(2) == u"-123");

        REQUIRE(query.get_int(3) == 0);
        REQUIRE(query.get_int64(3) == 0);
        REQUIRE(query.get_uint(3) == 0u);
        REQUIRE(query.get_double(3) == 0.123);
        REQUIRE(query.get_string(3) == "0.123");
        REQUIRE(query.get_u16string(3) == u"0.123");
        REQUIRE(query.get_u16string(3).size() == 5);

        const sqlite::value fifthColumnValue = query.get_value(4);
        REQUIRE(query.get_int(4) == 0);
        REQUIRE(query.get_int64(4) == 0);
        REQUIRE(query.get_uint(4) == 0u);
        REQUIRE(query.get_double(4) == 0.0);
        REQUIRE(query.get_string(4) == std::string("bl\0b", 4));
        // What the blob object that get_blob returns will depend on
        // if you called get_string or get_u16string as sqlite will
        // convert the data in the back and when calling blob will return
        // that representation of it.
        sqlite::blob sqlBlob = query.get_blob(4);
        REQUIRE(sqlBlob.size() == 4);
        REQUIRE(memcmp("bl\0b", sqlBlob.data(), sqlBlob.size()) == 0);

        REQUIRE(query.get_u16string(4) == std::u16string(u"bl\0b", 4));
        sqlBlob = query.get_blob(4);
        REQUIRE(sqlBlob.size() == 8);
        REQUIRE(memcmp(u"bl\0b", sqlBlob.data(), sqlBlob.size()) == 0);

        REQUIRE(query.get_int(5) == 0);
        REQUIRE(query.get_int64(5) == 0);
        REQUIRE(query.get_uint(5) == 0u);
        REQUIRE(query.get_double(5) == 0.0);
        REQUIRE(query.get_string(5) == "");
        REQUIRE(query.get_u16string(5) == u"");
        const sqlite::blob empty = query.get_blob(5);
        REQUIRE(empty.data() == nullptr);
        REQUIRE(empty.size() == 0);
    }

    {
        REQUIRE(query.get_type(0) == sqlite::datatype::integer);
        REQUIRE(query.get_type(1) == sqlite::datatype::text);
        REQUIRE(query.get_type(2) == sqlite::datatype::integer);
        REQUIRE(query.get_type(3) == sqlite::datatype::floating);
        // Note that the type returned by query for column four has
        // changed from a blob to text because of the conversions.
        // This is different then what a sqlite::value does.
        REQUIRE(query.get_type(4) == sqlite::datatype::text);
        REQUIRE(query.get_type(5) == sqlite::datatype::null);
    }
}

TEST_CASE("Binding to a statement", "[Statement]") {
    sqlite::dbconnection connection = sqlite::dbconnection::memory();

    REQUIRE(sqlite::execute(connection, "CREATE TABLE test (id INTEGER PRIMARY KEY, msg TEXT, int INTEGER, double REAL, binary BLOB, empty TEXT)") == 0);
    REQUIRE(connection.row_id() == 0);

    SECTION("Explicitly calling bind") {
        sqlite::statement insert(connection, "INSERT INTO test VALUES (NULL, ?, ?, ?, ?, NULL)");

        REQUIRE_NOTHROW(insert.bind(1, "first"));
        REQUIRE_NOTHROW(insert.bind(2, -123));
        REQUIRE_NOTHROW(insert.bind(3, 0.123));

        const char buffer[] = {'b', 'l', '\0', 'b'}; // "bl\0b" : 4 char, with a null byte inside
        const int size = sizeof(buffer);
        REQUIRE_NOTHROW(insert.bind(4, sqlite::blob(&buffer, size)));

        REQUIRE(insert.execute() == 1);
    }

    SECTION("Variadic bind") {
        sqlite::statement insert(connection, "INSERT INTO test VALUES (NULL, ?, ?, ?, ?, NULL)");

        const char buffer[] = {'b', 'l', '\0', 'b'}; // "bl\0b" : 4 char, with a null byte inside
        const int size = sizeof(buffer);
        REQUIRE_NOTHROW(insert.bind_all("first", -123, 0.123, sqlite::blob(&buffer, size)));

        REQUIRE(insert.execute() == 1);
    }


    SECTION("Binding through constructor") {
        const char buffer[] = {'b', 'l', '\0', 'b'}; // "bl\0b" : 4 char, with a null byte inside
        const int size = sizeof(buffer);

        sqlite::statement insert(
            connection,
            "INSERT INTO test VALUES (NULL, ?, ?, ?, ?, NULL)",
            "first",
            -123,
            0.123,
            sqlite::blob(&buffer, size));

        REQUIRE(insert.execute() == 1);
    }

    SECTION("Binding through execute") {
        const char buffer[] = {'b', 'l', '\0', 'b'}; // "bl\0b" : 4 char, with a null byte inside
        const int size = sizeof(buffer);

        REQUIRE_NOTHROW(
            sqlite::execute(
                connection,
                "INSERT INTO test VALUES (NULL, ?, ?, ?, ?, NULL)",
                "first",
                -123,
                0.123,
                sqlite::blob(&buffer, size)));
    }

    sqlite::statement query(connection, "SELECT * FROM test");
    REQUIRE(query.column_count() == 6);
    query.step();

    REQUIRE(query.get_string(1) == "first");
    REQUIRE(query.get_int(2) == -123);
    REQUIRE(query.get_double(3) == 0.123);
    const sqlite::blob sqlBlob = query.get_blob(4);
    REQUIRE(sqlBlob.size() == 4);
    REQUIRE(memcmp("bl\0b", sqlBlob.data(), sqlBlob.size()) == 0);

    {
        REQUIRE(query.get_type(0) == sqlite::datatype::integer);
        REQUIRE(query.get_type(1) == sqlite::datatype::text);
        REQUIRE(query.get_type(2) == sqlite::datatype::integer);
        REQUIRE(query.get_type(3) == sqlite::datatype::floating);
        REQUIRE(query.get_type(4) == sqlite::datatype::blob);
        REQUIRE(query.get_type(5) == sqlite::datatype::null);
    }
}

TEST_CASE("Binding using Names", "[Statement]") {
    sqlite::dbconnection connection = sqlite::dbconnection::memory();
    REQUIRE(sqlite::execute(connection, "CREATE TABLE test (id INTEGER PRIMARY KEY, string TEXT, int INTEGER, double REAL)") == 0);

    sqlite::statement insert(connection, "INSERT INTO test VALUES (NULL, @string, @int, @double)");
    // First row with text/int/double
    insert.bind_name("@string", "one");
    insert.bind_name("@int", 1234);
    insert.bind_name("@double", 0.1234);
    REQUIRE(insert.execute() == 1);

    // Compile a SQL query to check the result
    sqlite::statement query(connection, "SELECT * FROM test");
    REQUIRE(query.column_count() == 4);

    // Check the result
    REQUIRE_NOTHROW(query.step());
    REQUIRE(query.get_int64(0) == 1);
    REQUIRE(query.get_string(1) == std::string("one"));
    REQUIRE(query.get_int(2) == 1234);
    REQUIRE(query.get_double(3) == 0.1234);

    insert.reset();
    const std::string str("two");
    const int integer = 1234;
    const double dub = 0.1234;
    insert.bind_name("@string", str);
    insert.bind_name("@int", integer);
    insert.bind_name("@double", dub);
    REQUIRE(insert.execute() == 1);

    // Check the result
    REQUIRE_NOTHROW(query.step());
    REQUIRE(query.get_int64(0) == 2);
    REQUIRE(query.get_string(1) == std::string("two"));
    REQUIRE(query.get_int(2) == 1234);
    REQUIRE(query.get_double(3) == 0.1234);
}

TEST_CASE("Getting column names", "[Statement]") {
    sqlite::dbconnection connection = sqlite::dbconnection::memory();

    REQUIRE(sqlite::execute(connection, "CREATE TABLE test (id INTEGER PRIMARY KEY, string TEXT, int INTEGER, double REAL)") == 0);

    SECTION("get column name from SELECT") {
        sqlite::statement query(connection, "SELECT id, string, int, double FROM test");
        query.step();

        REQUIRE(query.get_column_name(0) == std::string("id"));
        REQUIRE(query.get_column_name(1) == std::string("string"));
        REQUIRE(query.get_column_name(2) == std::string("int"));
        REQUIRE(query.get_column_name(3) == std::string("double"));
    }

    SECTION("get column name after rename \"as\"") {
        sqlite::statement query(connection, "SELECT id, string as value FROM test");
        query.step();

        REQUIRE(query.get_column_name(0) == std::string("id"));
        REQUIRE(query.get_column_name(1) == std::string("value"));
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
    sqlite::dbconnection connection = sqlite::dbconnection::memory();

    REQUIRE(sqlite::execute(connection, "CREATE TABLE test (id INTEGER PRIMARY KEY, string TEXT, double REAL)") == 0);

    REQUIRE(sqlite::execute(connection, "INSERT INTO test VALUES (1, \"one\", 1.0)") == 1);
    REQUIRE(sqlite::execute(connection, "INSERT INTO test VALUES (2, \"two\", 2.0)") == 1);
    REQUIRE(sqlite::execute(connection, "INSERT INTO test VALUES (3, \"three\", 3.0)") == 1);

    std::vector<std::vector<std::pair<std::string, std::string> > > allColumnData;

    sqlite::execute_callback(connection, "SELECT * FROM test", test_callback, std::ref(allColumnData));
    REQUIRE(allColumnData.size() == 3);
    REQUIRE(allColumnData[0].size() == 3);
    REQUIRE(allColumnData[1].size() == 3);
    REQUIRE(allColumnData[2].size() == 3);
}

TEST_CASE("Using callback function with lambda", "[Statement]") {
    sqlite::dbconnection connection = sqlite::dbconnection::memory();

    REQUIRE(sqlite::execute(connection, "CREATE TABLE test (id INTEGER PRIMARY KEY, string TEXT, double REAL)") == 0);

    REQUIRE(sqlite::execute(connection, "INSERT INTO test VALUES (1, \"one\", 1.0)") == 1);
    REQUIRE(sqlite::execute(connection, "INSERT INTO test VALUES (2, \"two\", 2.0)") == 1);
    REQUIRE(sqlite::execute(connection, "INSERT INTO test VALUES (3, \"three\", 3.0)") == 1);

    std::vector<std::vector<std::pair<std::string, std::string> > > allColumnData;

    sqlite::execute_callback(
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

TEST_CASE("Statement to Bool conversion", "[Statement]") {
    sqlite::dbconnection connection = sqlite::dbconnection::memory();

    REQUIRE(sqlite::execute(connection, "CREATE TABLE test (id INTEGER PRIMARY KEY, string TEXT, double REAL)") == 0);

    REQUIRE(sqlite::execute(connection, "INSERT INTO test VALUES (1, \"one\", 1.0)") == 1);
    REQUIRE(sqlite::execute(connection, "INSERT INTO test VALUES (2, \"two\", 2.0)") == 1);
    REQUIRE(sqlite::execute(connection, "INSERT INTO test VALUES (3, \"three\", 3.0)") == 1);

    sqlite::statement query;
    REQUIRE(query == false);

    query.prepare(connection, "SELECT * FROM test");
    REQUIRE(query == true);
}

TEST_CASE("UTF16 Support", "[Statement]") {
    sqlite::dbconnection connection = sqlite::dbconnection::wide_memory();

    REQUIRE(sqlite::execute(connection, "CREATE TABLE test (id INTEGER PRIMARY KEY, string TEXT, double REAL)") == 0);

    REQUIRE(sqlite::execute(connection, "INSERT INTO test VALUES (NULL, ?, ?)", u"first", 1.0) == 1);

    sqlite::statement insert(connection, "INSERT INTO test VALUES (NULL, ?, ?)");
    insert.bind(1, u"second");
    insert.bind(2, 2.0);
    REQUIRE(insert.execute() == 1);

    sqlite::statement query(connection, "SELECT * FROM test");
    query.step();
    REQUIRE(query.get_u16string(1) == u"first");
    REQUIRE(query.get_double(2) == 1.0);

    query.step();
    REQUIRE(query.get_u16string(1) == u"second");
    REQUIRE(query.get_double(2) == 2.0);
}
