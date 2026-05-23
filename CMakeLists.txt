cmake_minimum_required(VERSION 3.16)
project(PROTEI LANGUAGES CXX)
option(BUILD_TESTS "Build unit tests" OFF)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(APP_SOURCES
    APP/app/app.cpp
    APP/app/networkaddress.cpp
    APP/menu/active.cpp
    APP/menu/exit.cpp
    APP/menu/move.cpp
    APP/menu/protocol.cpp
    COMMON/jsonparser.cpp
    COMMON/socketWorker.cpp
)

set(SERVER_SOURCES
    CORE/core/server.cpp
    COMMON/jsonparser.cpp
    COMMON/socketWorker.cpp
    COMMON/threadpool.cpp
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

if (BUILD_TESTS)
    FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG v1.14.0
    )
    FetchContent_MakeAvailable(googletest)
endif()

# for headers search 
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

add_executable(app main.cpp ${APP_SOURCES})
target_compile_definitions(app PRIVATE APP)
target_link_libraries(app PRIVATE nlohmann_json::nlohmann_json spdlog::spdlog)
target_add_include_dirs(app)

add_executable(server main.cpp ${SERVER_SOURCES})
target_compile_definitions(server PRIVATE SERVER)
target_link_libraries(server PRIVATE nlohmann_json::nlohmann_json spdlog::spdlog)
target_add_include_dirs(server)

set_target_properties(server PROPERTIES EXCLUDE_FROM_ALL TRUE)

configure_file(DATA/config.json ${CMAKE_CURRENT_BINARY_DIR}/config.json COPYONLY)
configure_file(DATA/config2.json ${CMAKE_CURRENT_BINARY_DIR}/config2.json COPYONLY)
configure_file(DATA/config3.json ${CMAKE_CURRENT_BINARY_DIR}/config3.json COPYONLY)
configure_file(DATA/config4.json ${CMAKE_CURRENT_BINARY_DIR}/config4.json COPYONLY)
configure_file(DATA/config5.json ${CMAKE_CURRENT_BINARY_DIR}/config5.json COPYONLY)
configure_file(DATA/config6.json ${CMAKE_CURRENT_BINARY_DIR}/config6.json COPYONLY)
configure_file(DATA/serverconfig.json ${CMAKE_CURRENT_BINARY_DIR}/serverconfig.json COPYONLY)

if(BUILD_TESTS AND EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/TESTS)
    add_subdirectory(TESTS)
endif()
