cmake_minimum_required(VERSION 3.16)
project(TimeParserTests CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

add_library(timeparser TimeParser.cpp)
target_include_directories(timeparser PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

enable_testing()
add_executable(TimeParserTest test_cases/TimeParserTest.cpp)
target_link_libraries(TimeParserTest GTest::gtest_main timeparser)

include(GoogleTest)
gtest_discover_tests(TimeParserTest)
