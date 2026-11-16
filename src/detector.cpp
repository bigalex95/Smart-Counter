#include "detector.h"
#include <iostream>

// Use the namespace for convenience
using namespace cv;
using namespace std;
using namespace Ort;

YOLODetector::YOLODetector(const std::string &model_path, bool use_cuda)
{
    // 1. Environment setup
    env = Env(ORT_LOGGING_LEVEL_WARNING, "YOLODetector");
    session_options = SessionOptions();

    // 2. CUDA setup (if available)
    bool cuda_enabled = false;
    if (use_cuda)
    {
        // Try to enable the CUDA provider
        // In newer API versions this is done via OrtSessionOptionsAppendExecutionProvider_CUDA
        // But the C++ API wrapper makes this easier (if built correctly):
        try
        {
            OrtCUDAProviderOptions cuda_options;
            session_options.AppendExecutionProvider_CUDA(cuda_options);
            cout << "✅ CUDA provider enabled." << endl;
            cuda_enabled = true;
        }
        catch (const std::exception &e)
        {
            cerr << "⚠️ Failed to enable CUDA provider: " << e.what() << endl;
            cout << "⚠️ Will try CPU fallback." << endl;
        }
    }

    // 3. Load the model, handling CUDA errors
    try
    {
        session = Session(env, model_path.c_str(), session_options);
        if (cuda_enabled)
        {
            cout << "✅ Model loaded with CUDA acceleration." << endl;
        }
        else
        {
            cout << "✅ Model loaded with CPU inference." << endl;
        }
    }
    catch (const std::exception &e)
    {
        // If CUDA was enabled but initialization failed, fall back to CPU
        if (cuda_enabled)
        {
            cerr << "⚠️ CUDA initialization failed: " << e.what() << endl;
            cout << "🔄 Retrying with CPU-only mode..." << endl;

            // Recreate session_options without CUDA
            session_options = SessionOptions();
            session = Session(env, model_path.c_str(), session_options);
            cout << "✅ Model loaded with CPU inference (fallback)." << endl;
        }
        else
        {
            // If the error is not CUDA-related, rethrow it
            throw;
        }
    }

    // 4. Get input and output information
    // (Simplified: take input 0 and output 0, since YOLOv8 has one of each)

    // IMPORTANT: In real code the names must be copied, since GetInputName returns a smart pointer
    // Here we use an allocator to get the names
    AllocatorWithDefaultOptions allocator;

    // Get the input name (usually "images")
    auto input_name_ptr = session.GetInputNameAllocated(0, allocator);
    input_names.push_back(strdup(input_name_ptr.get())); // Copy the string

    // Get the output name (usually "output0")
    auto output_name_ptr = session.GetOutputNameAllocated(0, allocator);
    output_names.push_back(strdup(output_name_ptr.get()));

    // Get the input size (usually [1, 3, 640, 640])
    auto input_type_info = session.GetInputTypeInfo(0);
    auto input_tensor_info = input_type_info.GetTensorTypeAndShapeInfo();
    input_shape = input_tensor_info.GetShape();

    // If the size is dynamic (-1), fix it
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

    // 1. Image preprocessing
    // Goal: [1, 3, 640, 640] float32 tensor
    int input_w = input_shape[3];
    int input_h = input_shape[2];

    Mat blob;
    // blobFromImage does: Resize, BGR->RGB, Normalize (1/255), HWC->CHW
    cv::dnn::blobFromImage(image, blob, 1.0 / 255.0, Size(input_w, input_h), Scalar(), true, false);

    // 2. Create the tensor
    // The blob data is already contiguous, so it can be passed to ONNX Runtime directly
    size_t input_tensor_size = input_shape[0] * input_shape[1] * input_shape[2] * input_shape[3];
    Value input_tensor = Value::CreateTensor<float>(
        MemoryInfo::CreateCpu(OrtArenaAllocator, OrtMemTypeDefault),
        (float *)blob.data, input_tensor_size, input_shape.data(), input_shape.size());

    // 3. Inference (Run)
    auto output_tensors = session.Run(
        RunOptions{nullptr},
        input_names.data(), &input_tensor, 1,
        output_names.data(), 1);

    // 4. Parse the output (Postprocess)
    // YOLOv8 Output shape: [1, 84, 8400] -> [Batch, (4 coords + 80 classes), NumAnchors]
    float *raw_output = output_tensors[0].GetTensorMutableData<float>();

    // Get the output dimensions
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

    // Vectors for NMS (Non-Maximum Suppression)
    vector<int> class_ids;
    vector<float> confidences;
    vector<Rect> boxes;

    // Compute the scale factor to map boxes back to the original size
    float x_factor = (float)image.cols / input_w;
    float y_factor = (float)image.rows / input_h;

    // YOLOv8 output is transposed compared to v5/v7 usually.
    // It's [Channels, Anchors]. We loop through anchors (columns).

    // Data is stored sequentially, but the matrix is [84, 8400].
    // So the stride between attributes of one anchor = 8400.
    // data[0][i] = x, data[1][i] = y ...

    for (int i = 0; i < num_anchors; i++)
    {
        // Find the class with the highest confidence
        float max_score = 0.0;
        int max_class_id = -1;

        // Iterate over classes (starting from row 4)
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
            // Extract coordinates
            float cx = raw_output[0 * num_anchors + i];
            float cy = raw_output[1 * num_anchors + i];
            float w = raw_output[2 * num_anchors + i];
            float h = raw_output[3 * num_anchors + i];

            // Convert from center to top-left corner and scale
            int left = int((cx - 0.5 * w) * x_factor);
            int top = int((cy - 0.5 * h) * y_factor);
            int width = int(w * x_factor);
            int height = int(h * y_factor);

            boxes.push_back(Rect(left, top, width, height));
            confidences.push_back(max_score);
            class_ids.push_back(max_class_id);
        }
    }

    // 5. NMS (remove duplicates)
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