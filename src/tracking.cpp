

#include "tracking.h"

#include "tracker/multi_object_tracker.h"
#include "tracker/contour_finder.h"
#include "utils/misc.h"
#include "utils/draw.h"
#include "utils/perspective_transformer.h"

#ifdef FMT
#include "spdlog/spdlog.h"
#include "spdlog/common.h"
#include "spdlog/fmt/ostr.h"
#endif

#include "opencv2/opencv.hpp"
#include "opencv2/video/tracking.hpp"

#include <filesystem>
#include <vector>
#include <set>
#include <thread>
#include <cstdlib>
#include <algorithm>

namespace fs = std::filesystem;

namespace OT::tracking{
    namespace fs = std::filesystem;

    static const double alpha = 1. / 16;

    bool matEquals(const cv::Mat& lhs, const cv::Mat& rhs){
        return cv::sum(cv::sum(lhs != rhs)).val[0] == 0;
    }

    bool FramesDirCapture::open(const cv::String &dirname, int apiPreference, const std::vector<int> &params) {
        auto dir = fs::path{dirname};
        if(!(fs::is_directory(dir) && fs::exists(dir))){
            return false;
        }

#ifdef FMT
        spdlog::info("Checking valid images in directory...");
#endif
        for(auto &file: fs::directory_iterator(dir)){
            if(fs::is_regular_file(file)){
                auto abs_path = fs::absolute(file.path()).lexically_normal();
                auto name_without_ext = abs_path.stem();

                std::uint64_t file_num;
                {
                    std::stringstream ss;
                    ss << name_without_ext.string();
                    if(!(ss >> file_num)){
                        continue;
                    }
                }
                cv::Mat img = cv::imread(abs_path.string());
                if(img.data != nullptr){
                    filenames[file_num] = abs_path;
                }
            }
        }

        cur_iter = filenames.begin();
#ifdef FMT
        spdlog::info("Count of valid images in directory \"{}\" is {}", dirname, std::to_string(filenames.size()));
#endif
        return true;
    }

    FramesDirCapture& FramesDirCapture::operator>>(cv::Mat &image){
#ifdef FMT
        spdlog::trace("Getting image from stream...");
#endif
        if(this->grab()){
            cur_iter++;
        }

        image = cur_image;
        return *this;
    }

    bool FramesDirCapture::grab() {
        if(cur_iter == filenames.end()){
#ifdef FMT
            spdlog::info("End of files");
#endif
            return false;
        }
        auto filename = cur_iter->second;
#ifdef FMT
        spdlog::debug("Reading {} ...", fs::absolute(filename).string());
#endif
        cv::Mat image = cv::imread(filename.string());
#ifdef FMT
        spdlog::trace("Size of read image is {}x{}", image.size[0], image.size[1]);
#endif
        if(image.size != cur_image.size || !matEquals(image, cur_image)){
#ifdef FMT
            spdlog::debug("Image changed");
            spdlog::debug("Grabbing new image with size {}", image.size);
#endif
            cur_image = image;
            return true;
        }
#ifdef FMT
        spdlog::warn("New image is equal to previous");
#endif
        return true;
    }


    Tracker::Tracker(const config::Config &config) {
        this->config = config;

        // Determine how to scale the video.
        maxDimension = config.maxDimension;

        // Read from the webcam or the parser.
        if (config.webCamNumber != -1) {
            capture = std::make_unique<cv::VideoCapture>(config.webCamNumber);
        } else {
            switch(config.mode){

                case config::TrackingMode::FILE:
                    capture = std::make_unique<cv::VideoCapture>();
                    break;
                case config::TrackingMode::DIRECTORY:
                    capture = std::make_unique<FramesDirCapture>();
                    break;
                case config::TrackingMode::RAW_FILE:
                    capture = std::make_unique<RawFileCapture>();
                    break;
            }
            capture->open(config.inputPath.string());
        }

        // Get the perspective transform, if there is one.
        auto perspectivePoints = config.perspectivePoints;
        OT::perspective::extractFourPoints(perspectivePoints, points);
        if (!points.empty()) {
            perspectiveMatrix = OT::perspective::getPerspectiveMatrix(points, perspectiveSize);
        }

#ifdef FMT
        // Ensure that the video has been opened correctly.
        if(!capture->isOpened()) {
            spdlog::error("Problem opening video source");
        }
#endif

        contourFinder.showWindows = show_windows;
    }

    void Tracker::run() {

        static const std::double_t needed_fps = 30.;
        static const auto needed_frame_time = std::chrono::duration<std::double_t>{1 / needed_fps};

        std::double_t fps = 0.;
        std::uint64_t frames = 0;
        std::chrono::duration<double> deltaTime{0.};

        if(show_windows){
            // Set the mouse callback.
            cv::namedWindow("Video");
            cv::namedWindow("Original");
        }

        cv::Mat frame;

        // Repeat while the user has not pressed "q" and while there's another frame.

        //Error here Seg Dump OT::utils::hasFrame(*capture)
        while(OT::utils::hasFrame(*capture)) {

            auto start = std::chrono::steady_clock::now();

            // Fetch the next frame.
            *capture >> frame;
#ifdef FMT

            spdlog::trace("Got new image from stream");
#endif
            track_frame(frame);

            auto end = std::chrono::steady_clock::now();
            std::chrono::duration<double> elapsed_seconds = end - start;

            deltaTime += elapsed_seconds;
            frames++;


            if( std::chrono::duration_cast<std::chrono::seconds>(deltaTime) >= std::chrono::milliseconds{100}){
                fps = frames * 0.7 + fps * 0.3; // stable
                frames = 0;
                deltaTime -= std::chrono::milliseconds {100};

                auto avgFrameTime = std::chrono::duration<std::double_t>{1.0 / fps};
                auto time_to_sleep = needed_frame_time - avgFrameTime;
                if(time_to_sleep.count() > 0){
                    std::this_thread::sleep_for(time_to_sleep);
#ifdef FMT
//                    spdlog::trace("Sleeping for {} seconds", time_to_sleep);
#endif
                }
#ifdef FMT
                spdlog::debug("fps: {}", fps);
#endif
            }
        }
#ifdef FMT
        spdlog::info("average fps: {}", fps);
#endif
    }

    std::string Tracker::track_frame(const cv::Mat& frame) {

        m_frame = frame;
        frameNumber++;


        if(show_windows){
            std::cout<<std::endl<< m_frame<<std::endl;

            cv::Mat tmp_frame = m_frame ;

            m_frame.convertTo(tmp_frame, CV_8U, alpha);
            cv::imshow("Original", tmp_frame);
        }

        // Do the perspective transform.
        if (!points.empty()) {
            cv::warpPerspective(m_frame, m_frame, perspectiveMatrix, perspectiveSize);
        }

        // Scale the image.
        OT::utils::scale(m_frame, maxDimension);

        // Create the tracker if it isn't created yet.
        if (tracker == nullptr) {
            tracker = std::make_unique<OT::MultiObjectTracker>(
                    cv::Size(m_frame.rows, m_frame.cols),
                    config.lifetimeThreshold,
                    config.distanceThreshold,
                    config.missedFramesThreshold,
                    config.dt,
                    config.magnitudeOfAccelerationNoise,
                    config.lifetimeSuppressionThreshold,
                    config.distanceSuppressionThreshold,
                    config.ageSuppressionThreshold);
        }

        // Set the frame dimension.
        trackerLog.setDimensions(m_frame.cols, m_frame.rows);

        // Find the contours.
        std::vector<cv::Point2f> mc(contours.size());
        std::vector<cv::Rect> boundRect(contours.size());
        contourFinder.findContours(m_frame, hierarchy, contours, mc, boundRect, config.foregroundThresh, config.foregroundMaxVal);

        OT::utils::draw::contourShow("Contours", contours, boundRect, m_frame.size());

        // Update the predicted locations of the objects based on the observed
        // mass centers.
        std::vector<OT::TrackingOutput> predictions;
        tracker->update(mc, boundRect, predictions);

        std::set<std::uint64_t> cur_objs;

        std::vector<TrackingDetectionEvent> new_events, events;


        for (int i(0); i < predictions.size(); ++i) {
            auto &pred = predictions[i];



            if(pred.trajectory.empty()){
                new_events.push_back({pred.id, pred.location });
            }
            events.push_back({pred.id, pred.location });
            cur_objs.insert(pred.id);

            // Draw a cross at the location of the prediction.
            OT::utils::draw::drawCross(m_frame, pred.location, pred.color, 5);

            // Draw the trajectory for the prediction.
            OT::utils::draw::drawTrajectory(m_frame, pred.trajectory, pred.color);

            // Update the tracker log.
            trackerLog.addTrack(pred.id, pred.location.x, pred.location.y, (long)frameNumber);
        }

        std::vector<std::uint64_t> res;
        std::set_difference(object_ids.begin(), object_ids.end(), cur_objs.begin(), cur_objs.end(), std::inserter(res, std::end(res)));
        std::vector<EndTrackingEvent> end_events;

        for(const auto &item: res){
            end_events.push_back({item, (std::uint64_t)(trackerLog.birthFrameForTrackerId[item]), frameNumber});
        }

        for(const auto &cb: detection_callbacks) {
            cb(frameNumber, new_events);
        }

        for(const auto &cb: tracking_callbacks){
            cb(frameNumber, new_events);
        }

        for(const auto &cb: end_tracking_callbacks){
            cb(frameNumber, end_events);
        }


        if(show_windows){
            cv::Mat tmp_frame = m_frame;
            m_frame.convertTo(tmp_frame, CV_8U, alpha);
            cv::imshow("Video", tmp_frame);
        }

        std::stringstream ss;
        trackerLog.logToStream(ss);
        return ss.str();
    }

    void Tracker::add_detection_callback(const detection_callback &cb) {
        detection_callbacks.push_back(cb);
    }

    void Tracker::add_end_tracking_callback(const end_tracking_callback &cb) {
        end_tracking_callbacks.push_back(cb);

    }

    void Tracker::add_tracking_callback(const tracking_callback &cb) {
        tracking_callbacks.push_back(cb);
    }


    bool RawFileCapture::open(const cv::String &dirname, int apiPreference, const std::vector<int> &params) {
        auto dir = fs::path{dirname};
        if(!(fs::is_regular_file(dir) && fs::exists(dir))){
            return false;
        }

        m_file = std::unique_ptr<std::FILE>(std::fopen(dir.string().c_str(), "rb"));
        return true;
    }

    bool RawFileCapture::grab() {
        //std::cout << *capture << std::endl;
        if(already_grabbed){
            return true;
        }

        auto buf = new std::int16_t[X_img * Y_img];

        fread(buf, sizeof(std::int16_t), (X_img * Y_img), m_file.get());

        const auto tmp_filepos = ftell(m_file.get());

        if(tmp_filepos == filepos || feof(m_file.get())){
            delete[] buf;
            return false;
        }
        filepos = tmp_filepos;

        cv::Mat frame(X_img, Y_img, CV_16SC1, buf);
        //delete[] buf;

        cur_image = frame;

        //cur_image.convertTo(cur_image, CV_8U, alpha);
        already_grabbed = true;

        return true;
    }

    RawFileCapture &RawFileCapture::operator>>(cv::Mat &image) {
        this->grab();
        image = cur_image;
        already_grabbed = false;
        return *this;
    }
} // OT

