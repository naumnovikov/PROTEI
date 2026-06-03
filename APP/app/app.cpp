#include "app.h"

#include <ctype.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

#include <cerrno>
#include <cmath>
#include <cstring>
#include <exception>
#include <numeric>
#include <sstream>

#include "active.h"
#include "exit.h"
#include "move.h"
#include "protocol.h"

constexpr int ACTIVE_COMMAND_MIN_TOKENS{2};
constexpr int MOVE_COMMAND_MIN_TOKENS{2};
constexpr int MOVE_COMMAND_MAX_TOKENS{4};
constexpr int PROTOCOL_COMMAND_MIN_TOKENS{2};

void printStatus(Status status) noexcept {
  if (status == Status::ACTIVE) {
    std::cout << "ACTIVE";
    return;
  }
  std::cout << "NON_ACTIVE";
}

void printTypeOfProtocol(TypeOfProtocol typeOfProtocol) noexcept {
  if (typeOfProtocol == TypeOfProtocol::JSON) {
    std::cout << "JSON";
  } else if (typeOfProtocol == TypeOfProtocol::BINARY) {
    std::cout << "BINARY";
  } else [[unlikely]] {
    SPDLOG_WARN("Unknown TypeOfProtocol value");
  }
}

void App::printMenu() const noexcept {
  std::cout << "App status: ";
  printStatus(status);
  std::cout << '\n';
  std::cout << "IMSI: " << device.imsi;
  std::cout << "\n1. ACTIVE\n";
  std::cout << "2. MOVE\n";
  std::cout << "3. PROTOCOL\n";
  std::cout << "4. EXIT\n";
}

std::vector<std::string> App::interpretateInputCommand(
    std::string command_buffer) {
  std::vector<std::string> tokens;
  std::istringstream iss(std::move(command_buffer));
  std::string token;
  while (iss >> token) {
    if (!token.empty()) [[likely]] {
      tokens.push_back(std::move(token));
    }
  }
  return tokens;
}

void App::turnStringIntoUpper(std::string& str) const {
  for (char& c : str) {
    c = std::toupper(c);
  }
}

void App::ProcessACTIVE(std::vector<std::string> tokens) {
  if (tokens.size() < ACTIVE_COMMAND_MIN_TOKENS) [[unlikely]] {
    throw std::invalid_argument("ACTIVE without arguments");
  }

  std::string arg{tokens[1]};

  turnStringIntoUpper(arg);
  if (arg == "TRUE" || arg == "1") {
    Active cmd(*this, Status::ACTIVE);
    cmd.execute();
  } else if (arg == "FALSE" || arg == "0") {
    Active cmd(*this, Status::NON_ACTIVE);
    cmd.execute();
  } else [[unlikely]] {
    SPDLOG_WARN("Invalid argument for ACTIVE");
  }
}

void App::ProcessMOVE(std::vector<std::string> tokens, Sock& sock) {
  if (status == Status::NON_ACTIVE) {
    throw std::logic_error("Cannot move as status is NON_ACTIVE");
  }
  std::size_t tokens_quantity{tokens.size()};
  if (tokens_quantity < MOVE_COMMAND_MIN_TOKENS) [[unlikely]] {
    throw std::invalid_argument("MOVE must get at least 1 argument");
  } else if (tokens_quantity > MOVE_COMMAND_MAX_TOKENS) {
    SPDLOG_WARN("MOVE: too many arguments, skipping extras");
  }
  if (tokens_quantity > 3) {
    tokens_quantity = 4;
  }
  std::vector<float> new_location;
  try {
    for (std::size_t i{1}; i < tokens_quantity; ++i) {
      new_location.push_back(std::stof(tokens[i]));
    }
    Move cmd(*this, new_location, sock);
    cmd.execute();
  } catch (const std::invalid_argument& e) {
    SPDLOG_ERROR("MOVE: invalid number format: {}", e.what());
  } catch (const std::out_of_range& e) {
    SPDLOG_ERROR("MOVE: index out of range: {}", e.what());
  } catch (const std::bad_alloc& e) {
    SPDLOG_ERROR("MOVE: memory allocation failed: {}", e.what());
  }
  // If any other exception drops,
  // I catch it in processInteract()
  // in catch(...) so here's no double logging
}

void App::ProcessEXIT(std::vector<std::string> tokens) {
  if (status == Status::ACTIVE) {
    SPDLOG_WARN("EXIT denied: status is ACTIVE");
    throw std::logic_error("Cannot exit as status is ACTIVE");
  }
  Exit cmd(*this);
  cmd.execute();
}
void App::ProcessPROTOCOL(std::vector<std::string> tokens) {
  if (tokens.size() < PROTOCOL_COMMAND_MIN_TOKENS) [[unlikely]] {
    throw std::invalid_argument("PROTOCOL without arguments");
  }
  std::string arg{tokens[1]};

  turnStringIntoUpper(arg);
  if (arg == "JSON") {
    Protocol cmd(*this, TypeOfProtocol::JSON);
    cmd.execute();
  } else if (arg == "BINARY") {
    Protocol cmd(*this, TypeOfProtocol::BINARY);
    cmd.execute();
  } else {
    throw std::invalid_argument("Invalid argument. Use JSON or BINARY");
  }
}

std::string App::inputCommand() {
  std::cout << ">> ";
  std::string command_buffer;
  if (!std::getline(std::cin, command_buffer)) [[unlikely]] {
    appWorkingState = WorkingState::NOT_WORKING;
    SPDLOG_ERROR("Input stream closed or error");
    throw std::runtime_error("Input stream closed or error");
  }
  return command_buffer;
}

void App::processInteract(Sock& sock) {
  while (appWorkingState == WorkingState::WORKING) {
    printMenu();
    std::string command_buffer;
    try {
      command_buffer = inputCommand();
    } catch (...) {
      break;
    }

    if (command_buffer.empty()) [[unlikely]] {
      continue;
    }
    std::vector<std::string> tokens{
        interpretateInputCommand(std::move(command_buffer))};

    if (tokens.empty()) {
      continue;
    }

    std::string input_command{tokens[0]};
    for (char& c : input_command) {
      c = std::toupper(c);
    }

    if (input_command == "ACTIVE") {
      try {
        ProcessACTIVE(std::move(tokens));
      } catch (const std::invalid_argument& e) {
        SPDLOG_ERROR("{}", e.what());
        continue;
      } catch (...) {
        SPDLOG_ERROR("Unknown ACTIVE error!");
        continue;
      }
    } else if (input_command == "MOVE") {
      try {
        ProcessMOVE(std::move(tokens), sock);
      } catch (const std::logic_error& e) {
        SPDLOG_ERROR("{}", e.what());
        continue;
      } catch (...) {
        SPDLOG_ERROR("Unknown MOVE error!");
        continue;
      }
    } else if (input_command == "PROTOCOL") {
      try {
        ProcessPROTOCOL(std::move(tokens));
      } catch (const std::invalid_argument& e) {
        SPDLOG_ERROR("{}", e.what());
        continue;
      } catch (...) {
        SPDLOG_ERROR("Unknown PROTOCOL error!");
        continue;
      }
    } else if (input_command == "EXIT") {
      try {
        ProcessEXIT(std::move(tokens));
      } catch (const std::logic_error& e) {
        SPDLOG_ERROR("{}", e.what());
        continue;
      } catch (...) {
        SPDLOG_ERROR("Unknown EXIT error!");
        continue;
      }
    } else [[unlikely]] {
      SPDLOG_ERROR("Unknown command: '{}'", input_command);
      continue;
    }
  }
}

void App::interact() {
  std::cout << "Configurate device from JSON. Input JSON-filename: ";
  std::string json_filename;
  std::cin >> json_filename;
  std::cin.ignore(
      std::numeric_limits<std::streamsize>::max(),
      '\n');  // wihtout cin.ignore() cin takes garbage in first command input

  JSONParser parser;
  try {
    parser.configurateApp(std::move(json_filename), *this);
  } catch (const std::invalid_argument& e) {
    SPDLOG_ERROR("Configuration error: {}", e.what());
    return;
  } catch (const std::out_of_range& e) {
    SPDLOG_ERROR("Configuration error: {}", e.what());
    return;
  } catch (const std::exception& e) {
    SPDLOG_ERROR("Configuration error: {}", e.what());
  } catch (...) {
    SPDLOG_ERROR("Configuration error");
    return;
  }

  // COPYPASTED FROM
  // https://rsdn.org/article/unix/sockets.xml

  Sock socket_obj{AF_INET, SOCK_STREAM, TCP_VALUE};
  int sock_value{socket_obj.getSocket()};
  if (sock_value < 0) [[unlikely]] {
    SPDLOG_ERROR("Socket initialization error");
    return;
  }

  if (socket_obj.connectAppSocketToServer(
          device.serverAddress.getPort(),
          device.serverAddress.getIpString().c_str()) < 0) {
    SPDLOG_ERROR("Connection error");
    return;
  }

  processInteract(socket_obj);
}
