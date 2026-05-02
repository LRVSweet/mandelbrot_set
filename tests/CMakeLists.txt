add_executable(mandelbrot_tests smoke_test.cpp)

target_include_directories(mandelbrot_tests PRIVATE ${PROJECT_SOURCE_DIR}/include)

target_link_libraries(mandelbrot_tests PRIVATE
    Catch2::Catch2WithMain
    trompeloeil::trompeloeil
)

include(CTest)
include(Catch)
catch_discover_tests(mandelbrot_tests)
