set(PRODUCTION_SOURCES
    ${CMAKE_SOURCE_DIR}/APP/app/app.cpp
    ${CMAKE_SOURCE_DIR}/APP/app/networkaddress.cpp
    ${CMAKE_SOURCE_DIR}/APP/menu/active.cpp
    ${CMAKE_SOURCE_DIR}/APP/menu/exit.cpp
    ${CMAKE_SOURCE_DIR}/APP/menu/protocol.cpp
    ${CMAKE_SOURCE_DIR}/APP/menu/move.cpp
    ${CMAKE_SOURCE_DIR}/CORE/core/core.cpp
)

set(TEST_SOURCES
    test_networkaddress.cpp
    test_process.cpp
    test_configurate.cpp
    test_menu.cpp
)

add_executable(protei_tests
    ${TEST_SOURCES}
    ${PRODUCTION_SOURCES}
)

target_include_directories(protei_tests PRIVATE
    ${CMAKE_SOURCE_DIR}/APP
    ${CMAKE_SOURCE_DIR}/APP/app
    ${CMAKE_SOURCE_DIR}/APP/menu
    ${CMAKE_SOURCE_DIR}/CORE
    ${CMAKE_SOURCE_DIR}/CORE/core
)

target_link_libraries(protei_tests PRIVATE
    gtest_main
    nlohmann_json::nlohmann_json
    spdlog::spdlog
)
