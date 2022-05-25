

#ifndef OBJECT_TRACKER_LOG_H
#define OBJECT_TRACKER_LOG_H

#ifdef FMT

#include "config.h"

#include "spdlog/spdlog.h"
#include "spdlog/common.h"

#include <optional>
#include <string>
#include <map>

#define _REGISTER_LOG_LEVEL(level) {#level, level}


namespace OT::utils {
    inline std::optional<spdlog::level::level_enum>
    ToLevel(const std::string &level){
        using namespace spdlog::level;
        static const std::map<std::string, level_enum> levels = {
                _REGISTER_LOG_LEVEL(trace),
                _REGISTER_LOG_LEVEL(debug),
                _REGISTER_LOG_LEVEL(info),
                _REGISTER_LOG_LEVEL(warn),
                _REGISTER_LOG_LEVEL(err),
                _REGISTER_LOG_LEVEL(critical),
                _REGISTER_LOG_LEVEL(off),
        };

        assert(levels.size() == n_levels);

        const auto it = levels.find(level);
        if(it == levels.cend()){
            return std::nullopt;
        }
        return it->second;
    }

    void configureLogger(const OT::config::Config &cfg);

}

#endif

#endif //OBJECT_TRACKER_LOG_H
