#pragma once
#include <opencv2/core.hpp>

// A single raw detection produced by the detector (no ONNX Runtime dependency).
struct Detection
{
    int class_id;
    float confidence;
    cv::Rect box;
};
