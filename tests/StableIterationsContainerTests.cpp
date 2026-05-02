#include <catch2/catch_test_macros.hpp>

#include "StableIterationsContainer.hpp"

TEST_CASE("StableIterationsContainer", "[StableIterationsContainer]") {

    SECTION("getWidth()") {

        SECTION("returns the width passed at construction") {
            StableIterationsContainer container(10u, 5u);
            CHECK(container.getWidth() == 10u);
        }
    }

    SECTION("getHeight()") {

        SECTION("returns the height passed at construction") {
            StableIterationsContainer container(10u, 5u);
            CHECK(container.getHeight() == 5u);
        }
    }

    SECTION("at()") {

        SECTION("given a valid index") {

            SECTION("stores and returns the value at the specified location") {
                StableIterationsContainer container(10u, 5u);
                container.at(1u, 0u) = 42u;
                const StableIterationsContainer& cref = container;
                CHECK(cref.at(1u, 0u) == 42u);
            }
        }
    }
}
