
#ifndef OBJECT_TRACKER_DRAW_H
#define OBJECT_TRACKER_DRAW_H

#include <vector>

#include <opencv2/opencv.hpp>

namespace OT::utils::draw {
        /**
         * Draw a cross on the image.
         *
         * img - The image
         * center - The center of the cross.
         * color - The RGB color.
         * diameter - The diameter of the cross.
         */
        void drawCross(const cv::Mat& img,
                       const cv::Point& center,
                       const cv::Scalar& color,
                       const int diameter);

        cv::Point drawBoundingRect(cv::Mat& img,
                                   const cv::Rect& boundingRect);

        void drawTrajectory(const cv::Mat& img,
                            const std::vector<cv::Point>& trajectory,
                            const cv::Scalar& color);

        /**
         * Draw the contours in a new image and show them.
         */
        void contourShow(const std::string& drawingName,
                         const std::vector<std::vector<cv::Point>>& contours,
                         const std::vector<cv::Rect>& boundingRect,
                         cv::Size imgSize);
    }


#endif //OBJECT_TRACKER_DRAW_H
