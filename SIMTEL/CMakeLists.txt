set(SIMTEL_SOURCES
    ${CMAKE_SOURCE_DIR}/SIMTEL/simtel.cpp
    ${CMAKE_SOURCE_DIR}/SIMTEL/BS/basestation.cpp
    ${CMAKE_SOURCE_DIR}/SIMTEL/MME/mme.cpp
    ${CMAKE_SOURCE_DIR}/SIMTEL/REGISTERS/EIRhandler.cpp
    ${CMAKE_SOURCE_DIR}/SIMTEL/REGISTERS/HLRhandler.cpp
    ${CMAKE_SOURCE_DIR}/SIMTEL/REGISTERS/registershandler.cpp
    ${CMAKE_SOURCE_DIR}/SIMTEL/REGISTERS/VLRhandler.cpp
    ${CMAKE_SOURCE_DIR}/COMMON/jsonparser.cpp
    ${CMAKE_SOURCE_DIR}/COMMON/socketBusinessWorker.cpp
    ${CMAKE_SOURCE_DIR}/COMMON/threadpool.cpp
    ${CMAKE_SOURCE_DIR}/COMMON/sock.cpp
    ${CMAKE_SOURCE_DIR}/COMMON/networkaddress.cpp
    ${CMAKE_SOURCE_DIR}/SIMTEL/SMSC/smsc.cpp
    ${CMAKE_SOURCE_DIR}/SIMTEL/SMSC/smsc_db.cpp
)

add_executable(SIMTEL main_SIMTEL.cpp ${SIMTEL_SOURCES})
target_compile_definitions(SIMTEL PRIVATE BUILD_SIMTEL)

target_include_directories(SIMTEL PRIVATE
    ${CMAKE_SOURCE_DIR}/UE
    ${CMAKE_SOURCE_DIR}/SIMTEL
    ${CMAKE_SOURCE_DIR}/SIMTEL/BS
    ${CMAKE_SOURCE_DIR}/SIMTEL/MME
    ${CMAKE_SOURCE_DIR}/SIMTEL/REGISTERS
    ${CMAKE_SOURCE_DIR}/SIMTEL/SMSC
    ${CMAKE_SOURCE_DIR}/SIMTEL/SIMTEL
    ${CMAKE_SOURCE_DIR}/COMMON
    ${CMAKE_SOURCE_DIR}/UE/UE
)

target_link_libraries(SIMTEL PRIVATE
    nlohmann_json::nlohmann_json
    spdlog::spdlog
    SQLite::SQLite3
)

configure_file(${CMAKE_SOURCE_DIR}/DATA/SIMTELconfig.json ${CMAKE_CURRENT_BINARY_DIR}/SIMTELconfig.json COPYONLY)