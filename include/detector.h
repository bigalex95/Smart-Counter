#pragma once
#include <opencv2/opencv.hpp>
#include <onnxruntime_cxx_api.h>
#include <vector>
#include <string>
#include "detection.h"

class YOLODetector
{
public:
    // Constructor: loads the model and configures the session
    YOLODetector(const std::string &model_path, bool use_cuda = true);

    // Main method: takes an image, returns a list of detected objects
    std::vector<Detection> detect(cv::Mat &image, float conf_threshold = 0.5);

private:
    // Internal ONNX Runtime resources
    Ort::Env env{nullptr};
    Ort::SessionOptions session_options{nullptr};
    Ort::Session session{nullptr};

    // Model parameters (read dynamically)
    std::vector<const char *> input_names;
    std::vector<const char *> output_names;
    std::vector<int64_t> input_shape;

    // Helper method for image preprocessing
    std::vector<float> preprocess(const cv::Mat &image, float &scale);
};