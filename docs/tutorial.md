# How To Use SQLiteXX
The following are simple examples to help start using SQLiteXX.
The tests are also a good area to explore to find examples of how to use the API.

## Connecting to a Database
To create a connection to an sqlite3 database you can use the dbconnection class.

```c++
int main(int argc, const char *argv[]) {
    // Create a connection that can read and write to a database and
    // will create the file if it does not exist.
    sqlite::dbconnection connection("database.db");


    // Create a connection that is read only.
    sqlite::dbconnection connection("database.db", sqlite::openmode::read_only);
    return 0;
}
```

## Querying a Database
To query a sqlite3 database you can use the Statement class.

```c++
int main(int argc, const char *argv[]) {
    sqlite::dbconnection connection("database.db");

    // Create and prepare a statement and then execute it on the database
    sqlite::statement query(connection, "CREATE TABLE test (id INTEGER PRIMARY KEY, value TEXT)");
    query.execute();

    // You can also use this helper function if applying a query with no expected results
    sqlite::execute(connection, "INSERT INTO test VALUES (NULL, \"temp1\")"));
    sqlite::execute(connection, "INSERT INTO test VALUES (NULL, \"temp2\")"));


    // Create a Statement and iterate over it by stepping
    sqlite::statement query(connection, "SELECT * FROM test");
    while(query.step()) {
        int rowID = query.get_int(0);
        std::string text = query.get_string(1);
    }

    // Or iterate of the returned rows with a ranged for loop
    for(const sqlite::row& row: sqlite::statement("SELECT * FROM test")) {
        int rowID = row.get_int(0);
        std::string text = row.get_string(1);
    }

    return 0;
}
```

## Handling Transactions on a Database
SQLiteXX has classes to help in handling transactions on a database.

```c++
int main(int argc, const char *argv[]) {
    sqlite::dbconnection connection("database.db");

    sqlite::execute(connection, "CREATE TABLE test (id INTEGER PRIMARY KEY, value TEXT)");

    std::vector values = {"temp1", "temp2", "temp3"};

    {
        sqlite::deferred_transaction transaction(connection);

        for(auto text: values) {
            sqlite::execute(connection, "INSERT INTO test VALUES (NULL, ?)", text);
        }

        transaction.commit();
    }

    return 0;
}
```

## Backup a Database

```c++
int main(int argc, const char *argv[]) {
    sqlite::dbconnection src = sqlite::dbconnection::memory();
    sqlite::execute(src, "CREATE TABLE test (id INTEGER PRIMARY KEY, integerValue TEXT, doubleValue REAL)");
    sqlite::execute(src, "INSERT INTO test VALUES (1, \"one\", 1.0)");
    sqlite::execute(src, "INSERT INTO test VALUES (2, \"two\", 2.0)");

    sqlite::dbconnection dest("database.db");

    sqlite::backup backup(src, dest);

    std::cout << "Total Number Of Pages to Backup: " << backup.total_page_count() << std::endl;

    // Backup a specify number of pages at a time.
    // Step returns a boolean value so that it can be done iteratively.
    backup.step(1);

    // Backup all/remaining pages
    backup.step();

    assert(backup.remaining_page_count() == 0);

    return 0;
}
```

## Creating a Function for a Database
```c++
int multiply(int x, int y) {
    return x * y;
}

int main(int argc, const char *argv[]) {
    sqlite::dbconnection connection = sqlite::dbconnection::memory();

    sqlite::execute(connection, "CREATE TABLE test (num INT)");
    sqlite::execute(connection, "INSERT INTO test VALUES (2)");
    sqlite::execute(connection, "INSERT INTO test VALUES (3)");
    sqlite::execute(connection, "INSERT INTO test VALUES (5)");

    // Create a scalar function from a function.
    connection.create_function("multiply", multiply);

    for(auto row : sqlite::statement(connection, "SELECT num, multiply(num, num) FROM test")) {
        int num = row.get_int(0);
        int product = grow.get_int(1);
        std::cout << num << " x " << num << " = " << product << endl;
    }

    // Create a scalar function from a lambda.
    // You can also specify the prefered text encoding of string values
    // and specify if the function is deterministic to improve performance.
    connection.create_function(
        "power",
        [](int x, int y) -> int {
            return x * y;
        },
        true,
        sqlite::textencoding::utf8);

    for(auto row : sqlite::statement(connection, "SELECT num, power(num, num) FROM test")) {
        int num = row.get_int(0);
        int product = grow.get_int(1);
        std::cout << num << "^" << num << " = " << product << endl;
    }
    return 0;
}
```

