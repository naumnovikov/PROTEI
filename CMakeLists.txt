cmake_minimum_required(VERSION 3.16)
project(PROTEI LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(COMMON_SOURCES
    APP/app/app.cpp
    APP/menu/active.cpp
    APP/menu/exit.cpp
    APP/menu/protocol.cpp
    APP/menu/move.cpp
    CORE/core/server.cpp
    COMMON/jsonparser.cpp
)

include(FetchContent)

FetchContent_Declare(
    json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG v3.11.3
)
FetchContent_MakeAvailable(json)

FetchContent_Declare(
    spdlog
    GIT_REPOSITORY https://github.com/gabime/spdlog
    GIT_TAG v1.13.0
)
FetchContent_MakeAvailable(spdlog)

FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG v1.14.0
)
FetchContent_MakeAvailable(googletest)

function(target_add_include_dirs TARGET)
    target_include_directories(${TARGET} PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/APP
        ${CMAKE_CURRENT_SOURCE_DIR}/APP/app
        ${CMAKE_CURRENT_SOURCE_DIR}/APP/menu
        ${CMAKE_CURRENT_SOURCE_DIR}/CORE
        ${CMAKE_CURRENT_SOURCE_DIR}/CORE/core
        ${CMAKE_CURRENT_SOURCE_DIR}/COMMON
    )
endfunction()

add_executable(app main.cpp ${COMMON_SOURCES})
target_compile_definitions(app PRIVATE APP)
target_link_libraries(app PRIVATE nlohmann_json::nlohmann_json spdlog::spdlog)
target_add_include_dirs(app)

add_executable(server main.cpp ${COMMON_SOURCES})
target_compile_definitions(server PRIVATE SERVER)
target_link_libraries(server PRIVATE nlohmann_json::nlohmann_json spdlog::spdlog)
target_add_include_dirs(server)
set_target_properties(server PROPERTIES EXCLUDE_FROM_ALL TRUE)

configure_file(DATA/config.json ${CMAKE_CURRENT_BINARY_DIR}/config.json COPYONLY)
configure_file(DATA/serverconfig.json ${CMAKE_CURRENT_BINARY_DIR}/serverconfig.json COPYONLY)

if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/TESTS)
    add_subdirectory(TESTS)
endif()