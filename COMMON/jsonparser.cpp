#include "jsonparser.h"

#include <spdlog/fmt/bundled/ranges.h>
#include <spdlog/spdlog.h>

#include <exception>
#include <fstream>
#include <utility>

#include "app.h"
#include "server.h"

constexpr int MIN_LAST_IP_BYTE{1};
constexpr int MAX_LAST_IP_BYTE{253};
constexpr int IMEI_SIZE{15};
constexpr int MAX_IMSI_SIZE{15};
constexpr int MIN_IMSI_SIZE{1};
constexpr int COORDINATES_QUANTITY{3};
constexpr int IANA_REGISTRED_PORTS_MAX{49151};
constexpr int IANA_REGISTRED_PORTS_MIN{1024};

bool JSONParser::validateJSONOnRequiredFiledsForApp(
    const json& json_data) const noexcept {
  if (!json_data.contains("ip") || !json_data.contains("imei") ||
      !json_data.contains("imsi") || !json_data.contains("location") ||
      !json_data.contains("server_ip") || !json_data.contains("server_port"))
      [[unlikely]] {
    SPDLOG_ERROR("configurate: Missing required fields");
    return false;
  }
  return true;
}

bool JSONParser::validateJSONOnRequiredFiledsForServer(
    const json& json_data) const noexcept {
  if (!json_data.contains("ip") || !json_data.contains("port") ||
      !json_data.contains("position")) [[unlikely]] {
    SPDLOG_ERROR("configurate: {}", "Missing required fields");
    return false;
  }
  return true;
}

bool JSONParser::isIPValid(std::string_view ip) const noexcept {
  std::size_t lastDotPos{ip.rfind('.')};
  if (lastDotPos == std::string::npos || lastDotPos + 1 >= ip.size())
      [[unlikely]] {
    SPDLOG_ERROR("Wrong IP: no valid last byte in '{}'", ip);
    return false;
  }
  std::string last_byte_str{ip.substr(lastDotPos + 1)};
  int last_byte_val;
  try {
    last_byte_val = std::stoi(last_byte_str);
  } catch (...) {
    SPDLOG_ERROR("Wrong IP: last byte is not a number");
    return false;
  }
  if (last_byte_val < MIN_LAST_IP_BYTE || last_byte_val > MAX_LAST_IP_BYTE)
      [[unlikely]] {
    SPDLOG_ERROR("Wrong IP: last byte is out of range(1,253)");
    return false;
  }
  return true;
}

bool JSONParser::isIMEIValid(std::string_view imei) const noexcept {
  if (imei.size() != IMEI_SIZE) [[unlikely]] {
    SPDLOG_ERROR("Wrong IMEI: no valid size");
    return false;
  }
  return true;
}

bool JSONParser::isIMSIValid(std::string_view imsi) const noexcept {
  if (imsi.size() > MAX_IMSI_SIZE || imsi.size() < MIN_IMSI_SIZE) [[unlikely]] {
    SPDLOG_ERROR("Wrong IMSI: no valid size");
    return false;
  }
  return true;
}

bool JSONParser::isLocationValid(
    const std::vector<float>& location) const noexcept {
  if (location.size() != COORDINATES_QUANTITY) [[unlikely]] {
    SPDLOG_ERROR("Wrong location: no valid size");
    return false;
  }
  return true;
}

void JSONParser::setValuesApp(const json& json_data, App& app) const {
  if (!validateJSONOnRequiredFiledsForApp(json_data)) [[unlikely]] {
    throw std::invalid_argument("Missing required fields");
  }
  std::string ip{json_data["ip"].get<std::string>()};

  if (!isIPValid(ip)) [[unlikely]] {
    throw std::invalid_argument("No valid IP");
  }

  std::string imei{json_data["imei"].get<std::string>()};
  if (!isIMEIValid(imei)) [[unlikely]] {
    throw std::invalid_argument("No valid IMEI in file");
  }
  app.setDeviceIMEI(std::move(imei));

  std::string imsi{json_data["imsi"].get<std::string>()};
  if (!isIMSIValid(imsi)) [[unlikely]] {
    throw std::invalid_argument("No valid IMSI in file");
  }
  app.setDeviceIMSI(std::move(imsi));

  std::vector<float> location{json_data["location"].get<std::vector<float>>()};
  if (!isLocationValid(location)) [[unlikely]] {
    throw std::invalid_argument("No valid location in file");
  }
  app.setDeviceLocation(std::move(location));

  std::string server_ip{json_data["server_ip"].get<std::string>()};
  if (!isIPValid(server_ip)) [[unlikely]] {
    throw std::invalid_argument("No valid server_ip");
  }

  uint16_t server_port{json_data["server_port"].get<uint16_t>()};
  if (server_port < IANA_REGISTRED_PORTS_MIN ||
      server_port > IANA_REGISTRED_PORTS_MAX) {
    throw std::invalid_argument("Wrong PORT according to IANA");
  }

  if (!json_data.contains("config")) {
    SPDLOG_INFO(
        "Missing not required field config {}. Replaced with default value: {}",
        "config", "config_default");
    app.setDeviceConfig("config_default");
  } else {
    app.setDeviceConfig(std::move(json_data["config"].get<std::string>()));
  }

  if (!json_data.contains("nodes")) {
    SPDLOG_INFO(
        "Missing not required field nodes {}. Replaced with default value: {}",
        "nodes", "nodes_default");
    app.setDeviceNodes("nodes_default");
  } else {
    app.setDeviceNodes(std::move(json_data["nodes"].get<std::string>()));
  }
  app.setDeviceServerAddress(NetworkAddress(server_ip, server_port));
}

void JSONParser::setValuesServer(const json& json_data, Server& server) const {
  if (!validateJSONOnRequiredFiledsForServer(json_data)) [[unlikely]] {
    throw std::invalid_argument("Missing required fields");
  }

  std::string ip{json_data["ip"].get<std::string>()};

  if (!isIPValid(ip)) [[unlikely]] {
    throw std::invalid_argument("No valid IP");
  }
  server.setIp(std::move(ip));

  uint16_t port{json_data["port"].get<uint16_t>()};
  if (port < IANA_REGISTRED_PORTS_MIN || port > IANA_REGISTRED_PORTS_MAX) {
    throw std::invalid_argument("Wrong PORT according to IANA");
  }
  server.setPort(std::move(port));

  std::vector<float> position{json_data["position"].get<std::vector<float>>()};
  if (!isLocationValid(position)) [[unlikely]] {
    throw std::invalid_argument("No valid position in file");
  }
  server.setPosition(std::move(position));
}

void JSONParser::configurateApp(std::string json_filenameParam,
                                App& app) const {
  if (json_filenameParam.empty()) [[unlikely]] {
    SPDLOG_ERROR("configurate: JSON filename is empty");
    throw std::invalid_argument("JSON filename is empty");
  }
  std::ifstream json_file(json_filenameParam);
  if (!json_file.is_open()) [[unlikely]] {
    throw std::invalid_argument("Cannot open file: " + json_filenameParam);
  }

  json json_data;
  try {
    json_data = json::parse(json_file);
  } catch (const nlohmann::detail::parse_error&) {
    SPDLOG_ERROR("configurate: JSON parse error in file {}",
                 json_filenameParam);
    json_file.close();
    throw std::invalid_argument("JSON parse error in file: " +
                                json_filenameParam);
  } catch (...) {
    SPDLOG_ERROR("configurate: Unknown error with parsing file {}",
                 json_filenameParam);
    json_file.close();
    throw std::invalid_argument("Unknown error with parsing file: " +
                                json_filenameParam);
  }
  json_file.close();

  try {
    setValuesApp(json_data, app);
  } catch (const std::invalid_argument& e) {
    throw;
  } catch (...) {
    throw std::invalid_argument("Unknown error with parsing file {}" +
                                json_filenameParam);
  }
  SPDLOG_INFO("Device configured: IP={}, port={}", app.getDeviceSocketIP(),
              app.getDeviceSocketPort());
}

void JSONParser::configurateServer(std::string json_filenameParam,
                                   Server& server) const {
  if (json_filenameParam.empty()) [[unlikely]] {
    SPDLOG_ERROR("configurate: JSON filename is empty");
    throw std::invalid_argument("JSON filename is empty");
  }
  std::ifstream json_file(json_filenameParam);
  if (!json_file.is_open()) [[unlikely]] {
    throw std::invalid_argument("Cannot open file: " + json_filenameParam);
  }

  json json_data;
  try {
    json_data = json::parse(json_file);
  } catch (const nlohmann::detail::parse_error& e) {
    SPDLOG_ERROR("configurate: JSON parse error in file {}",
                 json_filenameParam);
    json_file.close();
    throw std::invalid_argument("JSON parse error in file" +
                                json_filenameParam);
  } catch (...) {
    SPDLOG_ERROR("configurate: Unknown error with parsing file {}",
                 json_filenameParam);
    json_file.close();
    throw std::invalid_argument("Unknown error with parsing file" +
                                json_filenameParam);
  }
  json_file.close();

  try {
    setValuesServer(json_data, server);
  } catch (const std::invalid_argument& e) {
    throw e;
  } catch (const std::exception& e) {
    throw std::runtime_error(e.what());
  } catch (...) {
    throw std::runtime_error("Completely unknown error with file");
  }
  SPDLOG_INFO("Server configured: port={}, IP={}", server.getPort(),
              server.getIp(), server.getPosition());
}
