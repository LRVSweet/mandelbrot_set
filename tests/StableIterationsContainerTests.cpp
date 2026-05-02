#include <catch2/catch_test_macros.hpp>

#include "StableIterationsContainer.hpp"

TEST_CASE("StableIterationsContainer", "[StableIterationsContainer]") {

    SECTION("getWidth()") {

        SECTION("returns the width passed at construction") {
            StableIterationsContainer container(10u, 5u);
            CHECK(container.getWidth() == 10u);
        }
    }
}
