#include "detector.h"
#include <iostream>

// Используем пространство имен для удобства
using namespace cv;
using namespace std;
using namespace Ort;

YOLODetector::YOLODetector(const std::string &model_path, bool use_cuda)
{
    // 1. Настройка окружения
    env = Env(ORT_LOGGING_LEVEL_WARNING, "YOLODetector");
    session_options = SessionOptions();

    // 2. Подключение CUDA (если есть)
    if (use_cuda)
    {
        // Пытаемся подключить CUDA провайдер
        // В новых версиях API это делается через OrtSessionOptionsAppendExecutionProvider_CUDA
        // Но C++ API обертка делает это проще (если собрана правильно):
        try
        {
            OrtCUDAProviderOptions cuda_options;
            session_options.AppendExecutionProvider_CUDA(cuda_options);
            cout << "✅ CUDA provider enabled." << endl;
        }
        catch (const std::exception &e)
        {
            cerr << "⚠️ Failed to enable CUDA: " << e.what() << endl;
            cout << "⚠️ Using CPU fallback." << endl;
        }
    }

    // 3. Загрузка модели
    session = Session(env, model_path.c_str(), session_options);

    // 4. Получение информации о входах и выходах
    // (Упрощенно берем 0-й вход и 0-й выход, так как у YOLOv8 их по одному)

    // ВАЖНО: В реальном коде имена нужно копировать, так как GetInputName возвращает умный указатель
    // Здесь мы используем аллокатор для получения имен
    AllocatorWithDefaultOptions allocator;

    // Получаем имя входа (обычно "images")
    auto input_name_ptr = session.GetInputNameAllocated(0, allocator);
    input_names.push_back(strdup(input_name_ptr.get())); // Копируем строку

    // Получаем имя выхода (обычно "output0")
    auto output_name_ptr = session.GetOutputNameAllocated(0, allocator);
    output_names.push_back(strdup(output_name_ptr.get()));

    // Получаем размер входа (обычно [1, 3, 640, 640])
    auto input_type_info = session.GetInputTypeInfo(0);
    auto input_tensor_info = input_type_info.GetTensorTypeAndShapeInfo();
    input_shape = input_tensor_info.GetShape();

    // Если размер динамический (-1), фиксируем его
    for (size_t i = 0; i < input_shape.size(); i++)
    {
        if (input_shape[i] == -1)
        {
            if (i == 0)
                input_shape[i] = 1; // batch size
            else if (i == 1)
                input_shape[i] = 3; // channels
            else
                input_shape[i] = 640; // height/width
        }
    }

    cout << "Model loaded: Input shape [" << input_shape[2] << "x" << input_shape[3] << "]" << endl;
}

vector<Detection> YOLODetector::detect(Mat &image, float conf_threshold)
{
    vector<Detection> detections;

    // 1. Подготовка изображения (Preprocess)
    // Цель: [1, 3, 640, 640] float32 tensor
    int input_w = input_shape[3];
    int input_h = input_shape[2];

    Mat blob;
    // blobFromImage делает: Resize, BGR->RGB, Normalize (1/255), HWC->CHW
    cv::dnn::blobFromImage(image, blob, 1.0 / 255.0, Size(input_w, input_h), Scalar(), true, false);

    // 2. Создание тензора
    // Данные в blob уже лежат плоско (contiguous), можно передавать в ONNX Runtime
    size_t input_tensor_size = input_shape[0] * input_shape[1] * input_shape[2] * input_shape[3];
    Value input_tensor = Value::CreateTensor<float>(
        MemoryInfo::CreateCpu(OrtArenaAllocator, OrtMemTypeDefault),
        (float *)blob.data, input_tensor_size, input_shape.data(), input_shape.size());

    // 3. Инференс (Run) 🚀
    auto output_tensors = session.Run(
        RunOptions{nullptr},
        input_names.data(), &input_tensor, 1,
        output_names.data(), 1);

    // 4. Разбор ответа (Postprocess)
    // YOLOv8 Output shape: [1, 84, 8400] -> [Batch, (4 coords + 80 classes), NumAnchors]
    float *raw_output = output_tensors[0].GetTensorMutableData<float>();

    // Получаем размеры выхода
    auto output_info = output_tensors[0].GetTensorTypeAndShapeInfo();
    auto output_dims = output_info.GetShape(); // [1, 84, 8400]

    // Debug: Print output shape
    cout << "Output shape: [";
    for (size_t i = 0; i < output_dims.size(); i++)
    {
        cout << output_dims[i];
        if (i < output_dims.size() - 1)
            cout << ", ";
    }
    cout << "]" << endl;

    int num_classes = output_dims[1] - 4; // 84 - 4 = 80
    int num_anchors = output_dims[2];     // 8400

    // Вектора для NMS (Non-Maximum Suppression)
    vector<int> class_ids;
    vector<float> confidences;
    vector<Rect> boxes;

    // Считаем коэффициент масштабирования, чтобы вернуть боксы к размеру оригинала
    float x_factor = (float)image.cols / input_w;
    float y_factor = (float)image.rows / input_h;

    // YOLOv8 output is transposed compared to v5/v7 usually.
    // It's [Channels, Anchors]. We loop through anchors (columns).

    // Данные лежат последовательно, но матрица [84, 8400].
    // Значит шаг между атрибутами одного анкора = 8400.
    // data[0][i] = x, data[1][i] = y ...

    for (int i = 0; i < num_anchors; i++)
    {
        // Ищем класс с максимальной уверенностью
        float max_score = 0.0;
        int max_class_id = -1;

        // Пробегаем по классам (начинаются с 4-го ряда)
        for (int c = 0; c < num_classes; c++)
        {
            float score = raw_output[(4 + c) * num_anchors + i];
            if (score > max_score)
            {
                max_score = score;
                max_class_id = c;
            }
        }

        if (max_score > conf_threshold)
        {
            // Извлекаем координаты
            float cx = raw_output[0 * num_anchors + i];
            float cy = raw_output[1 * num_anchors + i];
            float w = raw_output[2 * num_anchors + i];
            float h = raw_output[3 * num_anchors + i];

            // Переводим из центра в левый верхний угол и масштабируем
            int left = int((cx - 0.5 * w) * x_factor);
            int top = int((cy - 0.5 * h) * y_factor);
            int width = int(w * x_factor);
            int height = int(h * y_factor);

            boxes.push_back(Rect(left, top, width, height));
            confidences.push_back(max_score);
            class_ids.push_back(max_class_id);
        }
    }

    // 5. NMS (Убираем дубликаты)
    vector<int> nms_result;
    cv::dnn::NMSBoxes(boxes, confidences, conf_threshold, 0.45, nms_result);

    for (int idx : nms_result)
    {
        Detection result;
        result.class_id = class_ids[idx];
        result.confidence = confidences[idx];
        result.box = boxes[idx];
        detections.push_back(result);
    }

    return detections;
}