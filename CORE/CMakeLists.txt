set(SERVER_SOURCES
    CORE/core/server.cpp
    ${CMAKE_SOURCE_DIR}COMMON/jsonparser.cpp
    ${CMAKE_SOURCE_DIR}COMMON/socketWorker.cpp
    ${CMAKE_SOURCE_DIR}COMMON/threadpool.cpp
)

target_include_directories(server PRIVATE
    ${CMAKE_SOURCE_DIR}/CORE/core
    ${CMAKE_SOURCE_DIR}/COMMON
)

add_executable(server main.cpp ${SERVER_SOURCES})
target_compile_definitions(server PRIVATE BUILD_SERVER)

target_link_libraries(server PRIVATE
    nlohmann_json::nlohmann_json
    spdlog::spdlog
)

configure_file(DATA/serverconfig.json ${CMAKE_CURRENT_BINARY_DIR}/serverconfig.json COPYONLY)