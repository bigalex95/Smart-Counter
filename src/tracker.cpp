#include "tracker.h"
#include <cmath>
#include <limits>

using namespace std;
using namespace cv;

SimpleTracker::SimpleTracker(int max_frames_missing, int distance_threshold)
    : max_frames_missing(max_frames_missing), distance_threshold(distance_threshold) {}

float SimpleTracker::calculate_distance(Point p1, Point p2)
{
    return sqrt(pow(p1.x - p2.x, 2) + pow(p1.y - p2.y, 2));
}

vector<TrackedObject> SimpleTracker::update(const vector<Detection> &detections)
{
    // 1. Convert detections to centroids
    vector<Point> input_centroids;
    vector<Rect> input_boxes;
    for (const auto &det : detections)
    {
        if (det.class_id != 0)
            continue; // track people only (COCO class 0)

        Point center(det.box.x + det.box.width / 2, det.box.y + det.box.height / 2);
        input_centroids.push_back(center);
        input_boxes.push_back(det.box);
    }

    // Tracker is empty: just register every detection as a new object
    if (objects.empty())
    {
        for (size_t i = 0; i < input_centroids.size(); i++)
        {
            TrackedObject obj;
            obj.id = next_id++;
            obj.center = input_centroids[i];
            obj.previous_center = input_centroids[i]; // new objects: previous position = current position
            obj.box = input_boxes[i];
            obj.frames_since_seen = 0;
            objects[obj.id] = obj;
        }

        // Return the current state
        vector<TrackedObject> result;
        for (auto &pair : objects)
            result.push_back(pair.second);
        return result;
    }

    // 2. Matching of existing tracks and new detections
    // Greedy nearest-neighbour search
    // Keys of the map are IDs of existing objects

    // Mark all existing tracks as "missed" (+1 frame)
    for (auto &pair : objects)
    {
        pair.second.frames_since_seen++;
    }

    // Try to find a match for every new detection
    for (size_t i = 0; i < input_centroids.size(); i++)
    {
        Point current_center = input_centroids[i];

        int best_id = -1;
        float min_dist = (float)distance_threshold; // distance gate

        for (auto &pair : objects)
        {
            float dist = calculate_distance(pair.second.center, current_center);
            if (dist < min_dist)
            {
                min_dist = dist;
                best_id = pair.first;
            }
        }

        if (best_id != -1)
        {
            // Match found: first keep the old position
            objects[best_id].previous_center = objects[best_id].center;
            // then update to the new one
            objects[best_id].center = current_center;
            objects[best_id].box = input_boxes[i];
            objects[best_id].frames_since_seen = 0;
        }
        else
        {
            // Nothing nearby -> new object
            TrackedObject new_obj;
            new_obj.id = next_id++;
            new_obj.center = current_center;
            new_obj.previous_center = current_center; // new objects: previous position = current position
            new_obj.box = input_boxes[i];
            new_obj.frames_since_seen = 0;
            objects[new_obj.id] = new_obj;
        }
    }

    // 3. Remove dead tracks
    // Drop tracks that have not been seen for too long
    auto it = objects.begin();
    while (it != objects.end())
    {
        if (it->second.frames_since_seen > max_frames_missing)
        {
            it = objects.erase(it);
        }
        else
        {
            ++it;
        }
    }

    // Return the result
    vector<TrackedObject> result;
    for (auto &pair : objects)
    {
        // Return only recently seen tracks (avoid drawing ghosts)
        if (pair.second.frames_since_seen < 2)
        {
            result.push_back(pair.second);
        }
    }
    return result;
}