#ifndef JSONPARSER_H
#define JSONPARSER_H

#include <string>
#include <string_view>
#include <vector>

class App;
class Server;

#include <nlohmann/json.hpp>

using json = nlohmann::json;

class JSONParser {
 private:
  bool validateJSONOnRequiredFiledsForApp(const json& json_file) const noexcept;
  bool validateJSONOnRequiredFiledsForServer(
      const json& json_file) const noexcept;
  bool isIPValid(std::string_view ip) const noexcept;
  bool isIMEIValid(const std::vector<char>& imei) const noexcept;
  bool isIMSIValid(const std::vector<char>& imsi) const noexcept;
  bool isLocationValid(const std::vector<float>& location) const noexcept;
  void setValuesApp(const json& json_data, App& app) const;
  void setValuesServer(const json& json_data, Server& server) const;

 public:
  void configurateApp(std::string json_filenameParam, App& app) const;
  void configurateServer(std::string json_filenameParam, Server& app) const;
};

#endif  // JSONPARSER_H