

#ifndef OBJECT_TRACKER_CONTOUR_FINDER_H
#define OBJECT_TRACKER_CONTOUR_FINDER_H

#include <vector>

#include <opencv2/opencv.hpp>
#include <opencv2/video/tracking.hpp>

namespace OT {
    /**
     * This class will find blobs representing objects in a frame. It uses
     * background subtraction to isolate the foreground, does some preprocessing, finds
     * contours, and removes small contours.
     */
    class ContourFinder {
    private:
        // The background subtractor that isolates the foreground.
        cv::Ptr<cv::BackgroundSubtractorMOG2> bg;

        // The foreground of the frame that should contain the blobs.
        cv::Mat foreground;

        // Remove contours that are too small.
        void filterOutBadContours(std::vector<std::vector<cv::Point>>& contours) const;

        // Filter out contours whose area is <= contourSizeThreshold * area of largest contour.
        float contourSizeThreshold;

        // We use median filter to remove noise, this is the size of the filter.
        // It must be an odd number.
        int medianFilterSize;

        // A threshold value between 0 and 1 that indicates when to merge to contours.
        // We merge if the distance between their mass centers is <= diagonal.
        float contourMergeThreshold;

        // The length of the diagonal.
        float diagonal;

        // Ignore mass centers that appear in these rectangles.
        std::vector<cv::Rect> suppressRectangles;

        // Suppress any mass centers that appear in the suppress rectangles.
        void suppressMassCenters(std::vector<std::vector<cv::Point>>& contours,
                                 std::vector<cv::Point2f>& massCenters,
                                 std::vector<cv::Rect>& boundingBoxes);

        /**
         * Find the mass centers and bounding boxes for the given contours.
         */
        static void getCentersAndBoundingBoxes(const std::vector<std::vector<cv::Point>>& contours,
                                        std::vector<cv::Point2f>& massCenters,
                                        std::vector<cv::Rect>& boundingBoxes);

        /**
         * Merge nearby contours.
         */
        void mergeContours(std::vector<std::vector<cv::Point> > &contours,
                           const std::vector<cv::Point2f>& massCenters,
                           const std::vector<cv::Rect>& boundingBoxes) const;
    public:
        ContourFinder(int history = 1000,
                      int nMixtures = 3,
                      float contourSizeThreshold = 0.1,
                      int medianFilterSize = 9,
                      float contourMergeThreshold = 0.01);

        /**
         * Find contours representing the objects in the frame.
         */
        void findContours(const cv::Mat& frame,
                          std::vector<cv::Vec4i>& hierarchy,
                          std::vector<std::vector<cv::Point>>& contours,
                          std::vector<cv::Point2f>& massCenters,
                          std::vector<cv::Rect>& boundingBoxes,
                          double foregroundThresh = 130.,
                          double foregroundMaxVal = 255.);

        void suppressRectangle(cv::Rect rect);

        bool showWindows = false;
    };
}


#endif //OBJECT_TRACKER_CONTOUR_FINDER_H
