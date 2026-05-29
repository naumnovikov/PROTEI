set(PRODUCTION_SOURCES
    ${CMAKE_SOURCE_DIR}/APP/app/app.cpp
    ${CMAKE_SOURCE_DIR}/APP/app/networkaddress.cpp
    ${CMAKE_SOURCE_DIR}/APP/menu/active.cpp
    ${CMAKE_SOURCE_DIR}/APP/menu/exit.cpp
    ${CMAKE_SOURCE_DIR}/APP/menu/protocol.cpp
    ${CMAKE_SOURCE_DIR}/APP/menu/move.cpp
    ${CMAKE_SOURCE_DIR}/CORE/core/server.cpp       
    ${CMAKE_SOURCE_DIR}/COMMON/jsonparser.cpp         
    ${CMAKE_SOURCE_DIR}/COMMON/socketWorker.cpp  
    ${CMAKE_SOURCE_DIR}/COMMON/threadpool.cpp
)

set(TEST_SOURCES
    test_networkaddress.cpp
    test_socketworker.cpp
    test_configurate.cpp
    test_menu.cpp
)

add_executable(tests
    ${TEST_SOURCES}
    ${PRODUCTION_SOURCES}
)
target_compile_definitions(app PRIVATE BUILD_TESTS)

target_include_directories(tests PRIVATE
    ${CMAKE_SOURCE_DIR}/APP
    ${CMAKE_SOURCE_DIR}/APP/app
    ${CMAKE_SOURCE_DIR}/APP/menu
    ${CMAKE_SOURCE_DIR}/CORE
    ${CMAKE_SOURCE_DIR}/CORE/core
    ${CMAKE_SOURCE_DIR}/COMMON
)

target_link_libraries(tests PRIVATE
    gtest_main
    nlohmann_json::nlohmann_json
    spdlog::spdlog
)
