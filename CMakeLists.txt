cmake_minimum_required(VERSION 3.15)
project(Stickshark LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(MSVC)
    add_compile_options(/W4)
else()
    add_compile_options(-Wall -Wextra -Wpedantic -O2)
endif()

file(GLOB_RECURSE STICKSHARK_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
)

add_executable(stickshark ${STICKSHARK_SOURCES})
target_include_directories(stickshark PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src")

