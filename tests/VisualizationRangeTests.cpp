#include <catch2/catch_test_macros.hpp>
#include <complex>

#include "VisualizationRange.hpp"

TEST_CASE("VisualizationRange", "[VisualizationRange]") {

    SECTION("horizontalPoints") {

        SECTION("stores the value passed at construction") {
            VisualizationRange range{1920u, 1080u, {-2.5, 0.984}, {1.0, -0.984}};
            CHECK(range.horizontalPoints == 1920u);
        }
    }

    SECTION("verticalPoints") {

        SECTION("stores the value passed at construction") {
            VisualizationRange range{1920u, 1080u, {-2.5, 0.984}, {1.0, -0.984}};
            CHECK(range.verticalPoints == 1080u);
        }
    }

    SECTION("topLeft") {

        SECTION("stores the value passed at construction") {
            VisualizationRange range{1920u, 1080u, {-2.5, 0.984}, {1.0, -0.984}};
            CHECK(range.topLeft == std::complex<double>(-2.5, 0.984));
        }
    }

    SECTION("bottomRight") {

        SECTION("stores the value passed at construction") {
            VisualizationRange range{1920u, 1080u, {-2.5, 0.984}, {1.0, -0.984}};
            CHECK(range.bottomRight == std::complex<double>(1.0, -0.984));
        }
    }
}
