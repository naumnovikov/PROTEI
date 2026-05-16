#include <exception>
#include <ctype.h>
#include <sstream>
#include <numeric>

#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/basic_file_sink.h>

#include "app.h"
#include "move.h"
#include "active.h"
#include "exit.h"
#include "protocol.h"

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
    size_t tokens_quantity{tokens.size()};
    if (tokens_quantity < 2) {
        throw std::invalid_argument("MOVE must get at least 1 argument");
    }else if (tokens_quantity > 4){
        spdlog::warn("MOVE: too many arguments, skipping extras");
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
        appWorkingState = WorkingState::NOT_WORKING;
        spdlog::error("Input stream closed or error");
        throw std::runtime_error("Input stream closed or error");
    }
    return command_buffer;
}

void App::interact(){
    spdlog::info("Configuring from config.json.");
    std::string json_filename{"config.json"};
    JSONParser parser;
    try{
        parser.configurateApp(std::move(json_filename), *this);
    }catch(const std::invalid_argument& e){
        spdlog::error("Configuration error: {}", e.what());
        return;
    }catch(const std::out_of_range& e){
        spdlog::error("Configuration error: {}", e.what());
        return;
    }catch(...){
        spdlog::error("Configuration error");
        return;
    }

    while (appWorkingState == WorkingState::WORKING){
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


