#include "app.h"
#include "move.h"
#include "active.h"
#include "exit.h"
#include "protocol.h"

#include <exception>
#include <fstream>
#include <ctype.h>
#include <sstream>
#include <numeric>
#include <utility>

#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/basic_file_sink.h>

void printStatus(Status status) noexcept{
    if (status == Status::ACTIVE){
        std::cout << "ACTIVE";
    }else if (status == Status::NON_ACTIVE){
        std::cout << "NON_ACTIVE";
    }else{
        spdlog::warn("Unknown Status value");
    }
}

void printTypeOfProtocol(TypeOfProtocol typeOfProtocol) noexcept{
    if (typeOfProtocol == TypeOfProtocol::JSON){
        std::cout << "JSON";
    }else if (typeOfProtocol == TypeOfProtocol::BINARY){
        std::cout << "BINARY";
    }else{
        spdlog::warn("Unknown TypeOfProtocol value");
    }
}

void App::printMenu() noexcept{
    std::cout << "App status: ";
    printStatus(status);
    std::cout << '\n';
    std::cout << "IMSI: ";
    for (const char& imsi_ch : device.imsi){
        std::cout << static_cast<int>(imsi_ch);
    }
    std::cout << "\n1. ACTIVE\n";
    std::cout << "2. MOVE\n";
    std::cout << "3. PROTOCOL\n";
    std::cout << "4. EXIT\n";
}

bool App::validateJSONOnRequiredFileds(const json& json_data) noexcept{
    if (!json_data.contains("ip") || !json_data.contains("port") || !json_data.contains("imei") || !json_data.contains("imsi") || !json_data.contains("location") || !json_data.contains("config") || !json_data.contains("nodes")) {
        spdlog::error("configurate: {}", "Missing required fields");
        return false;
    }
    return true;
}

bool App::isIPValid(std::string_view ip) noexcept{
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

bool App::isIMEIValid(const std::vector<char>& imei) noexcept{
    if (imei.size() != 15){
        spdlog::error("Wrong IMEI: no valid size");
        return false;
    }
    return true;
}

bool App::isIMSIValid(const std::vector<char>& imsi) noexcept{
    if (imsi.size() > 15){
        spdlog::error("Wrong IMSI: no valid size");
        return false;
    }
    return true;
}

bool App::isLocationValid(const std::vector<float>& location) noexcept{
    if (location.size() !=3){
        spdlog::error("Wrong location: no valid size");
        return false;
    }
    return true;
}

void App::setValues(const json& json_data){
    if (!validateJSONOnRequiredFileds(json_data)){
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
    device.imei = std::move(imei);

    std::vector<char> imsi{json_data["imsi"].get<std::vector<char>>()};
    if (!isIMSIValid(imsi)){
        throw std::invalid_argument("No valid IMSI in file");
    }
    device.imsi = std::move(imsi);

    std::vector<float> location{json_data["location"].get<std::vector<float>>()};
    if (!isLocationValid(location)){
        throw std::invalid_argument("No valid lcoation in file");
    }
    device.location = std::move(location);


    device.config = json_data["config"].get<std::string>();
    device.nodes = json_data["nodes"].get<std::string>();

    device.socket = NetworkAddress(ip, port);
}

void App::configurate(std::string json_filenameParam) {
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
        setValues(json_data);
    }catch(const std::invalid_argument& e){
        throw e;
    }catch(...){
        throw std::invalid_argument("Unknown error with parsing file {}" + json_filenameParam);
    }
    spdlog::info("Device configured: IP={}, port={}", device.socket.getIp(), device.socket.getPort());
}


std::vector<std::string> App::interpretateInputCommand(std::string command_buffer){
    std::vector<std::string> tokens;
    std::istringstream iss(std::move(command_buffer));
    std::string token;
    while (iss >> token){
        if (!token.empty()){
            tokens.push_back(std::move(token));
        }
    }
    return tokens;
}

void App::ProcessACTIVE(const std::vector<std::string>& tokens){
    if (tokens.size() < 2) {
        throw std::invalid_argument("ACTIVE without arguments");
    }

    std::string arg{tokens[1]};

    for (char& c : arg){
        c = std::toupper(c);
    }
    if (arg == "TRUE" || arg == "1"){
        Active cmd(*this, Status::ACTIVE);
        cmd.execute();
    }else if (arg == "FALSE" || arg == "0"){
        Active cmd(*this, Status::NON_ACTIVE);
        cmd.execute();
    }else{
        spdlog::warn("Invalid argument for ACTIVE");
    }
}

void App::ProcessMOVE(const std::vector<std::string>& tokens){
    if (status == Status::NON_ACTIVE){
        throw std::logic_error("Cannot move as status is NON_ACTIVE");
    }
    size_t tokens_quantity{0};
    if (tokens.size() < 2) {
        throw std::invalid_argument("MOVE must get at least 1 argument");
    }else if (tokens.size() > 4){
        spdlog::warn("MOVE: too many arguments, skipping extras");
    }else{
        tokens_quantity = tokens.size();
    }
    std::vector<float> new_location;
    try {
        for (size_t i{1}; i < tokens_quantity; ++i){
            new_location.push_back(std::stof(tokens[i]));
        }
    } catch (...) {
        spdlog::error("MOVE: invalid number format");
        throw std::invalid_argument("Invalid number format");
    }
    Move cmd(*this, new_location);
    cmd.execute();
}

void App::ProcessEXIT(const std::vector<std::string>& tokens){
    if (status == Status::ACTIVE){
        spdlog::warn("EXIT denied: status is ACTIVE");
        throw std::logic_error("Cannot exit as status is ACTIVE");
    }
    Exit cmd(*this);
    cmd.execute();
}
void App::ProcessPROTOCOL(const std::vector<std::string>& tokens){
    if (tokens.size() < 2) {
        throw std::invalid_argument("PROTOCOL without arguments");
    }
    std::string arg{tokens[1]};

    for (char& c : arg){
        c = std::toupper(c);
    }
    if (arg == "JSON"){
        Protocol cmd(*this, TypeOfProtocol::JSON);
        cmd.execute();
    }else if (arg == "BINARY"){
        Protocol cmd(*this, TypeOfProtocol::BINARY);
        cmd.execute();
    }else{
        throw std::invalid_argument("Invalid argument. Use JSON or BINARY");
    }
}

std::string App::inputCommand(){
    std::cout << ">> ";
    std::string command_buffer;
    if (!std::getline(std::cin, command_buffer)){
        appWorkingState = AppWorkingState::NOT_WORKING;
        spdlog::error("Input stream closed or error");
        throw std::runtime_error("Input stream closed or error");
    }
    return command_buffer;
}

void App::interact(){
    std::cout << "Configurate device from JSON. Input JSON-filename: ";
    std::string json_filename;
    std::cin >> json_filename;
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); // wihtout cin.ignore() cin takes garbage in first command input

    try{
        configurate(std::move(json_filename));
    }catch(const std::invalid_argument& e){
        spdlog::error("Configuration error: {}", e.what());
        return;
    }catch(const std::out_of_range& e){
        spdlog::error("Configuration error: {}", e.what());
        return;
    }catch(...){
        spdlog::error("Configuration error: {}");
        return;
    }

    while (appWorkingState == AppWorkingState::WORKING){
        printMenu();
        std::string command_buffer;
        try{
            command_buffer = inputCommand();
        }catch(...){
            break;
        }

        if (command_buffer.empty()){
            continue;
        }
        std::vector<std::string> tokens{interpretateInputCommand(std::move(command_buffer))};

        std::string input_command{tokens.at(0)};
        for (char& c : input_command){
            c = std::toupper(c);
        }

        if (input_command == "ACTIVE"){
            try{
                ProcessACTIVE(tokens);
            }catch(const std::invalid_argument& e){
                spdlog::warn("{}", e.what());
                continue;
            }catch(...){
                spdlog::warn("Unknown ACTIVE error!");
                continue;
            }
        }else if (input_command == "MOVE"){
            try{
                ProcessMOVE(tokens);
            }catch(const std::invalid_argument& e){
                spdlog::warn("{}", e.what());
                continue;
            }catch(const std::logic_error& e){
                spdlog::warn("{}", e.what());
                continue;
            }catch(...){
                spdlog::warn("Unknown MOVE error!");
                continue;
            }
        }
        else if (input_command == "PROTOCOL"){
            try{
                ProcessPROTOCOL(tokens);
            }catch(const std::invalid_argument& e){
                spdlog::warn("{}", e.what());
                continue;
            }catch(...){
                spdlog::warn("Unknown PROTOCOL error!");
                continue;
            }
        }
        else if (input_command == "EXIT"){
            try{
                ProcessEXIT(tokens);
            }catch(const std::logic_error& e){
                spdlog::warn("{}", e.what());
                continue;
            }catch(...){
                spdlog::warn("Unknown EXIT error!");
                continue;
            }
        }
        else {
            spdlog::warn("Unknown command: '{}'", input_command);
            continue;
        }
    }
}


