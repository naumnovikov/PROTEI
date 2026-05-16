#ifndef JSONPARSER_H
#define JSONPARSER_H

#include <string>
#include <string_view>
#include <vector>

class App; 
class Server;

#include <nlohmann/json.hpp>

using json = nlohmann::json;

class JSONParser{
private:
    bool validateJSONOnRequiredFiledsForApp(const json& json_file) noexcept;
    bool validateJSONOnRequiredFiledsForServer(const json& json_file) noexcept;
    bool isIPValid(std::string_view ip) noexcept;
    bool isIMEIValid(const std::vector<char>& imei) noexcept;
    bool isIMSIValid(const std::vector<char>& imsi) noexcept;
    bool isLocationValid(const std::vector<float>& location) noexcept;
    void setValuesApp(const json& json_data, App& app);
    void setValuesServer(const json& json_data, Server& server);
public:
    void configurateApp(std::string json_filenameParam, App& app);
    void configurateServer(std::string json_filenameParam, Server& app);
};

#endif // JSONPARSER_H