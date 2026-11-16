#pragma once
#include <opencv2/core.hpp>
#include <vector>
#include <map>
#include "detection.h"

struct TrackedObject
{
    int id;
    cv::Point center;
    cv::Point previous_center; // previous position, used to determine movement direction
    cv::Rect box;
    int frames_since_seen; // grace period: do not drop a track the moment a detection flickers
};

class SimpleTracker
{
public:
    SimpleTracker(int max_frames_missing = 5, int distance_threshold = 50);

    // Takes raw detections, returns tracked objects with persistent IDs
    std::vector<TrackedObject> update(const std::vector<Detection> &detections);

private:
    int next_id = 0;
    std::map<int, TrackedObject> objects; // active tracks

    int max_frames_missing;
    int distance_threshold;

    float calculate_distance(cv::Point p1, cv::Point p2);
};