
#include "utils/log.h"
#ifdef DEV

#include "spdlog/sinks/rotating_file_sink.h"
#include "spdlog/sinks/stdout_color_sinks.h"


void
OT::utils::configureLogger(const OT::config::Config &cfg) {
    std::vector<spdlog::sink_ptr> sinks;
    sinks.push_back(std::make_shared<spdlog::sinks::stdout_color_sink_st>());
    sinks.push_back(std::make_shared<spdlog::sinks::rotating_file_sink_mt>(
            cfg.logPath.c_str(), 1024 * 1024, 10, false));

    auto log = std::make_shared<spdlog::logger>("tracking_objects", std::begin(sinks),
                                                std::end(sinks));
    std::string jsonpattern = {
            "{\"time\": \"%Y-%m-%dT%H:%M:%S.%f%z\", \"name\": \"%n\", \"level\": "
            "\"%^%l%$\", \"thread\": %t, \"message\": \"%v\"}"};
    log->set_pattern(jsonpattern);
    log->set_level(cfg.logLevel);

    spdlog::register_logger(log);
    spdlog::set_default_logger(log);
}
#endif
