#include <iostream>
#include <opencv2/opencv.hpp>
#include "detector.h"

int main()
{
    // Пути к файлам (относительно корня проекта)
    std::string model_path = "models/yolov8s.onnx";
    std::string video_path = "data/videos/853889-hd_1920_1080_25fps.mp4";

    // Инициализация детектора
    std::cout << "Initializing Detector..." << std::endl;
    YOLODetector detector(model_path, true); // false = CPU only

    // Открытие видео
    cv::VideoCapture cap(video_path);
    if (!cap.isOpened())
    {
        std::cerr << "Error: Could not open video!" << std::endl;
        return -1;
    }

    cv::Mat frame;
    while (true)
    {
        cap >> frame;
        if (frame.empty())
            break;

        // Запуск детекции
        // Засекаем время для честного FPS
        auto start = std::chrono::high_resolution_clock::now();

        std::vector<Detection> results = detector.detect(frame, 0.5);

        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        float fps = 1000.0f / (duration.count() + 1e-5); // +1e-5 чтобы не делить на 0

        // Отрисовка
        for (const auto &det : results)
        {
            // Рисуем только людей (class_id == 0 для COCO датасета)
            if (det.class_id == 0)
            {
                cv::rectangle(frame, det.box, cv::Scalar(0, 255, 0), 2);
                cv::putText(frame, "Person", det.box.tl(), cv::FONT_HERSHEY_SIMPLEX, 0.6, cv::Scalar(0, 255, 0), 2);
            }
        }

        // Вывод FPS
        cv::putText(frame, "FPS: " + std::to_string((int)fps), cv::Point(20, 40),
                    cv::FONT_HERSHEY_SIMPLEX, 1, cv::Scalar(0, 0, 255), 2);

        cv::imshow("C++ YOLOv8 Inference", frame);
        if (cv::waitKey(1) == 'q')
            break;
    }

    return 0;
}