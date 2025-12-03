#include <iostream>
#include <opencv2/opencv.hpp>
#include "detector.h"
#include "tracker.h"
#include "fps_counter.h"
#include <set>

void print_usage(const char *program_name)
{
    std::cout << "Usage: " << program_name << " [options]\n\n"
              << "Options:\n"
              << "  --model <path>      Path to ONNX model (default: models/yolov8s.onnx)\n"
              << "  --input <path>      Path to input video (default: data/videos/853889-hd_1920_1080_25fps.mp4)\n"
              << "  --output <path>     Path to output video (default: data/output/output.mp4)\n"
              << "  --headless          Run without display window (save to file only)\n"
              << "  --cpu               Use CPU only (default: GPU if available)\n"
              << "  --help              Show this help message\n"
              << "\nExamples:\n"
              << "  " << program_name << " --input video.mp4\n"
              << "  " << program_name << " --model models/yolov8n.onnx --headless\n"
              << "  " << program_name << " --input video.mp4 --output result.mp4 --cpu\n"
              << std::endl;
}

int main(int argc, char **argv)
{
    // Default paths (relative to project root)
    std::string model_path = "models/yolov8s.onnx";
    std::string video_path = "data/videos/853889-hd_1920_1080_25fps.mp4";
    std::string output_path = "data/output/output.mp4";
    bool headless_mode = false;
    bool use_gpu = true;

    // Parse command-line arguments
    for (int i = 1; i < argc; i++)
    {
        std::string arg = argv[i];

        if (arg == "--help" || arg == "-h")
        {
            print_usage(argv[0]);
            return 0;
        }
        else if (arg == "--headless")
        {
            headless_mode = true;
        }
        else if (arg == "--cpu")
        {
            use_gpu = false;
        }
        else if (arg == "--model" && i + 1 < argc)
        {
            model_path = argv[++i];
        }
        else if (arg == "--input" && i + 1 < argc)
        {
            video_path = argv[++i];
        }
        else if (arg == "--output" && i + 1 < argc)
        {
            output_path = argv[++i];
        }
        else if (arg.substr(0, 2) == "--")
        {
            std::cerr << "Unknown option: " << arg << std::endl;
            std::cerr << "Use --help for usage information" << std::endl;
            return 1;
        }
    }

    if (headless_mode)
    {
        std::cout << "🖥️  Running in headless mode (no display, saving to file)" << std::endl;
    }

    std::cout << "📁 Model: " << model_path << std::endl;
    std::cout << "📹 Input: " << video_path << std::endl;
    std::cout << "💾 Output: " << output_path << std::endl;
    std::cout << "⚡ Using: " << (use_gpu ? "GPU" : "CPU") << std::endl;

    // Инициализация детектора
    std::cout << "\n🔄 Initializing Detector..." << std::endl;
    YOLODetector detector(model_path, use_gpu);
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

    // Настройка VideoWriter для headless режима
    cv::VideoWriter video_writer;
    if (headless_mode)
    {
        int frame_width = static_cast<int>(cap.get(cv::CAP_PROP_FRAME_WIDTH));
        int frame_height = static_cast<int>(cap.get(cv::CAP_PROP_FRAME_HEIGHT));
        double fps = cap.get(cv::CAP_PROP_FPS);
        if (fps <= 0)
            fps = 25.0; // Fallback FPS

        int fourcc = cv::VideoWriter::fourcc('m', 'p', '4', 'v');
        video_writer.open(output_path, fourcc, fps, cv::Size(frame_width, frame_height));

        if (!video_writer.isOpened())
        {
            std::cerr << "⚠️  Warning: Could not open video writer for " << output_path << std::endl;
            std::cerr << "   Output will not be saved." << std::endl;
        }
        else
        {
            std::cout << "📹 Output will be saved to: " << output_path << std::endl;
        }
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

        // 1. Детекция
        auto detections = detector.detect(frame, 0.5);

        // 2. Трекинг (превращаем просто боксы в объекты с ID)
        auto tracked_objects = tracker.update(detections);

        // 3. Логика подсчета и отрисовка
        const int line_tolerance = 20;    // Зона вокруг линии для детекции
        cv::Scalar line_color(0, 255, 0); // По умолчанию зеленая

        for (const auto &obj : tracked_objects)
        {
            // Рисуем бокс и ID
            cv::rectangle(frame, obj.box, cv::Scalar(0, 255, 0), 2);
            cv::putText(frame, "ID: " + std::to_string(obj.id),
                        cv::Point(obj.box.x, obj.box.y - 10),
                        cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0, 255, 0), 2);

            // Рисуем центральную точку
            cv::circle(frame, obj.center, 5, cv::Scalar(0, 255, 0), -1);

            // Логика пересечения
            if (obj.center.y > line_y - line_tolerance && obj.center.y < line_y + line_tolerance)
            {
                if (counted_ids.find(obj.id) == counted_ids.end())
                {
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
        cv::Point fps_position(frame.cols - text_size.width - 20, 40); // 20px padding from right edge
        cv::putText(frame, fps_text, fps_position,
                    cv::FONT_HERSHEY_SIMPLEX, 1, cv::Scalar(0, 0, 255), 2);

        // Print periodic statistics every 60 frames
        if (frame_count > 0 && frame_count % 60 == 0)
        {
            std::cout << "Frame " << frame_count << " — Avg FPS: " << avg_fps
                      << ", Instant FPS: " << instant_fps
                      << ", Count: " << counted_ids.size() << std::endl;
        }

        // Отображение или запись в зависимости от режима
        if (headless_mode)
        {
            // В headless режиме просто пишем в файл
            if (video_writer.isOpened())
            {
                video_writer.write(frame);
            }
        }
        else
        {
            // В обычном режиме показываем окно
            cv::imshow("C++ YOLOv8 Inference", frame);
            if (cv::waitKey(1) == 'q')
                break;
        }
    }

    // Освобождаем VideoWriter
    if (video_writer.isOpened())
    {
        video_writer.release();
        std::cout << "✅ Output saved to: " << output_path << std::endl;
    }

    // Print final summary
    std::cout << "\n--- Summary ---" << std::endl;
    std::cout << "Frames processed: " << fps_counter.getFrameCount() << std::endl;
    std::cout << "Average FPS: " << fps_counter.getAverageFPS() << std::endl;

    return 0;
}