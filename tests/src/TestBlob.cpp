#include "catch.hpp"
#include "SQLiteXX.h"

TEST_CASE("Blob Constructors", "[Blob]") {
    SQLite::Blob src("Hello World", 11);

    SECTION("L-Value Copy constructor") {
        SQLite::Blob copy(src);

        REQUIRE(copy.size() == 11);
        REQUIRE(std::string(static_cast<const char*>(copy.data()), copy.size()) == std::string("Hello World"));
    }

    SECTION("R-Value Copy constructor") {
        SQLite::Blob copy = SQLite::Blob(src);

        REQUIRE(copy.size() == 11);
        REQUIRE(std::string(static_cast<const char*>(copy.data()), copy.size()) == std::string("Hello World"));
    }
}

TEST_CASE("Blob Assigners", "[Blob]") {
    SQLite::Blob src("Hello World", 11);

    SECTION("L-Value assignment") {
        SQLite::Blob copy("Test original String", 20);

        copy = src;
        REQUIRE(copy.size() == 11);
        REQUIRE(std::string(static_cast<const char*>(copy.data()), copy.size()) == std::string("Hello World"));
    }

    SECTION("R-Value assignment") {
        SQLite::Blob copy("Test original String", 20);

        copy = SQLite::Blob(src);
        REQUIRE(copy.size() == 11);
        REQUIRE(std::string(static_cast<const char*>(copy.data()), copy.size()) == std::string("Hello World"));
    }
}
