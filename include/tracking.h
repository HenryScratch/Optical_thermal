

#ifndef OBJECT_TRACKER_TRACKING_H
#define OBJECT_TRACKER_TRACKING_H

#include "config.h"
#include "tracker/tracker_log.h"
#include "tracker/multi_object_tracker.h"
#include "tracker/contour_finder.h"
#include "utils/misc.h"
#include "utils/draw.h"
#include "utils/perspective_transformer.h"

#include "opencv2/opencv.hpp"

#include <filesystem>
#include <vector>
#include <set>
#include <fstream>

namespace fs = std::filesystem;

namespace OT::tracking{

    struct TrackingDetectionEvent{
        int object_id;
        cv::Point location;
    };

    struct EndTrackingEvent{
        std::uint64_t object_id, start_frame_id, end_frame_id;
    };

    using detection_callback = std::function<void(std::uint64_t frame_num, const std::vector<TrackingDetectionEvent>& objects)>;
    using tracking_callback = std::function<void(std::uint64_t frame_num, const std::vector<TrackingDetectionEvent>& objects)>;
    using end_tracking_callback = std::function<void(std::uint64_t frame_num, const std::vector<EndTrackingEvent>& objects)>;

    class CustomVideoCapture: public cv::VideoCapture{
        bool open(const cv::String &filename) {
            return CustomVideoCapture::open(filename, cv::CAP_ANY);
        }
        bool open(const cv::String &filename, int apiPreference) override {
            return CustomVideoCapture::open(filename, apiPreference, {});
        }
        virtual bool open(const cv::String& dirname, int apiPreference, const std::vector<int> &params) override {};
        CustomVideoCapture &operator>>(cv::Mat &image) override = 0;
    };

    class FramesDirCapture: public CustomVideoCapture{
    public:

        bool open(const cv::String& dirname, int apiPreference, const std::vector<int> &params) override;

        FramesDirCapture &operator>>(cv::Mat &image) override;

        [[nodiscard]] bool isOpened() const override {
            return !filenames.empty() && cur_iter != filenames.end();
        }

        bool grab() override;

    private:
        cv::Mat cur_image;
        std::map<std::uint64_t, fs::path> filenames;
        std::map<std::uint64_t, fs::path>::iterator cur_iter = filenames.begin();
    };

    class RawFileCapture: public CustomVideoCapture{
    public:
        bool open(const cv::String &filename){
            return RawFileCapture::open(filename, cv::CAP_ANY);
        }

        bool open(const cv::String &filename, int apiPreference) override {
            return RawFileCapture::open(filename, apiPreference, {});
        }

        bool open(const cv::String& dirname, int apiPreference, const std::vector<int> &params) override;

        RawFileCapture &operator>>(cv::Mat &image) override;

        [[nodiscard]] bool isOpened() const override {
            return m_file != nullptr;
        }

        bool grab() override;

        static const uint32_t X_img = 288, Y_img = 384;
    private:
        bool already_grabbed = false;

        cv::Mat cur_image;
        std::unique_ptr<std::FILE> m_file;
        std::uint64_t filepos = 0;
    };

    class Tracker{
    public:
        explicit Tracker(const OT::config::Config& config);
        void run();
        std::string track_frame(const cv::Mat& frame);
        bool show_windows = true;

        void add_tracking_callback(const tracking_callback& );
        void add_detection_callback(const detection_callback & );
        void add_end_tracking_callback(const end_tracking_callback& );

    private:
        // This does the actual tracking of the objects. We can't initialize it now because
        // it needs to know the size of the frame. So, we set it equal to nullptr and initialize
        // it after we get the first frame.
        std::unique_ptr<OT::MultiObjectTracker> tracker = nullptr;

        // We'll use this variable to store the current frame captured from the video.
        cv::Mat m_frame;

        // This object represents the video or image sequence that we are reading from.
        std::unique_ptr<cv::VideoCapture> capture = nullptr;

        // These two variables store the contours and contour hierarchy for the current frame.
        // We won't use the hierarchy, but we need it in order to be able to call the cv::findContours
        // function.
        std::vector<cv::Vec4i> hierarchy;
        std::vector<std::vector<cv::Point> > contours;

        // We'll use a ContourFinder to do the actual extraction of contours from the image.
        OT::ContourFinder contourFinder;

        OT::TrackerLog trackerLog{true};
        cv::Mat perspectiveMatrix;
        cv::Size perspectiveSize;
        std::vector<cv::Point2f> points;

        std::uint64_t frameNumber = 0;
        std::int64_t maxDimension;

        std::unique_ptr<cv::VideoWriter> vw_orig = nullptr, vw_video = nullptr;

        OT::config::Config config;

        std::set<std::uint64_t> object_ids;

        std::vector<detection_callback> detection_callbacks;
        std::vector<tracking_callback> tracking_callbacks;
        std::vector<end_tracking_callback> end_tracking_callbacks;
    };
}

#endif //OBJECT_TRACKER_TRACKING_H
