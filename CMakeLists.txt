cmake_minimum_required(VERSION 3.0.0)
project(CS118_Project2)
set(CMAKE_CXX_STANDARD 11)

# Make sure developers do not run cmake in the main project directory, to keep
# build artifacts from becoming clutter
if (${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_BINARY_DIR})
    message(FATAL_ERROR "In-source builds not allowed.
    Please make a new directory (called a build directory) and run CMake from there.
    You may need to remove CMakeCache.txt." )
endif()

# Turn on debug builds if we are building in a devel shell
if (CMAKE_BUILD_TYPE STREQUAL "" AND "$ENV{DEVEL_SHELL}" STREQUAL "1")
    message(STATUS "Setting debug build type by default in devel shell")
    set(CMAKE_BUILD_TYPE Debug)
endif()

# Output binaries to a sub directory "bin"
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Enable cmake testing
include(CTest)
enable_testing()

# Enable GoogleTest
# include(GoogleTest)
# add_subdirectory(/usr/src/googletest/googletest googletest)

# Enable Boost
# Use static libraries so binaries can be deployed without a full boost install

include_directories(include)

# TODO(!): Update name and srcs
add_library(lib_header src/Header.cpp)
add_library(lib_packet src/Packet.cpp)
# add_library(utils include/utils.hpp)

# TODO(!): Update executable name, srcs, and deps
add_executable(server src/server.cpp include/utils.hpp)
add_executable(client src/client.cpp)
target_link_libraries(server lib_packet lib_header)
target_link_libraries(client lib_packet lib_header)

# TODO(!): Update test executable name, srcs, and deps
# add_executable(header_test tests/tcp_header_test.cpp)
# add_executable(packet_test tests/packet_test.cpp)

# target_link_libraries(header_test lib_header)
# target_link_libraries(packet_test lib_packet lib_header)

# TODO(!): Update with test binary
# gtest_discover_tests(NginxConfig_test WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/tests)
