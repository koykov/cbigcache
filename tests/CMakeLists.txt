cmake_minimum_required(VERSION 3.6)

project(bigcache-tests)

find_package(GTest REQUIRED)
find_package(Threads REQUIRED)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED on)

include_directories(
    ../include
    ${GTEST_INCLUDE_DIRS}
)

add_executable(
    test_main test_main.cpp
    test_json test_json.cpp
    test_bigcache test_bigcache.cpp
    ../src/json.cpp
    ../src/helpers.cpp
    ../src/bigcache.cpp
    ../src/shard.cpp
    ../src/helpers.cpp
    ../src/json.cpp
    ../src/hash.cpp
    ../src/ts_counter.cpp
    ../src/debug.cpp)

target_link_libraries(
  test_main ${GTEST_LIBRARIES} Threads::Threads)

enable_testing()
add_test(test_json "./test_json")
add_test(test_bigcache "./test_bigcache")
