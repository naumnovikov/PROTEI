#include <iostream>   
#include <sstream>       
#include <cmath>         
#include <limits>        
#include <ctype.h>

#include <spdlog/spdlog.h> 

#include "server.h"

Server::Server(Server&& other) noexcept : port(other.port), position(other.position){
    other.port = 0;
    other.position = {0, 0, 0};
    spdlog::info("Servers' move-constuctor used");
}

Server& Server::operator=(Server&& other) noexcept{
    if (this != &other){
        port = other.port;
        position = other.position;
        other.port = 0;
        other.position = {0, 0, 0};
    }
    spdlog::info("Servers' move-operator = used");
    return *this;
}

std::string Server::inputPosition(){
    std::cout << ">> ";
    std::string input_buffer;
    if (!std::getline(std::cin, input_buffer)){
        serverWorkingState = WorkingState::NOT_WORKING;
        spdlog::error("Input stream closed or error");
        throw std::runtime_error("Input stream closed or error");
    }
    return input_buffer;
}

std::vector<std::string> Server::interpretateInput(std::string input_buffer){
    std::vector<std::string> tokens;
    std::istringstream iss(std::move(input_buffer));
    std::string token;
    while (iss >> token){
        if (!token.empty()){
            tokens.push_back(std::move(token));
        }
    }
    return tokens;
}

bool Server::isExitCommand(std::string firstToken){
    for (char& ch : firstToken){
        ch = std::toupper(ch);
    }

    if (firstToken == "EXIT"){
        return true;
    }
    return false;
}

void Server::processPositionInput(std::vector<std::string> tokens){  
    size_t tokens_quantity{tokens.size()};
    if (tokens_quantity == 0){
        throw std::invalid_argument("Empty input");
    }

    std::string firstToken{tokens.at(0)};
    if (isExitCommand(std::move(firstToken))){
        serverWorkingState = WorkingState::NOT_WORKING;
        spdlog::info("Exiting...");
        return;
    }    


    if (tokens_quantity != 3) {
        throw std::invalid_argument("Input must have 3 arguments");
    }



    std::vector<float> new_position;
    try {
        for (size_t i{0}; i < tokens_quantity; ++i){
            new_position.push_back(std::stof(tokens.at(i)));
        }
    } catch (...) {
        spdlog::error("MOVE: invalid number format");
        throw std::invalid_argument("Invalid number format");
    }

    float distance{static_cast<float>(std::sqrt(std::pow(std::abs(position.at(0) - new_position.at(0)), 2) + 
                                            std::pow(std::abs(position.at(1) - new_position.at(1)), 2) + 
                                            std::pow(std::abs(position.at(2) - new_position.at(2)), 2)))};
    spdlog::info("Distance is {}", distance);
}

void Server::interact(){
    spdlog::info("Configuring from serverconfig.json.");
    std::string json_filename{"serverconfig.json"};
    JSONParser parser;
    try{
        parser.configurateServer(std::move(json_filename), *this);
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

        while (serverWorkingState == WorkingState::WORKING){
            std::string input_buffer;
            try{
                input_buffer = inputPosition();
            }catch(...){
                break;
            }

            if (input_buffer.empty()){
                continue;
            }
            std::vector<std::string> tokens{interpretateInput(std::move(input_buffer))};

            try{
                processPositionInput(std::move(tokens));
            }catch(const std::invalid_argument& e){
                spdlog::warn("{}", e.what());
            }catch(...){
                spdlog::warn("Unknown error");
            }
        }
}