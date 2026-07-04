set(UE_SOURCES
    ${CMAKE_SOURCE_DIR}/UE/UE/ue.cpp
    ${CMAKE_SOURCE_DIR}/UE/menu/active.cpp
    ${CMAKE_SOURCE_DIR}/UE/menu/exit.cpp
    ${CMAKE_SOURCE_DIR}/UE/menu/move.cpp
    ${CMAKE_SOURCE_DIR}/UE/menu/sms.cpp
    ${CMAKE_SOURCE_DIR}/COMMON/jsonparser.cpp
    ${CMAKE_SOURCE_DIR}/COMMON/sock.cpp
    ${CMAKE_SOURCE_DIR}/COMMON/socketBusinessWorker.cpp
)

add_executable(UE main_UE.cpp ${UE_SOURCES})
target_compile_definitions(UE PRIVATE BUILD_UE)

target_include_directories(UE PRIVATE
    ${CMAKE_SOURCE_DIR}/UE
    ${CMAKE_SOURCE_DIR}/UE/UE
    ${CMAKE_SOURCE_DIR}/UE/menu
    ${CMAKE_SOURCE_DIR}/COMMON
)

target_link_libraries(UE PRIVATE
    nlohmann_json::nlohmann_json
    spdlog::spdlog
)

configure_file(${CMAKE_SOURCE_DIR}/DATA/config.json ${CMAKE_CURRENT_BINARY_DIR}/config.json COPYONLY)
configure_file(${CMAKE_SOURCE_DIR}/DATA/config2.json ${CMAKE_CURRENT_BINARY_DIR}/config2.json COPYONLY)
configure_file(${CMAKE_SOURCE_DIR}/DATA/config3.json ${CMAKE_CURRENT_BINARY_DIR}/config3.json COPYONLY)
configure_file(${CMAKE_SOURCE_DIR}/DATA/config4.json ${CMAKE_CURRENT_BINARY_DIR}/config4.json COPYONLY)