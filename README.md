# 🚀 Smart-Counter

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![C++](https://img.shields.io/badge/C++-17-blue.svg)](https://isocpp.org/)
[![Python](https://img.shields.io/badge/Python-3.9+-blue.svg)](https://www.python.org/)

**A production-ready, end-to-end real-time video analytics system for people counting and footfall analysis.**

Most "AI video projects" stop at a notebook demo. Smart-Counter is designed for **actual production deployment** on edge devices and servers, with a fully optimized C++ engine built for real-world performance.

---

## 📝 Overview

Smart-Counter is a comprehensive people-counting system that goes beyond simple detection. It provides:

- **Real-time video processing** with optimized C++ engine
- **Accurate people detection** using YOLO family models
- **Robust object tracking** with stable ID assignment across frames
- **Intelligent counting logic** with virtual lines, zones, and direction tracking
- **Analytics layer** for footfall metrics, occupancy, and heatmaps
- **Production deployment** ready for edge devices or cloud

---

## ✨ Key Features

### 🎥 Real-Time Engine (Built for Performance)

The system was prototyped in Python, then **rebuilt entirely in C++ with ONNX Runtime** for:

- ✅ **Real-time speed** on edge devices
- ✅ **Low latency** video processing
- ✅ **Reliable deployment** with minimal dependencies
- ✅ **GPU acceleration** support (CUDA)

### 🧠 Custom ML Pipeline

Inside the engine:

- **Video preprocessing** – Efficient decoding, resizing, and frame management
- **Object detection** – YOLOv8 for high-accuracy person detection
- **Object tracking** – Persistent ID assignment with BoTSORT/ByteTrack
- **Counting logic** – Virtual lines, zones, direction detection, and staff exclusion rules
- **Visualization** – Real-time annotated video output with metrics

All wrapped in a **clean, modular architecture** that's easy to extend.

### 📊 Analytics Layer

Beyond simple counting, Smart-Counter provides:

- **Footfall analytics** – Track people entering/exiting zones
- **Occupancy monitoring** – Real-time capacity tracking
- **Time-based metrics** – Peak hours, dwell time analysis
- **SQLite database** – Persistent storage with automatic logging (see [docs/DATABASE.md](docs/DATABASE.md))
- **Heatmaps** – Visualize high-traffic areas
- **Extensible hooks** – Add age/gender estimation or custom business rules

### 🚀 Deployable End-to-End

Designed with **MLOps principles** in mind:

- ✅ Runs on edge devices (Jetson, Raspberry Pi) or servers
- ✅ API-ready architecture
- ✅ Easy monitoring and logging
- ✅ Integration with dashboards and cloud analytics
- ✅ Fully open-source – deploy anywhere

---

## 🏗️ Architecture

```
┌─────────────┐
│    Video    │
└──────┬──────┘
       │
       ▼
┌─────────────────┐
│ 1. Preprocessing│  ← Decoding, resizing frames
└────────┬────────┘
         │
         ▼
┌─────────────────┐
│  2. Detection   │  ← YOLOv8 finds people (bounding boxes)
└────────┬────────┘
         │
         ▼
┌─────────────────┐
│  3. Tracking    │  ← Assigns IDs, tracks movement
└────────┬────────┘
         │
         ▼
┌─────────────────┐
│  4. Counting    │  ← Counts line crossings
└────────┬────────┘
         │
         ▼
┌─────────────────┐
│5. Visualization │  ← Draws results
└─────────────────┘
```

**For detailed architecture documentation, see [docs/ARCHITECTURE.md](docs/ARCHITECTURE.md)**

---

## 🛠️ Technology Stack

| Component            | Technology              |
| -------------------- | ----------------------- |
| **Core Engine**      | C++17 with ONNX Runtime |
| **Prototype**        | Python 3.9+             |
| **Detection Model**  | YOLOv8 (Ultralytics)    |
| **Tracking**         | BoTSORT / ByteTrack     |
| **Computer Vision**  | OpenCV 4.8+             |
| **Build System**     | CMake 3.10+             |
| **GPU Acceleration** | CUDA + cuDNN (optional) |

---

## ⚡ Quick Start

### Prerequisites

- **Linux** (Ubuntu 20.04+, Fedora, Arch, or similar)
- **CMake** 3.10+
- **OpenCV** 4.8+
- **Python** 3.9+ (for model conversion)
- **CUDA + cuDNN** (optional, for GPU acceleration)

### Installation

1. **Clone the repository**

```bash
git clone https://github.com/bigalex95/Smart-Counter.git
cd Smart-Counter
```

2. **Run the setup script** (automated installation)

```bash
./scripts/setup.sh
```

This script will:

- Install system dependencies
- Download ONNX Runtime (GPU or CPU version)
- Set up Python virtual environment
- Download YOLOv8 model
- Convert model to ONNX format

3. **Build the C++ engine**

```bash
./scripts/build.sh
```

4. **Run the application**

```bash
./scripts/run.sh
```

Or use the combined script:

```bash
./scripts/build_and_run.sh
```

---

## 📂 Project Structure

```
Smart-Counter/
├── src/              # C++ source code
│   ├── main.cpp      # Main application entry
│   └── detector.cpp  # YOLO detector implementation
├── include/          # C++ headers
│   └── detector.h    # Detector interface
├── python/           # Python prototype and utilities
│   ├── prototype.py  # Python-based people counter
│   └── convert.py    # Model conversion to ONNX
├── scripts/          # Build and deployment scripts
│   ├── setup.sh      # Automated setup
│   ├── build.sh      # Build C++ project
│   └── run.sh        # Run application
├── models/           # ML models (YOLO weights, ONNX)
├── data/             # Input videos and output results
│   ├── videos/       # Video files
│   └── output/       # Processed results
├── docs/             # Documentation
│   ├── ARCHITECTURE.md  # System architecture
│   └── TECH_STACK.md    # Technology details
└── CMakeLists.txt    # CMake build configuration
```

---

## 🎯 Usage

### Python Prototype (Quick Testing)

Perfect for rapid prototyping and testing:

```bash
source venv/bin/activate
python python/prototype.py
```

Features:

- YOLOv8 detection + tracking
- Line crossing counter
- Real-time FPS display
- Visual feedback

### C++ Engine (Production)

Optimized for deployment:

```bash
./build/SmartCounter
```

Features:

- High-performance ONNX inference
- GPU acceleration support
- Low memory footprint
- Production-ready

---

## 🔧 Configuration

### Model Selection

Choose the right YOLOv8 variant for your needs:

| Model     | Speed      | Accuracy     | Use Case                   |
| --------- | ---------- | ------------ | -------------------------- |
| `yolov8n` | ⚡⚡⚡⚡⚡ | ⭐⭐⭐       | Edge devices, high FPS     |
| `yolov8s` | ⚡⚡⚡⚡   | ⭐⭐⭐⭐     | **Recommended** (balanced) |
| `yolov8m` | ⚡⚡⚡     | ⭐⭐⭐⭐⭐   | More accuracy needed       |
| `yolov8l` | ⚡⚡       | ⭐⭐⭐⭐⭐⭐ | High accuracy priority     |
| `yolov8x` | ⚡         | ⭐⭐⭐⭐⭐⭐ | Maximum accuracy           |

### Counting Line Setup

Modify the counting line position in `src/main.cpp` or `python/prototype.py`:

```cpp
int line_y = frame_height / 2;  // Horizontal line at 50%
int line_tolerance = 20;         // Detection zone
```

---

## 📊 Performance

### Python Prototype

- **Model FPS**: ~30-60 FPS (depending on hardware)
- **System FPS**: ~25-45 FPS (full pipeline)

### C++ Engine

- **Inference**: ~60-100+ FPS on GPU
- **Full Pipeline**: ~50-80 FPS
- **Latency**: < 20ms per frame

_Benchmarks on NVIDIA GeForce RTX 3060 Laptop GPU with YOLOv8s_

---

## 🚀 Deployment

### Edge Devices

Smart-Counter can run on:

- **NVIDIA Jetson** (Nano, Xavier, Orin)
- **Raspberry Pi 4** (with optimization)
- **Edge servers** (Intel NUC, etc.)

### Cloud Deployment

- Containerize with Docker
- Deploy on AWS, GCP, Azure
- Use Kubernetes for scaling
- Integrate with cloud analytics platforms

### API Integration

The system is designed to be API-ready:

- RESTful API for video streams
- WebSocket for real-time updates
- gRPC for high-performance communication

---

## 🤝 Contributing

Contributions are welcome! Please feel free to submit a Pull Request.

### Development Setup

1. Fork the repository
2. Create your feature branch (`git checkout -b feature/amazing-feature`)
3. Commit your changes (`git commit -m 'Add amazing feature'`)
4. Push to the branch (`git push origin feature/amazing-feature`)
5. Open a Pull Request

---

## 📄 License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

---

## 🙏 Acknowledgments

- **Ultralytics** for YOLOv8
- **Microsoft** for ONNX Runtime
- **OpenCV** community
- All open-source contributors

---

## 📫 Contact

**Alibek Erkabayev** - [@bigalex95](https://github.com/bigalex95)

Project Link: [https://github.com/bigalex95/Smart-Counter](https://github.com/bigalex95/Smart-Counter)

---

## 🎓 Learn More

- [Architecture Documentation](docs/ARCHITECTURE.md) – System design and data flow
- [Technology Stack](docs/TECH_STACK.md) – Detailed tech specs and resources

---

**Built with ❤️ for production ML deployment**
