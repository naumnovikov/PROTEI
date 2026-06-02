set(SERVER_SOURCES
    ${CMAKE_SOURCE_DIR}/CORE/core/server.cpp
    ${CMAKE_SOURCE_DIR}/COMMON/jsonparser.cpp
    ${CMAKE_SOURCE_DIR}/COMMON/socketBusinessWorker.cpp
    ${CMAKE_SOURCE_DIR}/COMMON/threadpool.cpp
    ${CMAKE_SOURCE_DIR}/COMMON/sock.cpp
)

add_executable(server main_server.cpp ${SERVER_SOURCES})
target_compile_definitions(server PRIVATE BUILD_SERVER)

target_include_directories(server PRIVATE
    ${CMAKE_SOURCE_DIR}/APP
    ${CMAKE_SOURCE_DIR}/CORE
    ${CMAKE_SOURCE_DIR}/CORE/core
    ${CMAKE_SOURCE_DIR}/COMMON
    ${CMAKE_SOURCE_DIR}/APP/app
)

target_link_libraries(server PRIVATE
    nlohmann_json::nlohmann_json
    spdlog::spdlog
)

configure_file(${CMAKE_SOURCE_DIR}/DATA/serverconfig.json ${CMAKE_CURRENT_BINARY_DIR}/serverconfig.json COPYONLY)