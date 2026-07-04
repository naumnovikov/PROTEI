cmake_minimum_required(VERSION 3.16)
project(PROTEI LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(BUILD_UE "Build UE" OFF)
option(BUILD_SIMTEL "Build SIMTEL" OFF)
option(BUILD_TESTS "Build unit tests" OFF)

# COPYPASTED FROM
# https://cmake.org/cmake/help/latest/module/FetchContent.html#module:FetchContent

include(FetchContent)

FetchContent_Declare(
    json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG v3.12.0
)
FetchContent_MakeAvailable(json)

FetchContent_Declare(
    spdlog
    GIT_REPOSITORY https://github.com/gabime/spdlog
    GIT_TAG v1.17.0
)
FetchContent_MakeAvailable(spdlog)

if (BUILD_TESTS)
    FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG v1.17.0
    )
    FetchContent_MakeAvailable(googletest)
endif()

if (BUILD_SIMTEL)
    find_package(SQLite3 REQUIRED)
endif()

if(BUILD_UE AND EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/UE)
    add_subdirectory(UE)
endif()

if(BUILD_SIMTEL AND EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/SIMTEL)
    add_subdirectory(SIMTEL)
endif()

if(BUILD_TESTS AND EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/TESTS)
    add_subdirectory(TESTS)
endif()
