#include "catch.hpp"
#include "SQLiteXX.h"

#include <cstdio>
#include <iostream>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

static const size_t kNumberOfThreads = 2;

static void table_insert_shared_connection(sqlite::dbconnection connection, std::string text, int count) {
    sqlite::mutex m = connection.mutex();
    std::lock_guard<sqlite::mutex> lock(m);
    {
        sqlite::deferred_transaction transaction(connection);
        for (int i = 0; i < count; ++i) {
            sqlite::execute(connection, "INSERT INTO test VALUES (NULL, ?)", text);
        }
        REQUIRE_NOTHROW(transaction.commit());
    }
}

static void table_insert_shared_connection_try_lock(sqlite::dbconnection connection, std::string text, int count) {
    sqlite::mutex mutex = connection.mutex();
    {
        int i = 0;
        while (i < count) {
            if (mutex.try_lock()) {
                sqlite::execute(connection, "INSERT INTO test VALUES (NULL, ?)", text);
                ++i;
                mutex.unlock();
            }
        }
    }
}

TEST_CASE("Sharing a database connection", "[Threading]") {
    sqlite::dbconnection connection = sqlite::dbconnection::memory();

    sqlite::statement query(connection, "CREATE TABLE test (id INTEGER PRIMARY KEY, value TEXT)");
    REQUIRE_NOTHROW(query.execute());

    int count = 1000;
    size_t numThreads = kNumberOfThreads;
    std::vector<std::thread> threadpool;

    SECTION("Using sqlite::Lock") {
        for (size_t i = 0; i < numThreads; ++i) {
            threadpool.push_back(std::thread(table_insert_shared_connection, connection, "thread" + std::to_string(i), count));
        }
        for (size_t i = 0; i < threadpool.size(); ++i) {
            threadpool[i].join();
        }

        REQUIRE(connection.row_id() == (count * threadpool.size()));
    }

    SECTION("Using Mutex tryLock") {
        for (size_t i = 0; i < numThreads; ++i) {
            threadpool.push_back(std::thread(table_insert_shared_connection_try_lock, connection, "thread" + std::to_string(i), count));
        }
        for (size_t i = 0; i < threadpool.size(); ++i) {
            threadpool[i].join();
        }

        REQUIRE(connection.row_id() == (count * threadpool.size()));
    }
}


void table_insert_default_busy_timeout(std::string filename, std::string text, int count) {
    sqlite::dbconnection connection(filename);
    {
        for (int i = 0; i < count; ++i) {
            REQUIRE_NOTHROW(sqlite::execute(connection, "INSERT INTO test VALUES (NULL, ?)", text));
        }
    }
}

void table_insert_transaction_default_busy_timeout(std::string filename, std::string text, int count) {
    sqlite::dbconnection connection(filename);
    {
        sqlite::deferred_transaction t(connection);
        for (int i = 0; i < count; ++i) {
            REQUIRE_NOTHROW(sqlite::execute(connection, "INSERT INTO test VALUES (NULL, ?)", text));
        }
        t.commit();
    }
}


#define RETRY_BUSY_BEGIN( statement, ... ) \
    bool retry = false; \
    do { \
        retry = false; \
        try { \
            sqlite::execute(connection, statement, __VA_ARGS__); \
        } catch (const sqlite::busy_exception&) { \
            retry = true; \
        } \
    } while (retry)

void table_insert_using_busy_exception2(std::string filename, std::string text, int count) {
    sqlite::dbconnection connection(filename, std::chrono::milliseconds(0));
    for (int i = 0; i < count; ++i) {
        RETRY_BUSY_BEGIN("INSERT INTO test VALUES (NULL, ?)", text);
    }
}

void table_insert_using_busy_exception(std::string filename, std::string text, int count) {
    sqlite::dbconnection connection(filename, std::chrono::milliseconds(0));
    for (int i = 0; i < count; ++i) {
        bool retry = false;
        do {
            retry = false;
            try {
                sqlite::execute(connection, "INSERT INTO test VALUES (NULL, ?)", text);
            } catch (const sqlite::busy_exception&) {
                retry = true;
            }
        } while (retry);
    }
}

void table_insert_deferredtransaction_busy_exception(std::string filename, std::string text, int count) {
    sqlite::dbconnection connection(filename, std::chrono::milliseconds(0));
    bool retry = false;
    do {
        sqlite::deferred_transaction t(connection);
        retry = false;
        try {
            for (int i = 0; i < count; ++i) {
                sqlite::execute(connection, "INSERT INTO test VALUES (NULL, ?)", text);
            }
        } catch (const sqlite::busy_exception&) {
            retry = true;
        }
        if (!retry) {
            do {
                retry = false;
                try {
                    t.commit();
                } catch (const sqlite::busy_exception&) {
                    retry = true;
                }
            } while (retry);
        }
    } while (retry);
}

void table_insert_immediatetransaction_busy_exception(std::string filename, std::string text, int count) {
    sqlite::dbconnection connection(filename, std::chrono::milliseconds(0));
    bool retry = false;
    do {
        retry = false;
        try {
            sqlite::immediate_transaction t(connection);
            for (int i = 0; i < count; ++i) {
                sqlite::execute(connection, "INSERT INTO test VALUES (NULL, ?)", text);
            }
            t.commit();
        } catch (const sqlite::busy_exception&) {
            retry = true;
        }
    } while (retry);
}

void table_insert_exclusivetransaction_busy_exception(std::string filename, std::string text, int count) {
    sqlite::dbconnection connection(filename, std::chrono::milliseconds(0));
    bool retry = false;
    do {
        retry = false;
        try {
            sqlite::immediate_transaction t(connection);
            for (int i = 0; i < count; ++i) {
                sqlite::execute(connection, "INSERT INTO test VALUES (NULL, ?)", text);
            }
            t.commit();
        } catch (const sqlite::busy_exception&) {
            retry = true;
        }
    } while (retry);
}
TEST_CASE("Thread-local connection", "[Threading]") {
    std::string testFile = "test_Threading.db";
    remove(testFile.c_str());

    {
        sqlite::dbconnection connection(testFile.c_str());
        sqlite::statement query(connection, "CREATE TABLE test (id INTEGER PRIMARY KEY, value TEXT)");
        REQUIRE_NOTHROW(query.execute());
    }

    int count = 50;
    size_t numThreads = kNumberOfThreads;
    std::vector<std::thread> threadpool;

    std::set<std::string> expectedStringValues;
    for(size_t i = 0; i < numThreads; ++i) {
        expectedStringValues.insert("thread" + std::to_string(i));
    }

    SECTION("Using default busy timeout") {
        for (size_t i = 0; i < numThreads; ++i) {
            threadpool.push_back(std::thread(table_insert_default_busy_timeout, testFile, "thread" + std::to_string(i), count));
        }

        for (size_t i = 0; i < threadpool.size(); ++i) {
            threadpool[i].join();
        }

        sqlite::dbconnection connection(testFile.c_str());
        unsigned int numRows = 0;
        for (auto row : sqlite::statement(connection, "SELECT * FROM test")) {
            ++numRows;
            REQUIRE(expectedStringValues.find(row.get_string(1)) != expectedStringValues.end());
        }

        REQUIRE(numRows == (count * threadpool.size()));
    }

    SECTION("Using busy Exception") {
        for (size_t i = 0; i < numThreads; ++i) {
            threadpool.push_back(std::thread(table_insert_using_busy_exception, testFile, "thread" + std::to_string(i), count));
        }

        for (size_t i = 0; i < threadpool.size(); ++i) {
            threadpool[i].join();
        }

        sqlite::dbconnection connection(testFile.c_str());
        unsigned int numRows = 0;
        for (auto row : sqlite::statement(connection, "SELECT * FROM test")) {
            ++numRows;
            REQUIRE(expectedStringValues.find(row.get_string(1)) != expectedStringValues.end());
        }

        REQUIRE(numRows == (count * threadpool.size()));
    }

    SECTION("Using transaction with default") {
        for (size_t i = 0; i < numThreads; ++i) {
            threadpool.push_back(std::thread(table_insert_transaction_default_busy_timeout, testFile, "thread" + std::to_string(i), count));
        }

        for (size_t i = 0; i < threadpool.size(); ++i) {
            threadpool[i].join();
        }

        sqlite::dbconnection connection(testFile.c_str());
        unsigned int numRows = 0;
        for (auto row : sqlite::statement(connection, "SELECT * FROM test")) {
            ++numRows;
            REQUIRE(expectedStringValues.find(row.get_string(1)) != expectedStringValues.end());
        }

        REQUIRE(numRows == (count * threadpool.size()));
    }

    SECTION("Using transaction with busy Exception") {
        for (size_t i = 0; i < numThreads; ++i) {
            threadpool.push_back(
                std::thread(table_insert_deferredtransaction_busy_exception, testFile, "thread" + std::to_string(i), count));
        }

        for (size_t i = 0; i < threadpool.size(); ++i) {
            threadpool[i].join();
        }

        sqlite::dbconnection connection(testFile.c_str());
        unsigned int numRows = 0;
        for (auto row : sqlite::statement(connection, "SELECT * FROM test")) {
            ++numRows;
            REQUIRE(expectedStringValues.find(row.get_string(1)) != expectedStringValues.end());
        }

        REQUIRE(numRows == (count * threadpool.size()));
    }
}

TEST_CASE("Thread-local connection different transactions", "[Threading]") {
    std::string testFile = "test_Threading.db";
    remove(testFile.c_str());

    {
        sqlite::dbconnection connection(testFile.c_str());
        sqlite::statement query(connection, "CREATE TABLE test (id INTEGER PRIMARY KEY, value TEXT)");
        REQUIRE_NOTHROW(query.execute());
    }

    int count = 50;
    size_t numThreads = kNumberOfThreads;
    std::vector<std::thread> threadpool;

    SECTION("Using deferred transaction") {
        for (size_t i = 0; i < numThreads; ++i) {
            threadpool.push_back(
                std::thread(table_insert_deferredtransaction_busy_exception, testFile, "thread" + std::to_string(i), count));
        }

        for (size_t i = 0; i < threadpool.size(); ++i) {
            threadpool[i].join();
        }
    }

    SECTION("Using immediate transaction") {
        for (size_t i = 0; i < numThreads; ++i) {
            threadpool.push_back(
                std::thread(table_insert_immediatetransaction_busy_exception, testFile, "thread" + std::to_string(i), count));
        }

        for (size_t i = 0; i < threadpool.size(); ++i) {
            threadpool[i].join();
        }
    }

    SECTION("Using exclusive transaction") {
        for (size_t i = 0; i < numThreads; ++i) {
            threadpool.push_back(
                std::thread(table_insert_exclusivetransaction_busy_exception, testFile, "thread" + std::to_string(i), count));
        }

        for (size_t i = 0; i < threadpool.size(); ++i) {
            threadpool[i].join();
        }
    }
}
