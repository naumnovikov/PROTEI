#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/basic_file_sink.h>

#include "app.h"
#include "server.h"

void init_logger();

int main()
{
    init_logger();
    #if defined(APP)
        App app;
        app.interact();
    #elif defined(SERVER)
        Server server;
        server.interact();
    #else
        std::cerr << "Wrong macro!\n";
    #endif
    return 0;
}

void init_logger(){
    auto console_sink{std::make_shared<spdlog::sinks::stdout_color_sink_mt>()};
    auto file_sink{std::make_shared<spdlog::sinks::basic_file_sink_mt>("protei.log", false)};

    console_sink->set_level(spdlog::level::info);
    file_sink->set_level(spdlog::level::trace);
    file_sink->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%l] [%s:%#] %v");

    spdlog::sinks_init_list sinks = { console_sink, file_sink };
    auto logger{std::make_shared<spdlog::logger>("app_logger", sinks.begin(), sinks.end())};
    logger->set_level(spdlog::level::trace);
    logger->flush_on(spdlog::level::warn);

    spdlog::set_default_logger(logger);
}