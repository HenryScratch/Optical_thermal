
#include "utils/misc.h"

#include <opencv2/opencv.hpp>

#include <cctype>

namespace OT::utils {
bool hasFrame(cv::VideoCapture& capture) {
    bool hasNotQuit = ((char) cv::waitKey(1)) != 'q';
    bool hasAnotherFrame = capture.grab();
    return hasNotQuit && hasAnotherFrame;
}

void scale(cv::Mat& img, std::int64_t maxDimension) {
    if (maxDimension == -1) {
        return;
    }
    if (maxDimension >= img.rows && maxDimension >= img.cols) {
        return;
    }

    std::double_t scale = (1.0 * (std::double_t)maxDimension) / img.rows;
    if (img.cols > img.rows) {
        scale = (1.0 * (std::double_t)maxDimension) / img.cols;
    }

    auto newRows = (std::int64_t)(img.rows * scale);
    auto newCols = (std::int64_t)(img.cols * scale);

    cv::resize(img, img, cv::Size2l(newCols, newRows));
}
} // OT:utils
