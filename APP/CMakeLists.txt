set(APP_SOURCES
    APP/app/app.cpp
    APP/app/networkaddress.cpp
    APP/menu/active.cpp
    APP/menu/exit.cpp
    APP/menu/move.cpp
    APP/menu/protocol.cpp
    ${CMAKE_SOURCE_DIR}COMMON/jsonparser.cpp
    ${CMAKE_SOURCE_DIR}COMMON/socketWorker.cpp
)

target_include_directories(app PRIVATE
    ${CMAKE_SOURCE_DIR}/APP
    ${CMAKE_SOURCE_DIR}/APP/app
    ${CMAKE_SOURCE_DIR}/APP/menu
    ${CMAKE_SOURCE_DIR}/COMMON
)

add_executable(app main.cpp ${APP_SOURCES})
target_compile_definitions(app PRIVATE BUILD_APP)

target_link_libraries(app PRIVATE
    nlohmann_json::nlohmann_json
    spdlog::spdlog
)

configure_file(DATA/config.json ${CMAKE_CURRENT_BINARY_DIR}/config.json COPYONLY)
configure_file(DATA/config2.json ${CMAKE_CURRENT_BINARY_DIR}/config2.json COPYONLY)
configure_file(DATA/config3.json ${CMAKE_CURRENT_BINARY_DIR}/config3.json COPYONLY)
configure_file(DATA/config4.json ${CMAKE_CURRENT_BINARY_DIR}/config4.json COPYONLY)
configure_file(DATA/config5.json ${CMAKE_CURRENT_BINARY_DIR}/config5.json COPYONLY)
configure_file(DATA/config6.json ${CMAKE_CURRENT_BINARY_DIR}/config6.json COPYONLY)