#pragma once
#include <opencv2/opencv.hpp>
#include <vector>
#include <map>
#include "detector.h" // Нам нужна структура Detection

struct TrackedObject
{
    int id;
    cv::Point center;
    cv::Point previous_center; // Предыдущая позиция для определения направления движения
    cv::Rect box;
    int frames_since_seen; // Чтобы не удалять объект сразу, если он моргнул
};

class SimpleTracker
{
public:
    SimpleTracker(int max_frames_missing = 5, int distance_threshold = 50);

    // Принимает сырые детекции, возвращает объекты с ID
    std::vector<TrackedObject> update(const std::vector<Detection> &detections);

private:
    int next_id = 0;
    std::map<int, TrackedObject> objects; // Хранилище активных объектов

    int max_frames_missing;
    int distance_threshold;

    float calculate_distance(cv::Point p1, cv::Point p2);
};