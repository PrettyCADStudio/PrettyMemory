cmake_minimum_required(VERSION 3.15)
project(PrettyMemory VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(MSVC)
    add_compile_options(/utf-8)
endif()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)

option(BUILD_TESTS "Build tests" ON)
option(BUILD_DOCS "Build API documentation with Doxygen" OFF)

if(BUILD_DOCS)
    find_package(Doxygen REQUIRED)

    set(DOXYGEN_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/doc" CACHE PATH "Output directory for generated API documentation")
    configure_file(
        "${CMAKE_SOURCE_DIR}/Doxyfile.in"
        "${CMAKE_BINARY_DIR}/Doxyfile"
        @ONLY
    )

    add_custom_target(docs
        COMMAND "${DOXYGEN_EXECUTABLE}" "${CMAKE_BINARY_DIR}/Doxyfile"
        WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
        COMMENT "Generating API documentation with Doxygen"
        VERBATIM
    )
endif()

if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(test)
endif()
