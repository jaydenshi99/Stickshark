cmake_minimum_required(VERSION 3.15)
project(Stickshark VERSION 2.0.4 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)  # Generate compile_commands.json for IDE

# Get current timestamp
string(TIMESTAMP CMAKE_BUILD_DATE "%Y-%m-%d %H:%M:%S UTC" UTC)

# Generate version header
configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/src/version.h.in"
    "${CMAKE_CURRENT_BINARY_DIR}/version.h"
    @ONLY
)

# Generate VERSION.txt file in build directory
file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/VERSION.txt"
    "Stickshark Chess Engine\nVersion ${PROJECT_VERSION}\nBuild Date: ${CMAKE_BUILD_DATE}\n")

if(MSVC)
    add_compile_options(/W4)
else()
    add_compile_options(-Wall -Wextra -Wpedantic -O2)
endif()

file(GLOB_RECURSE STICKSHARK_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
)

# Main executable
add_executable(stickshark ${STICKSHARK_SOURCES})
target_include_directories(stickshark PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src")
target_include_directories(stickshark PRIVATE "${CMAKE_CURRENT_BINARY_DIR}")

# Test executables (optional - build manually if needed)
file(GLOB_RECURSE TEST_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/tests/*.cpp"
)

if(TEST_SOURCES)
    # Get source files without main.cpp for tests
    set(TEST_LIB_SOURCES ${STICKSHARK_SOURCES})
    list(FILTER TEST_LIB_SOURCES EXCLUDE REGEX ".*main\\.cpp$")
    
    # Create separate test executables for each test file
    foreach(TEST_FILE ${TEST_SOURCES})
        get_filename_component(TEST_NAME ${TEST_FILE} NAME_WE)
        add_executable(test_${TEST_NAME} ${TEST_FILE} ${TEST_LIB_SOURCES})
        target_include_directories(test_${TEST_NAME} PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src")
        target_include_directories(test_${TEST_NAME} PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/tests")
        target_include_directories(test_${TEST_NAME} PRIVATE "${CMAKE_CURRENT_BINARY_DIR}")
    endforeach()
endif()

