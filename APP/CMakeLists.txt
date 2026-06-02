set(APP_SOURCES
    ${CMAKE_SOURCE_DIR}/APP/app/app.cpp
    ${CMAKE_SOURCE_DIR}/APP/app/networkaddress.cpp
    ${CMAKE_SOURCE_DIR}/APP/menu/active.cpp
    ${CMAKE_SOURCE_DIR}/APP/menu/exit.cpp
    ${CMAKE_SOURCE_DIR}/APP/menu/move.cpp
    ${CMAKE_SOURCE_DIR}/APP/menu/protocol.cpp
    ${CMAKE_SOURCE_DIR}/COMMON/jsonparser.cpp
    ${CMAKE_SOURCE_DIR}/COMMON/sock.cpp
    ${CMAKE_SOURCE_DIR}/COMMON/socketBusinessWorker.cpp
)

add_executable(app main_app.cpp ${APP_SOURCES})
target_compile_definitions(app PRIVATE BUILD_APP)

target_include_directories(app PRIVATE
    ${CMAKE_SOURCE_DIR}/APP
    ${CMAKE_SOURCE_DIR}/APP/app
    ${CMAKE_SOURCE_DIR}/APP/menu
    ${CMAKE_SOURCE_DIR}/COMMON
    ${CMAKE_SOURCE_DIR}/CORE
    ${CMAKE_SOURCE_DIR}/CORE/core
)

target_link_libraries(app PRIVATE
    nlohmann_json::nlohmann_json
    spdlog::spdlog
)

configure_file(${CMAKE_SOURCE_DIR}/DATA/config.json ${CMAKE_CURRENT_BINARY_DIR}/config.json COPYONLY)
configure_file(${CMAKE_SOURCE_DIR}/DATA/config2.json ${CMAKE_CURRENT_BINARY_DIR}/config2.json COPYONLY)
configure_file(${CMAKE_SOURCE_DIR}/DATA/config3.json ${CMAKE_CURRENT_BINARY_DIR}/config3.json COPYONLY)
configure_file(${CMAKE_SOURCE_DIR}/DATA/config4.json ${CMAKE_CURRENT_BINARY_DIR}/config4.json COPYONLY)
configure_file(${CMAKE_SOURCE_DIR}/DATA/config5.json ${CMAKE_CURRENT_BINARY_DIR}/config5.json COPYONLY)
configure_file(${CMAKE_SOURCE_DIR}/DATA/config6.json ${CMAKE_CURRENT_BINARY_DIR}/config6.json COPYONLY)