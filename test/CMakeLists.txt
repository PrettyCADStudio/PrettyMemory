cmake_minimum_required(VERSION 3.15)
project(PrettyMemoryTests LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(PrettyMemory INTERFACE)
target_include_directories(PrettyMemory INTERFACE ${CMAKE_SOURCE_DIR}/include)

include(FetchContent)
FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG v1.14.0
)
FetchContent_MakeAvailable(googletest)

file(
    GLOB_RECURSE PRETTY_MEMORY_TEST_SOURCES
    CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.h"
)

add_executable(PrettyMemoryTests ${PRETTY_MEMORY_TEST_SOURCES})
target_link_libraries(PrettyMemoryTests PRIVATE PrettyMemory GTest::gtest_main)

include(GoogleTest)
gtest_discover_tests(PrettyMemoryTests)

set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT PrettyMemoryTests)
