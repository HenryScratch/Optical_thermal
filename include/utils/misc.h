
#ifndef OBJECT_TRACKER_MISC_H
#define OBJECT_TRACKER_MISC_H

#include <opencv2/opencv.hpp>

namespace OT::utils {
/**
 * Check if there's another frame in the video capture. We do this by first checking if the user has quit (i  .e. pressed
 * the "Q" key) and then trying to retrieve the next frame of the video.
 */
bool hasFrame(cv::VideoCapture& capture);
/**
 * Resize the image so that neither # rows nor # cols exceed maxDimension.
 * Preserve the aspect ratio though.
 * Set maxDimension = -1 if you don't want to do any scaling.
 */
void scale(cv::Mat& img, std::int64_t maxDimension);
}


#endif //OBJECT_TRACKER_MISC_H
