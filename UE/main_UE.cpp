#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

#include <iostream>

#include "ue.h"

void init_logger();

int main() {
  try {
    init_logger();
  } catch (...) {
    std::cerr << "Log initialization failed" << std::endl;
  }

  UE UE;
  UE.work();

  return 0;
}

// COPYPASTED FROM
// https://github.com/gabime/spdlog/wiki/QuickStart

void init_logger() {
  auto console_sink{std::make_shared<spdlog::sinks::stdout_color_sink_mt>()};
  console_sink->set_level(spdlog::level::info);
  console_sink->set_pattern(
      "[console_sink][%Y-%m-%d %H:%M:%S.%e] [%l] [%s:%#] %v");

  auto file_sink{
      std::make_shared<spdlog::sinks::basic_file_sink_mt>("protei.log", false)};
  file_sink->set_level(spdlog::level::trace);
  file_sink->set_pattern("[file_sink][%Y-%m-%d %H:%M:%S.%e] [%l] [%s:%#] %v");

  spdlog::sinks_init_list sinks{console_sink, file_sink};

  auto logger{
      std::make_shared<spdlog::logger>("logger", sinks.begin(), sinks.end())};
  logger->set_level(spdlog::level::trace);
  logger->flush_on(spdlog::level::warn);

  spdlog::set_default_logger(logger);
}