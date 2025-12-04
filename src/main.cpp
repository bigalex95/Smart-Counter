#include <iostream>
#include <opencv2/opencv.hpp>
#include "detector.h"
#include "tracker.h"
#include "fps_counter.h"
#include <set>
#include "database.h"

void print_usage(const char *program_name)
{
    std::cout << "Usage: " << program_name << " [options]\n\n"
              << "Options:\n"
              << "  --model <path>      Path to ONNX model (default: models/yolov8s.onnx)\n"
              << "  --input <path>      Path to input video (default: data/videos/853889-hd_1920_1080_25fps.mp4)\n"
              << "  --output <path>     Path to output video (default: data/output/output.mp4)\n"
              << "  --db <path>         Path to SQLite database (default: logs/analytics.db)\n"
              << "  --headless          Run without display window (save to file only)\n"
              << "  --loop              Loop video infinitely (for camera-like streaming)\n"
              << "  --cpu               Use CPU only (default: GPU if available)\n"
              << "  --help              Show this help message\n"
              << "\nExamples:\n"
              << "  " << program_name << " --input video.mp4\n"
              << "  " << program_name << " --model models/yolov8n.onnx --headless --loop\n"
              << "  " << program_name << " --input video.mp4 --output result.mp4 --cpu\n"
              << "  " << program_name << " --db data_logs/analytics.db --loop\n"
              << std::endl;
}

int main(int argc, char **argv)
{
    // Default paths (relative to project root)
    std::string model_path = "models/yolov8s.onnx";
    std::string video_path = "data/videos/853889-hd_1920_1080_25fps.mp4";
    std::string output_path = "data/output/output.mp4";
    std::string db_path = "logs/analytics.db";
    bool headless_mode = false;
    bool loop_video = false;
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
        else if (arg == "--loop")
        {
            loop_video = true;
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
        else if (arg == "--db" && i + 1 < argc)
        {
            db_path = argv[++i];
        }
        else if (arg.substr(0, 2) == "--")
        {
            std::cerr << "Unknown option: " << arg << std::endl;
            std::cerr << "Use --help for usage information" << std::endl;
            return 1;
        }
    }

    // Initialize database with configured path
    Database db(db_path);
    db.init();

    if (headless_mode)
    {
        std::cout << "🖥️  Running in headless mode (no display, saving to file)" << std::endl;
    }

    std::cout << "📁 Model: " << model_path << std::endl;
    std::cout << "📹 Input: " << video_path << std::endl;
    std::cout << "💾 Output: " << output_path << std::endl;
    std::cout << "💿 Database: " << db_path << std::endl;
    std::cout << "🔁 Loop mode: " << (loop_video ? "enabled" : "disabled") << std::endl;
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

    // Узнаем FPS видео, чтобы проигрывать с правильной скоростью
    double video_fps = cap.get(cv::CAP_PROP_FPS);
    int delay_ms = 1000 / video_fps; // Например, 1000/25 = 40 мс

    std::set<int> counted_ids;
    int line_y = cap.get(cv::CAP_PROP_FRAME_HEIGHT) / 2; // Линия на середине кадра

    // Счетчики входа и выхода
    int count_in = 0;
    int count_out = 0;

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

    int last_saved_count = 0; // Чтобы не спамить в БД

    cv::Mat frame;
    while (true)
    {
        cap >> frame;
        // If video ended - restart from beginning (if loop enabled) or exit
        if (frame.empty())
        {
            if (loop_video)
            {
                std::cout << "🔁 Video ended, restarting from beginning..." << std::endl;
                cap.set(cv::CAP_PROP_POS_FRAMES, 0);
                cap >> frame;
                if (frame.empty())
                {
                    std::cerr << "❌ Error: Cannot restart video" << std::endl;
                    break;
                }
            }
            else
            {
                std::cout << "✅ Video processing completed" << std::endl;
                break;
            }
        }

        // Запуск детекции
        // Засекаем время для честного FPS
        auto start = std::chrono::high_resolution_clock::now();

        // 1. Детекция
        auto detections = detector.detect(frame, 0.5);

        // 2. Трекинг (превращаем просто боксы в объекты с ID)
        auto tracked_objects = tracker.update(detections);

        // 3. Логика двунаправленного подсчета
        cv::Scalar line_color(0, 255, 255); // По умолчанию желтая

        for (const auto &obj : tracked_objects)
        {
            // Рисуем бокс и ID
            cv::rectangle(frame, obj.box, cv::Scalar(0, 255, 0), 2);
            cv::putText(frame, "ID: " + std::to_string(obj.id),
                        cv::Point(obj.box.x, obj.box.y - 10),
                        cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0, 255, 0), 2);

            // Рисуем центральную точку
            cv::circle(frame, obj.center, 5, cv::Scalar(0, 255, 0), -1);

            // Логика векторного пересечения
            // Условие 1: Сейчас ниже линии, был выше (ВХОД / DOWN)
            if (obj.previous_center.y < line_y && obj.center.y >= line_y)
            {
                if (counted_ids.find(obj.id) == counted_ids.end())
                {
                    count_in++;
                    counted_ids.insert(obj.id);
                    line_color = cv::Scalar(0, 255, 0); // Зеленый миг
                }
            }

            // Условие 2: Сейчас выше линии, был ниже (ВЫХОД / UP)
            if (obj.previous_center.y > line_y && obj.center.y <= line_y)
            {
                if (counted_ids.find(obj.id) == counted_ids.end())
                {
                    count_out++;
                    counted_ids.insert(obj.id);
                    line_color = cv::Scalar(0, 0, 255); // Красный миг
                }
            }
        }

        // ЛОГИКА СОХРАНЕНИЯ
        int current_count = count_in + count_out;

        // Пишем в базу, только если счетчик увеличился
        if (current_count > last_saved_count)
        {
            db.insert_log(count_in, count_out);
            last_saved_count = current_count;
            std::cout << "📦 Data saved to DB: IN=" << count_in << " OUT=" << count_out << std::endl;
        }

        // Рисуем линию подсчета (цвет меняется при пересечении)
        cv::line(frame, cv::Point(0, line_y), cv::Point(frame.cols, line_y), line_color, 2);

        // Вычисляем занятость (сколько внутри)
        int occupancy = count_in - count_out;
        int corrected_occupancy = std::max(0, occupancy); // Защита от отрицательных значений

        // Рисуем информационную панель
        cv::rectangle(frame, cv::Point(0, 0), cv::Point(300, 140), cv::Scalar(0, 0, 0), -1);
        cv::putText(frame, "IN: " + std::to_string(count_in),
                    cv::Point(10, 40), cv::FONT_HERSHEY_SIMPLEX, 1, cv::Scalar(0, 255, 0), 2);
        cv::putText(frame, "OUT: " + std::to_string(count_out),
                    cv::Point(10, 80), cv::FONT_HERSHEY_SIMPLEX, 1, cv::Scalar(0, 0, 255), 2);

        // Показываем корректированное значение с предупреждением о дрейфе
        cv::Scalar occupancy_color = (occupancy < 0) ? cv::Scalar(0, 165, 255) : cv::Scalar(255, 255, 255);
        std::string occupancy_text = "INSIDE: " + std::to_string(corrected_occupancy);
        if (occupancy < 0)
        {
            occupancy_text += " (!" + std::to_string(occupancy) + ")";
        }
        cv::putText(frame, occupancy_text,
                    cv::Point(10, 120), cv::FONT_HERSHEY_SIMPLEX, 0.8, occupancy_color, 2);

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
                      << ", IN: " << count_in << ", OUT: " << count_out
                      << ", INSIDE: " << (count_in - count_out) << std::endl;
        }

        // Отображение или запись в зависимости от режима
        if (headless_mode)
        {
            // В headless режиме просто пишем в файл
            if (video_writer.isOpened())
            {
                video_writer.write(frame);
            }
            // Small delay to control processing speed and allow database writes
            cv::waitKey(1);
        }
        else
        {
            // В обычном режиме показываем окно
            cv::imshow("C++ YOLOv8 Inference", frame);
            if (cv::waitKey(delay_ms) == 'q')
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