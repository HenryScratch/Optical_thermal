#include "config.h"
#include "utils/misc.h"
#ifdef FMT
#include "utils/log.h"
#endif
#include "tracking.h"

#include "nlohmann/json.hpp"

#include "lyra/arg.hpp"
#include "lyra/arguments.hpp"
#include "lyra/help.hpp"
#include "lyra/lyra.hpp"
#include "lyra/opt.hpp"

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <string_view>

namespace fs = std::filesystem;

OT::config::TrackingMode ToMode(const std::string &mode) {
  OT::config::TrackingMode m =
      mode == "dir" ? OT::config::TrackingMode::DIRECTORY
                    : (mode == "file" ? OT::config::TrackingMode::FILE
                                      : OT::config::TrackingMode::RAW_FILE);
  return m;
}

OT::config::Config read_json(const fs::path &p_file) {
  std::ifstream file(p_file);
  nlohmann::json file_content;
  file >> file_content;

#ifdef FMT
  auto optLogLevel =
      OT::utils::ToLevel(file_content.value<std::string>("logLevel", "info"));
  auto spdLogLevel = spdlog::level::info;

  if (optLogLevel.has_value()) {
    spdLogLevel = optLogLevel.value();
  }
#endif

  return OT::config::Config{
      file_content.value<std::int64_t>("maxDimension", -1),
#ifdef FMT
      spdLogLevel,
      fs::path{file_content.value<std::string>("logPath", "tracker.log")},
#endif
      fs::path{file_content.value<std::string>("outputPath", "output.json")},
      fs::path{file_content["inputPath"].get<std::string>()},
      ToMode(file_content.value<std::string>("mode", "dir")),
      file_content.value<std::int64_t>("webCamNumber", -1),
      file_content.value<std::vector<std::int64_t>>("perspectivePoints", {}),
      file_content.value<long>("lifetimeThreshold", 20),
      file_content.value<float>("distanceThreshold", 0.1),
      file_content.value<long>("missedFramesThreshold", 10),
      file_content.value<float>("dt", 0.2),
      file_content.value<float>("magnitudeOfAccelerationNoise", 0.5),
      file_content.value<int>("lifetimeSuppressionThreshold", 20),
      file_content.value<float>("distanceSuppressionThreshold", 0.1),
      file_content.value<float>("ageSuppressionThreshold", 2),
      file_content.value<double>("foregroundThresh", 130.),
      file_content.value<double>("foregroundMaxVal", 255.),
  };
}

int main(int argc, char *argv[]) {
  try {
    bool showHelp = false;

    fs::path inputPath;

    auto cli =
        lyra::help(showHelp) |
        lyra::opt(inputPath, "inputPath")["-c"]["--config"]("config file")
            .required();

    auto result = cli.parse({argc, argv});
    if (!result) {
      std::cerr << "Error in command line: " << result.message() << std::endl;
      std::cout << cli << std::endl;
      return EXIT_FAILURE;
    }

    if (showHelp) {
      std::cout << cli << std::endl;
      return EXIT_SUCCESS;
    }

    OT::config::Config config;
    config = read_json(inputPath);

#ifdef FMT
    OT::utils::configureLogger(config);
#endif
    OT::tracking::Tracker tracker(config);

    tracker.add_detection_callback(
        [](std::uint64_t frame_num,
           const std::vector<OT::tracking::TrackingDetectionEvent> &objects) {
          std::string str;
          for (auto &obj : objects) {
            std::stringstream ss;
            ss << obj.location;
            str += fmt::format(" obj_id: {}, location: {}", obj.object_id,
                               ss.str());
          }
          std::cout << fmt::format(
                           "detection_callback - frame: {}; objects: [{}]",
                           frame_num, str)
                    << std::endl;
        });

    tracker.add_tracking_callback(
        [](std::uint64_t frame_num,
           const std::vector<OT::tracking::TrackingDetectionEvent> &objects) {
          std::string str;
          for (auto &obj : objects) {
            std::stringstream ss;
            ss << obj.location;
            str += fmt::format(" obj_id: {}, location: {}", obj.object_id,
                               ss.str());
          }
          std::cout << fmt::format(
                           "tracking_callback - frame: {}; objects: [{}]",
                           frame_num, str)
                    << std::endl;
        });

    tracker.add_end_tracking_callback(
        [](std::uint64_t frame_num,
           const std::vector<OT::tracking::EndTrackingEvent> &objects) {
          std::string str;
          for (auto &obj : objects) {
            str += fmt::format(
                " obj_id: {}, start_frame_id: {}, end_frame_id: {}",
                obj.object_id, obj.start_frame_id, obj.end_frame_id);
          }
          std::cout << fmt::format(
                           "end_tracking_callback - frame: {}; objects: [{}]",
                           frame_num, str)
                    << std::endl;
        });

    tracker.run();
  } catch (std::exception &e) {
    std::cerr << e.what() << std::endl;
  }

  return 0;
}
