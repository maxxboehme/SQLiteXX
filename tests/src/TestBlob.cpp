#include "catch.hpp"
#include "SQLiteXX.h"


TEST_CASE("Blob Constructors", "[Blob]") {
    sqlite::blob src("Hello World", 11);

    SECTION("L-Value Copy constructor") {
        sqlite::blob copy(src);

        REQUIRE(copy.size() == 11);
        REQUIRE(std::string(static_cast<const char*>(copy.data()), copy.size()) == std::string("Hello World"));
    }

    SECTION("R-Value Copy constructor") {
        sqlite::blob copy(std::move(src));

        REQUIRE(copy.size() == 11);
        REQUIRE(std::string(static_cast<const char*>(copy.data()), copy.size()) == std::string("Hello World"));
    }
}

TEST_CASE("Blob Assigners", "[Blob]") {
    sqlite::blob src("Hello World", 11);

    SECTION("L-Value assignment") {
        sqlite::blob copy("Test original String", 20);

        copy = src;
        REQUIRE(copy.size() == 11);
        REQUIRE(std::string(static_cast<const char*>(copy.data()), copy.size()) == std::string("Hello World"));
    }

    SECTION("R-Value assignment") {
        sqlite::blob copy("Test original String", 20);

        copy = sqlite::blob(src);
        REQUIRE(copy.size() == 11);
        REQUIRE(std::string(static_cast<const char*>(copy.data()), copy.size()) == std::string("Hello World"));
    }
}
