from ultralytics import YOLO

# Load model
model = YOLO("models/yolov8s.pt")

# Export to ONNX
model.export(
    format="onnx",
    opset=12,  # ONNX opset
    dynamic=True,  # dynamic batch/shape
    simplify=True,  # run onnx-simplifier
)
