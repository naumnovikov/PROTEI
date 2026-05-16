#include <fstream>
#include <utility>

#include <spdlog/spdlog.h>
#include <spdlog/fmt/bundled/ranges.h>

#include "jsonparser.h"
#include "app.h"
#include "server.h"

bool JSONParser::validateJSONOnRequiredFiledsForApp(const json& json_data) noexcept{
    if (!json_data.contains("ip") || !json_data.contains("port") || !json_data.contains("imei") || !json_data.contains("imsi") || !json_data.contains("location") || !json_data.contains("config") || !json_data.contains("nodes")) {
        spdlog::error("configurate: {}", "Missing required fields");
        return false;
    }
    return true;
}

bool JSONParser::validateJSONOnRequiredFiledsForServer(const json& json_data) noexcept{
    if (!json_data.contains("port") || !json_data.contains("position")) {
        spdlog::error("configurate: {}", "Missing required fields");
        return false;
    }
    return true;
}

bool JSONParser::isIPValid(std::string_view ip) noexcept{
    size_t lastDotPos{ip.rfind('.')};
    if (lastDotPos == std::string::npos || lastDotPos + 1 >= ip.size()) {
        spdlog::error("Wrong IP: no valid last byte in '{}'", ip);
        return false;
    }
    std::string last_byte_str{ip.substr(lastDotPos + 1)};
    int last_byte_val;
    try {
        last_byte_val = std::stoi(last_byte_str);
    } catch (...) {
        spdlog::error("Wrong IP: last byte is not a number");
        return false;
    }
    if (last_byte_val < 1 || last_byte_val > 253) {
        spdlog::error("Wrong IP: last byte is out of range(1,253)");
        return false;
    }
    return true;
}

bool JSONParser::isIMEIValid(const std::vector<char>& imei) noexcept{
    if (imei.size() != 15){
        spdlog::error("Wrong IMEI: no valid size");
        return false;
    }
    return true;
}

bool JSONParser::isIMSIValid(const std::vector<char>& imsi) noexcept{
    if (imsi.size() > 15 || imsi.size() < 1){
        spdlog::error("Wrong IMSI: no valid size");
        return false;
    }
    return true;
}

bool JSONParser::isLocationValid(const std::vector<float>& location) noexcept{
    if (location.size() !=3){
        spdlog::error("Wrong location: no valid size");
        return false;
    }
    return true;
}

void JSONParser::setValuesApp(const json& json_data, App& app){
    if (!validateJSONOnRequiredFiledsForApp(json_data)){
        throw std::invalid_argument("Missing required fields");
    }
    std::string ip{json_data["ip"].get<std::string>()};

    if (!isIPValid(ip)){
        throw std::invalid_argument("No valid IP");
    }

    uint16_t port{json_data["port"].get<uint16_t>()};
    if (port < 1024 || port > 49151) {
        throw std::invalid_argument("Wrong PORT according to IANA");
    }

    std::vector<char> imei{json_data["imei"].get<std::vector<char>>()};
    if (!isIMEIValid(imei)){
        throw std::invalid_argument("No valid IMEI in file");
    }
    app.setDeviceIMEI(std::move(imei));

    std::vector<char> imsi{json_data["imsi"].get<std::vector<char>>()};
    if (!isIMSIValid(imsi)){
        throw std::invalid_argument("No valid IMSI in file");
    }
    app.setDeviceIMSI(std::move(imsi));

    std::vector<float> location{json_data["location"].get<std::vector<float>>()};
    if (!isLocationValid(location)){
        throw std::invalid_argument("No valid location in file");
    }
    app.setDeviceLocation(std::move(location));

    app.setDeviceConfig(std::move(json_data["config"].get<std::string>()));
    app.setDeviceNodes(std::move(json_data["nodes"].get<std::string>()));

    app.setDeviceSocket(std::move(NetworkAddress(ip, port)));
}

void JSONParser::setValuesServer(const json& json_data, Server& server){
    if (!validateJSONOnRequiredFiledsForServer(json_data)){
        throw std::invalid_argument("Missing required fields");
    }

    uint16_t port{json_data["port"].get<uint16_t>()};
    if (port < 1024 || port > 49151) {
        throw std::invalid_argument("Wrong PORT according to IANA");
    }
    server.setPort(std::move(port));

    std::vector<float> position{json_data["position"].get<std::vector<float>>()};
    if (!isLocationValid(position)){
        throw std::invalid_argument("No valid position in file");
    }
    server.setPosition(std::move(position));
}

void JSONParser::configurateApp(std::string json_filenameParam, App& app){
    if (json_filenameParam.empty()){
        spdlog::error("configurate: JSON filename is empty");
        throw std::invalid_argument("JSON filename is empty");
    }
    std::ifstream json_file(json_filenameParam);
    if (!json_file.is_open()) [[unlikely]] {
        throw std::invalid_argument("Cannot open file: " + json_filenameParam);
    }

    json json_data;
    try{
        json_data = json::parse(json_file);
    }catch (const nlohmann::detail::parse_error&){
        spdlog::error("configurate: JSON parse error in file {}", json_filenameParam);
        json_file.close();
        throw std::invalid_argument("JSON parse error in file {}" + json_filenameParam);
    }catch (...){
        spdlog::error("configurate: Unknown error with parsing file {}", json_filenameParam);
        json_file.close();
        throw std::invalid_argument("Unknown error with parsing file: " + json_filenameParam);
    }
    json_file.close();

    try{
        setValuesApp(json_data, app);
    }catch(const std::invalid_argument& e){
        throw e;
    }catch(...){
        throw std::invalid_argument("Unknown error with parsing file {}" + json_filenameParam);
    }
    spdlog::info("Device configured: IP={}, port={}", app.getDevice().socket.getIp(), app.getDevice().socket.getPort());
}

void JSONParser::configurateServer(std::string json_filenameParam, Server& server){
    if (json_filenameParam.empty()){
        spdlog::error("configurate: JSON filename is empty");
        throw std::invalid_argument("JSON filename is empty");
    }
    std::ifstream json_file(json_filenameParam);
    if (!json_file.is_open()) [[unlikely]] {
        throw std::invalid_argument("Cannot open file: " + json_filenameParam);
    }

    json json_data;
    try{
        json_data = json::parse(json_file);
    }catch (nlohmann::detail::parse_error){
        spdlog::error("configurate: JSON parse error in file {}", json_filenameParam);
        json_file.close();
        throw std::invalid_argument("JSON parse error in file {}" + json_filenameParam);
    }catch (...){
        spdlog::error("configurate: Unknown error with parsing file {}", json_filenameParam);
        json_file.close();
        throw std::invalid_argument("Unknown error with parsing file {}" + json_filenameParam);
    }
    json_file.close();

    try{
        setValuesServer(json_data, server);
    }catch(const std::invalid_argument& e){
        throw e;
    }catch(...){
        throw std::invalid_argument("Unknown error with parsing file {}" + json_filenameParam);
    }
    spdlog::info("Server configured: port={}, position={}", server.getPort(), server.getPosition());
}