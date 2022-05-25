

#ifndef OBJECT_TRACKER_CONFIG_H
#define OBJECT_TRACKER_CONFIG_H

#ifdef DEV
#include <spdlog/common.h>
#endif

#include <optional>
#include <map>
#include <string>
#include <filesystem>
#include <vector>

namespace OT{
    namespace detail{
        template<class... Ts> struct overload : Ts... { using Ts::operator()...; };
        template<class... Ts> overload(Ts...) -> overload<Ts...>;
    } // detail

    namespace config{
        namespace fs = std::filesystem;

        enum class TrackingMode{
            FILE,
            DIRECTORY,
            RAW_FILE,
        };

        struct Config{
            std::int64_t maxDimension;
#ifdef DEV
            spdlog::level::level_enum logLevel;
            fs::path logPath;
#endif
            fs::path outputPath;
            fs::path inputPath;
            TrackingMode mode;
            std::int64_t webCamNumber;
            std::vector<std::int64_t> perspectivePoints;
            long lifetimeThreshold = 20;
            float distanceThreshold = 0.1;
            long missedFramesThreshold = 10;
            float dt = 0.2;
            float magnitudeOfAccelerationNoise = 0.5;
            int lifetimeSuppressionThreshold = 20;
            float distanceSuppressionThreshold = 0.1;
            float ageSuppressionThreshold = 2;

            double foregroundThresh = 130.;
            double foregroundMaxVal = 255.;
        };

    } // config
}

#endif //OBJECT_TRACKER_CONFIG_H
