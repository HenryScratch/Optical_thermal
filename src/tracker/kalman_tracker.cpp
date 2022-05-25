

#include "tracker/kalman_tracker.h"

#include <memory>

#include <opencv2/video/tracking.hpp>

#include <vector>
#include <cstdlib>     /* srand, rand */
#include <ctime>       /* time */

namespace OT {
    KalmanTracker::KalmanTracker(cv::Point startPt,
                                 float dt,
                                 float magnitudeOfAccelerationNoise,
                                 size_t maxTrajectorySize) {

        // Seed the random number generator and pick a random ID and random color.
        srand(time(NULL) + startPt.x + startPt.y);
        this->id = rand();
        this->color = cv::Scalar(rand() % 256, rand() % 256, rand() % 256);

        this->maxTrajectorySize = maxTrajectorySize;
        this->kf = std::make_unique<cv::KalmanFilter>();
        this->trajectory = std::make_shared<std::vector<cv::Point>>();
        this->numFramesWithoutUpdate = 0;
        this->prediction = startPt;
        this->lifetime = 0;

        // Initialize filter with 4 dynamic parameters (x, y, x velocity, y
        // velocity), 2 measurement parameters (x, y), and no control parameters.
        this->kf->init(4, 2, 0);

        // Set the pre and post states.
        this->kf->statePre.setTo(0);
        this->kf->statePre.at<float>(0, 0) = startPt.x;
        this->kf->statePre.at<float>(1, 0) = startPt.y;

        this->kf->statePost.setTo(0);
        this->kf->statePost.at<float>(0, 0) = startPt.x;
        this->kf->statePost.at<float>(1, 0) = startPt.y;

        // Create the matrices.
        this->kf->transitionMatrix = (cv::Mat_<float>(4, 4) << 1,0,dt,0,   0,1,0,dt,  0,0,1,0,  0,0,0,1);

        cv::setIdentity(this->kf->measurementMatrix);
        this->kf->processNoiseCov = (cv::Mat_<float>(4, 4) <<
                                                           pow(dt,4.0)/4.0, 0, pow(dt,3.0)/2.0, 0,
                0, pow(dt,4.0)/4.0 , 0 ,pow(dt,3.0)/2.0,
                pow(dt,3.0)/2.0, 0, pow(dt,2.0), 0,
                0, pow(dt,3.0)/2.0, 0, pow(dt,2.0));
        this->kf->processNoiseCov *= magnitudeOfAccelerationNoise;

        cv::setIdentity(this->kf->measurementNoiseCov, cv::Scalar::all(0.1));
        cv::setIdentity(this->kf->errorCovPost, cv::Scalar::all(0.1));
    }

    cv::Point KalmanTracker::correct(cv::Point pt) {
        cv::Mat_<float> measurement = cv::Mat_<float>::zeros(2, 1);
        measurement(0) = pt.x;
        measurement(1) = pt.y;
        cv::Mat estimated = this->kf->correct(measurement);
        cv::Point statePt(estimated.at<float>(0), estimated.at<float>(1));
        this->prediction.x = statePt.x;
        this->prediction.y = statePt.y;
        return statePt;
    }

    cv::Point KalmanTracker::predict() {
        cv::Mat prediction = this->kf->predict();
        cv::Point predictedPt(prediction.at<float>(0), prediction.at<float>(1));
        this->kf->statePre.copyTo(this->kf->statePost);
        this->kf->errorCovPre.copyTo(this->kf->errorCovPost);
        this->addPointToTrajectory(predictedPt);
        this->prediction = predictedPt;
        return predictedPt;
    }

    cv::Point KalmanTracker::latestPrediction() {
        return this->prediction;
    }

    void KalmanTracker::addPointToTrajectory(cv::Point pt) {
        if (this->trajectory->size() >= this->maxTrajectorySize) {
            this->trajectory->erase(this->trajectory->begin(),
                                    this->trajectory->begin()+1);
        }
        this->trajectory->push_back(pt);
    }


    long KalmanTracker::getLifetime() {
        return this->lifetime;
    }

    void KalmanTracker::noUpdateThisFrame() {
        this->lifetime++;
        this->numFramesWithoutUpdate++;
    }

    int KalmanTracker::getNumFramesWithoutUpdate() {
        return this->numFramesWithoutUpdate;
    }

    void KalmanTracker::gotUpdate() {
        this->lifetime++;
        this->numFramesWithoutUpdate = 0;
    }

    OT::TrackingOutput KalmanTracker::latestTrackingOutput() {
        auto output = OT::TrackingOutput{
                this->id,
                this->latestPrediction(),
                this->color,
                std::vector<cv::Point>()
        };
        std::copy(this->trajectory->cbegin(),
                  this->trajectory->cend(),
                  std::back_inserter(output.trajectory));
        return output;
    }
}

