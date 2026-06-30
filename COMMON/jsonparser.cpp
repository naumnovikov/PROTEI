#include "jsonparser.h"

#include <spdlog/fmt/bundled/ranges.h>
#include <spdlog/spdlog.h>

#include <exception>
#include <utility>

#include "common_types.h"

#ifdef BUILD_UE
#include "ue.h"
#endif

#ifdef BUILD_SIMTEL
#include "simtel.h"
#endif

bool CommonJsonValidator::isIPValid(std::string_view ip) const {
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

bool CommonJsonValidator::isLocationValid(
    const position_vector& location) const noexcept {
  if (location.size() != COORDINATES_QUANTITY) [[unlikely]] {
    SPDLOG_ERROR("Wrong location: no valid size");
    return false;
  }
  return true;
}

#ifdef BUILD_UE

bool UEJsonParser::validateJSONOnRequiredFiledsForUE(
    const json& json_data) const {
  if (!json_data.contains("ip") || !json_data.contains("imei") ||
      !json_data.contains("imsi") || !json_data.contains("location") ||
      !json_data.contains("server_ip") || !json_data.contains("server_port"))
      [[unlikely]] {
    SPDLOG_ERROR("configurate: Missing required fields for UE");
    return false;
  }
  return true;
}

bool UEJsonParser::isIMEIValid(std::string_view imei) const noexcept {
  if (imei.size() != IMEI_SIZE) [[unlikely]] {
    SPDLOG_ERROR("Wrong IMEI: no valid size");
    return false;
  }
  return true;
}

bool UEJsonParser::isIMSIValid(std::string_view imsi) const noexcept {
  if (imsi.size() > MAX_IMSI_SIZE || imsi.size() < MIN_IMSI_SIZE) [[unlikely]] {
    SPDLOG_ERROR("Wrong IMSI: no valid size");
    return false;
  }
  return true;
}

void UEJsonParser::setValuesUE(const json& json_data, UE& ue) const {
  if (!validateJSONOnRequiredFiledsForUE(json_data)) [[unlikely]] {
    throw std::invalid_argument{"Missing required fields"};
  }

  CommonJsonValidator validator{};

  IPv4 ip{json_data["ip"].get<IPv4>()};
  if (!validator.isIPValid(ip)) [[unlikely]] {
    throw std::invalid_argument{"No valid IP"};
  }
  ue.setDeviceIP(std::move(ip));

  IMEI imei{json_data["imei"].get<IMEI>()};
  if (!isIMEIValid(imei)) [[unlikely]] {
    throw std::invalid_argument{"No valid IMEI in file"};
  }
  ue.setDeviceIMEI(std::move(imei));

  IMSI imsi{json_data["imsi"].get<IMSI>()};
  if (!isIMSIValid(imsi)) [[unlikely]] {
    throw std::invalid_argument{"No valid IMSI in file"};
  }
  ue.setDeviceIMSI(std::move(imsi));

  position_vector location{json_data["location"].get<position_vector>()};
  if (!validator.isLocationValid(location)) [[unlikely]] {
    throw std::invalid_argument{"No valid location in file"};
  }
  ue.setDeviceLocation(std::move(location));

  IPv4 server_ip{json_data["server_ip"].get<IPv4>()};
  if (!validator.isIPValid(server_ip)) [[unlikely]] {
    throw std::invalid_argument{"No valid server_ip"};
  }

  PORT server_port{json_data["server_port"].get<PORT>()};
  if (server_port < IANA_REGISTRED_PORTS_MIN ||
      server_port > IANA_REGISTRED_PORTS_MAX) [[unlikely]] {
    throw std::invalid_argument{"Wrong PORT according to IANA"};
  }

  ue.setDeviceServerAddress(NetworkAddress{std::move(server_ip), server_port});
}

#endif

#ifdef BUILD_SIMTEL

bool SimtelJsonParser::validateJSONOnRequiredFiledsForServer(
    const json& json_data) const {
  if (!json_data.contains("base_stations") ||
      !json_data["base_stations"].is_array()) [[unlikely]] {
    SPDLOG_ERROR("configurate: Missing 'base_stations' array for Server");
    return false;
  }
  return true;
}

void SimtelJsonParser::setValuesSIMTEL(const json& json_data,
                                       SIMTEL& simtel) const {
  if (!validateJSONOnRequiredFiledsForServer(json_data)) {
    throw std::invalid_argument{"Missing required fields for SIMTEL"};
  }

  CommonJsonValidator validator{};

  for (const auto& bs_json : json_data["base_stations"]) {
    if (!bs_json.contains("id") || !bs_json.contains("ip") ||
        !bs_json.contains("port") || !bs_json.contains("location") ||
        !bs_json.contains("radius")) {
      SPDLOG_WARN("Skipping invalid BS entry: missing fields");
      continue;
    }
    ENODE_B_ID id{bs_json["id"].get<ENODE_B_ID>()};
    std::string bs_ip{bs_json["ip"].get<std::string>()};
    if (!validator.isIPValid(bs_ip)) {
      continue;
    }
    PORT bs_port{bs_json["port"].get<PORT>()};
    if (bs_port < IANA_REGISTRED_PORTS_MIN ||
        bs_port > IANA_REGISTRED_PORTS_MAX) {
      continue;
    }
    position_vector location{bs_json["location"].get<position_vector>()};
    if (!validator.isLocationValid(location)) {
      continue;
    }
    BS_RADIUS radius{bs_json["radius"].get<BS_RADIUS>()};
    if (radius <= 0) {
      continue;
    }

    simtel.addBaseStation(id, bs_ip, bs_port, std::move(location), radius);
    SPDLOG_INFO("Added BS id={} at {}:{} with radius {}", id, bs_ip, bs_port,
                radius);
  }
}

#endif //BUILD_SIMTEL

#ifdef BUILD_UE
void JSONParser::configurateUE(std::string json_filenameParam, UE& ue) const {
  if (json_filenameParam.empty()) [[unlikely]] {
    SPDLOG_ERROR("configurate: JSON filename is empty");
    throw std::invalid_argument{"JSON filename is empty"};
  }
  JSON_FILE json_file{json_filenameParam};
  if (!json_file.is_open()) [[unlikely]] {
    throw std::invalid_argument{"Cannot open file: " + json_filenameParam};
  }

  json json_data;
  try {
    json_data = json::parse(json_file);
  } catch (const nlohmann::detail::parse_error&) {
    SPDLOG_ERROR("configurate: JSON parse error in file {}",
                 json_filenameParam);
    json_file.close();
    throw std::invalid_argument{"JSON parse error in file: " +
                                json_filenameParam};
  }
  json_file.close();

  UEJsonParser ue_parser{};
  ue_parser.setValuesUE(json_data, ue);

  SPDLOG_INFO("Device configured: IP={}, server_port={}, server_ip={}",
              ue.getDeviceIP(), ue.getDeviceSocketPort(),
              ue.getDeviceSocketIP());
}
#endif

#ifdef BUILD_SIMTEL
void JSONParser::configurateSIMTEL(std::string json_filenameParam,
                                   SIMTEL& simtel) const {
  if (json_filenameParam.empty()) [[unlikely]] {
    SPDLOG_ERROR("configurate: JSON filename is empty");
    throw std::invalid_argument{"JSON filename is empty"};
  }
  JSON_FILE json_file{json_filenameParam};
  if (!json_file.is_open()) [[unlikely]] {
    throw std::invalid_argument{"Cannot open file: " + json_filenameParam};
  }

  json json_data;
  try {
    json_data = json::parse(json_file);
  } catch (const nlohmann::detail::parse_error&) {
    SPDLOG_ERROR("configurate: JSON parse error in file {}",
                 json_filenameParam);
    json_file.close();
    throw std::invalid_argument{"JSON parse error in file" +
                                json_filenameParam};
  }
  json_file.close();

  SimtelJsonParser simtel_parser{};
  simtel_parser.setValuesSIMTEL(json_data, simtel);

  SPDLOG_INFO("SIMTEL configured");
}
#endif
