#include <iostream>
#include <opencv2/opencv.hpp>
#include "detector.h"
#include "tracker.h"
#include "fps_counter.h"
#include <set>

int main()
{
    // Пути к файлам (относительно корня проекта)
    std::string model_path = "models/yolov8s.onnx";
    std::string video_path = "data/videos/853889-hd_1920_1080_25fps.mp4";

    // Инициализация детектора
    std::cout << "Initializing Detector..." << std::endl;
    YOLODetector detector(model_path, true); // false = CPU only
    SimpleTracker tracker; // Создаем трекер

    // Открытие видео
    cv::VideoCapture cap(video_path);
    if (!cap.isOpened())
    {
        std::cerr << "Error: Could not open video!" << std::endl;
        return -1;
    }
    
    std::set<int> counted_ids;
    int line_y = cap.get(cv::CAP_PROP_FRAME_HEIGHT) / 2; // Линия на середине кадра
    
    // FPS counter for tracking performance
    FPSCounter fps_counter;

    cv::Mat frame;
    while (true)
    {
        cap >> frame;
        if (frame.empty())
            break;

        // Запуск детекции
        // Засекаем время для честного FPS
        auto start = std::chrono::high_resolution_clock::now();

        // 1. Детекция
        auto detections = detector.detect(frame, 0.5);
        
        // 2. Трекинг (превращаем просто боксы в объекты с ID)
        auto tracked_objects = tracker.update(detections);

        // 3. Логика подсчета и отрисовка
        const int line_tolerance = 20;  // Зона вокруг линии для детекции
        cv::Scalar line_color(0, 255, 0);  // По умолчанию зеленая
        
        for (const auto& obj : tracked_objects) {
            // Рисуем бокс и ID
            cv::rectangle(frame, obj.box, cv::Scalar(0, 255, 0), 2);
            cv::putText(frame, "ID: " + std::to_string(obj.id), 
                        cv::Point(obj.box.x, obj.box.y - 10), 
                        cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0, 255, 0), 2);
            
            // Рисуем центральную точку
            cv::circle(frame, obj.center, 5, cv::Scalar(0, 255, 0), -1);
            
            // Логика пересечения
            if (obj.center.y > line_y - line_tolerance && obj.center.y < line_y + line_tolerance) {
                 if (counted_ids.find(obj.id) == counted_ids.end()) {
                     counted_ids.insert(obj.id);
                     // Меняем цвет линии на красный для визуальной обратной связи
                     line_color = cv::Scalar(0, 0, 255);
                 }
            }
        }
        
        // Рисуем линию подсчета (цвет меняется на красный при подсчете)
        cv::line(frame, cv::Point(0, line_y), cv::Point(frame.cols, line_y), line_color, 2);
        
        // Рисуем зону толерантности (желтые линии)
        cv::line(frame, cv::Point(0, line_y - line_tolerance), 
                 cv::Point(frame.cols, line_y - line_tolerance), cv::Scalar(0, 255, 255), 1);
        cv::line(frame, cv::Point(0, line_y + line_tolerance), 
                 cv::Point(frame.cols, line_y + line_tolerance), cv::Scalar(0, 255, 255), 1);
        
        // Вывод счетчика
        cv::putText(frame, "Count: " + std::to_string(counted_ids.size()), 
                   cv::Point(10, 50), cv::FONT_HERSHEY_SIMPLEX, 1.5, cv::Scalar(0, 255, 255), 3);

        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        float frame_time_ms = static_cast<float>(duration.count());
        
        // Add sample to FPS counter
        fps_counter.addSample(frame_time_ms);
        
        // Get FPS metrics
        float avg_fps = fps_counter.getAverageFPS();
        float instant_fps = fps_counter.getInstantFPS();
        int frame_count = fps_counter.getFrameCount();

        // Display FPS on frame (showing both average and instantaneous) - top-right corner
        std::string fps_text = "FPS: " + std::to_string(static_cast<int>(instant_fps)) + 
                               " (avg: " + std::to_string(static_cast<int>(avg_fps)) + ")";
        int baseline = 0;
        cv::Size text_size = cv::getTextSize(fps_text, cv::FONT_HERSHEY_SIMPLEX, 1, 2, &baseline);
        cv::Point fps_position(frame.cols - text_size.width - 20, 40);  // 20px padding from right edge
        cv::putText(frame, fps_text, fps_position,
                    cv::FONT_HERSHEY_SIMPLEX, 1, cv::Scalar(0, 0, 255), 2);

        // Print periodic statistics every 60 frames
        if (frame_count > 0 && frame_count % 60 == 0) {
            std::cout << "Frame " << frame_count << " — Avg FPS: " << avg_fps 
                      << ", Instant FPS: " << instant_fps << std::endl;
        }

        cv::imshow("C++ YOLOv8 Inference", frame);
        if (cv::waitKey(1) == 'q')
            break;
    }

    // Print final summary
    std::cout << "\n--- Summary ---" << std::endl;
    std::cout << "Frames processed: " << fps_counter.getFrameCount() << std::endl;
    std::cout << "Average FPS: " << fps_counter.getAverageFPS() << std::endl;

    return 0;
}