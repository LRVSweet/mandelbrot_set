#include <catch2/catch_test_macros.hpp>
#include <trompeloeil.hpp>

TEST_CASE("build infrastructure", "[infrastructure]") {
    SECTION("Catch2 and Trompeloeil link correctly") {
        CHECK(true);
    }
}
