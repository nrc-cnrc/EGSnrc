// Testing framework
#define CATCH_CONFIG_MAIN
#include "catch.hpp"

TEST_CASE("Test framework functions properly"){
    REQUIRE(2 + 2 == 4);
    REQUIRE(2 + 3 != 4);
}
