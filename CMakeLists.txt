cmake_minimum_required(VERSION 3.6)

project(cbigcache)

option(USE_GCOV "Create a GCov-enabled build." OFF)
option(USE_CLANG "Build project with Clang." OFF)

if (USE_GCOV)
    set(GCC_COVERAGE_COMPILE_FLAGS "-fprofile-arcs -ftest-coverage")
    set(GCC_COVERAGE_LINK_FLAGS    "-lgcov")
endif()

if (USE_CLANG)
    set(CMAKE_C_COMPILER   "/usr/bin/clang")
    set(CMAKE_CXX_COMPILER "/usr/bin/clang++")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")
endif()

set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} ${GCC_COVERAGE_COMPILE_FLAGS}" )
set(CMAKE_EXE_LINKER_FLAGS  "${CMAKE_EXE_LINKER_FLAGS} ${GCC_COVERAGE_LINK_FLAGS}" )

include_directories(
    include
    src)

set(PROJECT_LANGUAGES C CXX)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED on)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -lpthread -Wall -Wextra -Werror")

set(SOURCES
    src/app.cpp
    src/debug.cpp
    src/bigcache.cpp
    src/shard.cpp
    src/helpers.cpp
    src/json.cpp
    src/hash.cpp
    src/ts_counter.cpp
    src/export.cpp)

add_executable(app ${SOURCES})
add_library(${PROJECT_NAME} SHARED ${SOURCES} ${HEADERS})
target_link_libraries(app)

enable_testing()
add_subdirectory(tests)
