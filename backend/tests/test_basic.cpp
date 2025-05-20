#define CATCH_CONFIG_MAIN
#include <catch2/catch_all.hpp>

TEST_CASE("Basic arithmetic works", "[math]") {
    REQUIRE(1 + 1 == 2);
    REQUIRE(2 * 2 == 4);
}

TEST_CASE("Strings are equal", "[string]") {
    std::string hello = "Hello";
    REQUIRE(hello == "Hello");
}
