# CLI Usage Guide

All Python scripts have been refactored to use command-line arguments instead of hardcoded paths. This guide shows how to use each script with custom configurations.

## 🎯 Overview

All scripts now follow the same coding style as `main.cpp` and `read_database.py`:

- Command-line arguments with sensible defaults
- `--help` flag for usage information
- Consistent argument naming across all scripts

---

## 📊 Dashboard (`dashboard/app.py`)

Run the Streamlit dashboard with custom configuration:

```bash
# Basic usage (uses defaults)
streamlit run dashboard/app.py

# Custom database path
streamlit run dashboard/app.py -- --db path/to/analytics.db

# Custom refresh interval and data limit
streamlit run dashboard/app.py -- --db logs/analytics.db --refresh 5 --limit 200

# All options
streamlit run dashboard/app.py -- --db <path> --refresh <seconds> --limit <records>
```

**Arguments:**

- `--db`: Path to SQLite database (default: `../logs/analytics.db` or `DB_PATH` env var)
- `--refresh`: Refresh interval in seconds (default: 2)
- `--limit`: Maximum number of records to display (default: 100)

**Note:** When using Streamlit, you need `--` before your custom arguments.

---

## 🎥 Prototype (`python/prototype.py`)

Run the Python prototype with YOLOv8:

```bash
# Basic usage (uses defaults)
python python/prototype.py

# Show help
python python/prototype.py --help

# Custom model and video
python python/prototype.py --model models/yolov8n.pt --input data/videos/my_video.mp4

# Custom counting line position (0.3 = 30% from top)
python python/prototype.py --line-position 0.3

# Custom tolerance zone around counting line
python python/prototype.py --tolerance 30

# Print stats every 30 frames instead of 60
python python/prototype.py --stats-interval 30

# All options combined
python python/prototype.py \
    --model models/yolov8n.pt \
    --input data/videos/custom.mp4 \
    --line-position 0.4 \
    --tolerance 25 \
    --stats-interval 30
```

**Arguments:**

- `--model`: Path to YOLO model file (default: `models/yolov8s.pt`)
- `--input`: Path to input video file (default: `data/videos/853889-hd_1920_1080_25fps.mp4`)
- `--line-position`: Counting line position as fraction of frame height (default: 0.5 for middle)
- `--tolerance`: Detection zone tolerance around counting line in pixels (default: 20)
- `--stats-interval`: Print statistics every N frames (default: 60)

---

## 🔄 Model Conversion (`python/convert.py`)

Convert PyTorch models to ONNX format:

```bash
# Basic usage (converts yolov8s.pt to yolov8s.onnx)
python python/convert.py

# Show help
python python/convert.py --help

# Convert specific model
python python/convert.py --input models/yolov8n.pt

# Convert with custom output path
python python/convert.py --input models/yolov8n.pt --output models/custom_name.onnx

# Use different ONNX opset version
python python/convert.py --input models/yolov8s.pt --opset 13

# Disable dynamic shapes
python python/convert.py --input models/yolov8s.pt --no-dynamic

# Skip ONNX simplifier
python python/convert.py --input models/yolov8s.pt --no-simplify

# All options
python python/convert.py \
    --input models/yolov8n.pt \
    --output models/yolov8n_custom.onnx \
    --opset 13 \
    --no-simplify
```

**Arguments:**

- `--input`: Path to input PyTorch model file (default: `models/yolov8s.pt`)
- `--output`: Path to output ONNX model file (default: same as input with `.onnx` extension)
- `--opset`: ONNX opset version (default: 12)
- `--dynamic`: Enable dynamic batch/shape support (default: enabled)
- `--no-dynamic`: Disable dynamic batch/shape support
- `--simplify`: Run onnx-simplifier (default: enabled)
- `--no-simplify`: Skip onnx-simplifier

---

## ✅ Model Verification (`python/verify_onnx.py`)

Verify ONNX model validity:

```bash
# Basic usage (checks default model)
python python/verify_onnx.py

# Show help
python python/verify_onnx.py --help

# Check specific model
python python/verify_onnx.py models/yolov8n.onnx

# Verbose output with detailed model info
python python/verify_onnx.py models/yolov8s.onnx --verbose
```

**Arguments:**

- `model`: Path to ONNX model file (positional, default: `models/yolov8s.onnx`)
- `--verbose`: Print detailed model information

---

## 💾 Database Reader (`python/read_database.py`)

Read and analyze the SQLite database:

```bash
# Basic usage (interactive mode)
python python/read_database.py

# Show help
python python/read_database.py --help

# Custom database path
python python/read_database.py --db path/to/analytics.db

# Automatically export to CSV
python python/read_database.py --export

# Export with custom filename
python python/read_database.py --export --export-file my_data.csv

# All options
python python/read_database.py \
    --db logs/analytics.db \
    --export \
    --export-file results.csv
```

**Arguments:**

- `--db`: Path to SQLite database (default: `logs/analytics.db`)
- `--export`: Automatically export data to CSV without prompting
- `--export-file`: CSV export filename (default: `export.csv`)

---

## 🖥️ C++ Application (`build/SmartCounter`)

The C++ application already supports command-line arguments:

```bash
# Show help
./build/SmartCounter --help

# Custom model and video
./build/SmartCounter --model models/yolov8n.onnx --input data/videos/my_video.mp4

# Headless mode (no GUI, save to file)
./build/SmartCounter --headless --output data/output/result.mp4

# Use CPU instead of GPU
./build/SmartCounter --cpu

# Custom database path
./build/SmartCounter --db logs/custom_analytics.db

# All options combined
./build/SmartCounter \
    --model models/yolov8n.onnx \
    --input data/videos/custom.mp4 \
    --output data/output/result.mp4 \
    --db logs/analytics.db \
    --headless \
    --cpu
```

**Arguments:**

- `--model`: Path to ONNX model (default: `models/yolov8s.onnx`)
- `--input`: Path to input video (default: `data/videos/853889-hd_1920_1080_25fps.mp4`)
- `--output`: Path to output video (default: `data/output/output.mp4`)
- `--db`: Path to SQLite database (default: `logs/analytics.db`)
- `--headless`: Run without display window (save to file only)
- `--cpu`: Use CPU only (default: GPU if available)
- `--help`: Show help message

---

## 📝 Environment Variables

Some scripts also support environment variables as fallback:

### Dashboard

- `DB_PATH`: Database path (overridden by `--db` argument)

### All Scripts

You can also set paths via environment variables:

```bash
# Set database path
export DB_PATH=/path/to/analytics.db

# Run dashboard (will use DB_PATH if --db not specified)
streamlit run dashboard/app.py
```

---

## 🎨 Coding Style Notes

All refactored scripts follow these conventions:

1. **Argument Parsing**: Using `argparse` with descriptive help messages
2. **Default Values**: Sensible defaults matching the original hardcoded paths
3. **Docstrings**: Functions have clear docstrings
4. **Main Function**: Scripts use `if __name__ == "__main__":` pattern
5. **Exit Codes**: Scripts return proper exit codes (0 for success, 1 for errors)
6. **Print Statements**: Emoji prefixes for visual clarity (🔄, ✅, ❌, etc.)
7. **Error Handling**: Try-except blocks with user-friendly error messages

---

## 🚀 Quick Examples

### Example 1: Process custom video with smaller model

```bash
# Convert model if needed
python python/convert.py --input models/yolov8n.pt

# Run with C++
./build/SmartCounter \
    --model models/yolov8n.onnx \
    --input path/to/custom_video.mp4 \
    --output data/output/custom_result.mp4
```

### Example 2: Test prototype with custom line position

```bash
python python/prototype.py \
    --input data/videos/test.mp4 \
    --line-position 0.7 \
    --tolerance 15
```

### Example 3: Run in headless mode and analyze results

```bash
# Run counter
./build/SmartCounter --headless --db logs/test_analytics.db

# Analyze database
python python/read_database.py --db logs/test_analytics.db --export

# View in dashboard
streamlit run dashboard/app.py -- --db logs/test_analytics.db
```

---

## 📚 Additional Resources

- Main documentation: `README.md`
- Architecture details: `docs/ARCHITECTURE.md`
- Database guide: `docs/DATABASE.md`
- Deployment: `docs/DEPLOYMENT.md`
- Quick start: `docs/QUICKSTART.md`
