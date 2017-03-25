# How To Use SQLiteXX

## Connecting to a Database
To create a connection to an sqlite3 database you can use the DBConnection class.

```c++
int main(int argc, const char *argv[]) {
    // Create a connection that can read and write to a database and
    // will create the file if it does not exist.
    SQLite::DBConnection connection("database.db");


    // Create a connection that is read only.
    SQLite::DBConnection connection("database.db", SQLite::OpenMode::ReadOnly);
    return 0;
}
```

## Querying a Database
To query a sqlite3 database you can use the Statement class.

```c++
int main(int argc, const char *argv[]) {
    SQLite::DBConnection connection("database.db");

    // Create and prepare a statement and then execute it on the database
    SQLite::Statement query(connection, "CREATE TABLE test (id INTEGER PRIMARY KEY, value TEXT)");
    query.execute();

    // You can also use this helper function if applying a query with no expected results
    SQLite::Execute(connection, "INSERT INTO test VALUES (NULL, \"temp1\")"));
    SQLite::Execute(connection, "INSERT INTO test VALUES (NULL, \"temp2\")"));


    // Create a Statement and iterate over it by stepping
    SQLite::Statement query(connection, "SELECT * FROM test");
    while(query.step()) {
        int rowID = query.getInt(0);
        std::string text = query.getString(1);
    }

    // Or iterate of the returned rows with a ranged for loop
    for(const SQLite::Row &row: SQLite::Statement("SELECT * FROM test")) {
        int rowID = row.getInt(0);
        std::string text = row.getString(1);
    }

    return 0;
}

## Handling Transactions on a Database
SQLiteXX has classes to help in handling transactions on a database.

```c++
int main(int argc, const char *argv[]) {
    SQLite::DBConnection connection("database.db");

    SQLite::Execute(connection, "CREATE TABLE test (id INTEGER PRIMARY KEY, value TEXT)");

    std::vector stringValues = {"temp1", "temp2", "temp3"};

    try {
        SQLite::DeferredTransaction transaction(connection);

        for(auto text: stringValues) {
            SQLite::Execute(connection, "INSERT INTO test VALUES (NULL, ?)", text);
        }

        transaction.commit();
    } catch (SQLite::Exception &e) {
        std::cout << "Exception: " << e.what() << std::endl;
    }

    return 0;
}

## Backup a Database

```c++
int main(int argc, const char *argv[]) {
    SQLite::DBConnection src = SQLite::DBConnection::memory();
    SQLite::Execute(src, "CREATE TABLE test (id INTEGER PRIMARY KEY, integerValue TEXT, doubleValue REAL)");
    SQLite::Execute(src, "INSERT INTO test VALUES (1, \"one\", 1.0)");
    SQLite::Execute(src, "INSERT INTO test VALUES (2, \"two\", 2.0)");

    SQLite::DBConnection dest("database.db");

    SQLite::Backup backup(src, dest);

    std::cout << "Total Number Of Pages to Backup: " << backup.getTotalPageCount() << std::endl;

    // Backup a specify number of pages at a time.
    // Step returns a boolean value so that it can be done iteratively.
    backup.step(1);

    // Backup all/remaining pages
    backup.step();

    assert(backup.getRemainingPageCount() == 0);

    return 0;
}

## Creating a Function for a Database
```c++
int multiply(const std::vector<SQLite::Value> &values) {
    int product = 1;
    for (size_t i = 0; i < values.size(); ++i) {
        product *= values[i].getInt();
    }

    return product;
}

int main(int argc, const char *argv[]) {
    SQLite::DBConnection connection = SQLite::DBConnection::memory();

    SQLite::Execute(connection, "CREATE TABLE test (num INT)");
    SQLite::Execute(connection, "INSERT INTO test VALUES (2)");
    SQLite::Execute(connection, "INSERT INTO test VALUES (3)");
    SQLite::Execute(connection, "INSERT INTO test VALUES (5)");

    // Create a scalar function from a function.
    SQLite::CreateScalarFunction(connection, "multiply", multiply);

    for(auto row : SQLite::Statement(connection, "SELECT num, multiply(num, num) FROM test")) {
        int num = row.getInt(0);
        int product = grow.getInt(1);
        std::cout << num << " x " << num << " = " << product << endl;
    }

    // Create a scalar function from a lambda.
    // You can also specify the number of expected arguments and the 
    // prefered text encoding of string values and specify if the function is deterministic
    SQLite::CreateScalarFunction(
        connection,
        "power",
        [](const std::vector<SQLite::Value> &values) -> int {
            int product = 1;
            for (size_t i = 0; i < values.size(); ++i) {
                product *= values[i].getInt();
            }
            return product;
        },
        SQLite::TextEncoding::UTF8 | SQLite::FunctionType::Deterministic,
        2
    );

    for(auto row : SQLite::Statement(connection, "SELECT num, power(num, num) FROM test")) {
        int num = row.getInt(0);
        int product = grow.getInt(1);
        std::cout << num << "^" << num << " = " << product << endl;
    }
    return 0;
}
