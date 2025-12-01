#pragma once
#include <opencv2/opencv.hpp>
#include <onnxruntime_cxx_api.h>
#include <vector>
#include <string>

// Структура для хранения результата детекции
struct Detection
{
    int class_id;
    float confidence;
    cv::Rect box;
};

class YOLODetector
{
public:
    // Конструктор: загружает модель и настраивает сессию
    YOLODetector(const std::string &model_path, bool use_cuda = true);

    // Главный метод: принимает картинку, возвращает список найденных объектов
    std::vector<Detection> detect(cv::Mat &image, float conf_threshold = 0.5);

private:
    // Внутренние ресурсы ONNX Runtime
    Ort::Env env{nullptr};
    Ort::SessionOptions session_options{nullptr};
    Ort::Session session{nullptr};

    // Параметры модели (будем считывать их динамически)
    std::vector<const char *> input_names;
    std::vector<const char *> output_names;
    std::vector<int64_t> input_shape;

    // Вспомогательный метод для подготовки картинки
    std::vector<float> preprocess(const cv::Mat &image, float &scale);
};