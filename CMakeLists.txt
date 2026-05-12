cmake_minimum_required(VERSION 3.16)
project(PROTEI LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(PROTEI
    main.cpp
    APP/app/app.h
    APP/app/app.cpp
    APP/app/networkaddress.h
    APP/app/networkaddress.cpp
    APP/menu/menu.h
    APP/menu/active.h
    APP/menu/active.cpp
    APP/menu/exit.h
    APP/menu/exit.cpp
    APP/menu/protocol.h
    APP/menu/protocol.cpp
    APP/menu/move.h
    APP/menu/move.cpp
    CORE/core/core.h
    CORE/core/core.cpp
)

target_include_directories(PROTEI PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/APP
    ${CMAKE_CURRENT_SOURCE_DIR}/APP/app
    ${CMAKE_CURRENT_SOURCE_DIR}/APP/menu
    ${CMAKE_CURRENT_SOURCE_DIR}/CORE
    ${CMAKE_CURRENT_SOURCE_DIR}/CORE/core
)

include(FetchContent)

FetchContent_Declare(
    json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG v3.11.3
)
FetchContent_MakeAvailable(json)
target_link_libraries(PROTEI PRIVATE nlohmann_json::nlohmann_json)

FetchContent_Declare(
    spdlog
    GIT_REPOSITORY https://github.com/gabime/spdlog
    GIT_TAG v1.13.0
)
FetchContent_MakeAvailable(spdlog)
target_link_libraries(PROTEI PRIVATE spdlog::spdlog)

FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG v1.14.0
)
FetchContent_MakeAvailable(googletest)

configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/DATA/config.json
    ${CMAKE_CURRENT_BINARY_DIR}/config.json
    COPYONLY
)

if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/TESTS)
    add_subdirectory(TESTS)
endif()
