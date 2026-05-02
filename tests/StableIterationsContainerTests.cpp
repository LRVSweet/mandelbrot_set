#include <catch2/catch_test_macros.hpp>
#include <stdexcept>

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

        SECTION("given an out-of-range row") {

            SECTION("throws std::out_of_range") {
                StableIterationsContainer container(10u, 5u);
                REQUIRE_THROWS_AS(container.at(5u, 0u), std::out_of_range);
            }
        }

        SECTION("given an out-of-range column") {

            SECTION("throws std::out_of_range") {
                StableIterationsContainer container(10u, 5u);
                REQUIRE_THROWS_AS(container.at(0u, 10u), std::out_of_range);
            }
        }
    }
}
