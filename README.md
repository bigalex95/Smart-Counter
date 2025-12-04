# 🚗 Smart-Counter: Edge AI Traffic Analytics

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![C++](https://img.shields.io/badge/C++-17-blue.svg)](https://isocpp.org/)
[![Python](https://img.shields.io/badge/Python-3.9+-blue.svg)](https://www.python.org/)
[![Docker](https://img.shields.io/badge/Docker-Ready-blue.svg)](https://www.docker.com/)

**High-performance people counting system with C++ (OpenCV, ONNX Runtime) + YOLOv8. Designed for edge devices with real-time analytics dashboard.**

Most "AI video projects" stop at a notebook demo. Smart-Counter is built for **production deployment** — optimized C++ inference engine, persistent analytics storage, and Dockerized deployment.

---

## 🚀 Key Features

- **⚡ Real-Time Detection** – YOLOv8 on GPU using ONNX Runtime (C++) achieving ~100 FPS on RTX 3060
- **🎯 Bi-Directional Counting** – Tracks both entry (IN) and exit (OUT) flows with virtual counting lines
- **💾 Data Persistence** – SQLite database logs all analytics with automatic drift protection
- **📊 Live Dashboard** – Streamlit-based real-time visualization with historical analytics
- **🐳 Fully Dockerized** – One-command deployment with Docker Compose (CPU/GPU support)
- **🔧 Production Ready** – Modular architecture, error handling, and comprehensive logging

---

## 🛠 Tech Stack

| Component           | Technology                                |
| ------------------- | ----------------------------------------- |
| **Core Engine**     | C++17, OpenCV 4.8+                        |
| **AI Inference**    | ONNX Runtime (CUDA Execution Provider)    |
| **Detection Model** | YOLOv8 (Ultralytics)                      |
| **Tracking**        | Custom Centroid Tracker with state memory |
| **Database**        | SQLite3 with analytics schema             |
| **Dashboard**       | Python 3.9+, Streamlit, Pandas            |
| **Build System**    | CMake 3.10+                               |
| **DevOps**          | Docker, Docker Compose                    |

---

## 🏗 Architecture

```
┌──────────────┐
│ Video Source │
└──────┬───────┘
       │
       ▼
┌─────────────────────────────────────────┐
│     C++ Detector (YOLO + Tracker)       │
│  ┌─────────┐  ┌─────────┐  ┌─────────┐ │
│  │  Detect │→ │  Track  │→ │  Count  │ │
│  └─────────┘  └─────────┘  └─────────┘ │
└──────────┬──────────────────────────────┘
           │
           ▼
    ┌─────────────┐
    │  SQLite DB  │  ← Persistent Analytics
    └──────┬──────┘
           │
           ▼
  ┌──────────────────┐
  │ Python Dashboard │  ← Real-time Visualization
  └──────────────────┘
```

**Detailed documentation:** [docs/ARCHITECTURE.md](docs/ARCHITECTURE.md) | [docs/BI_DIRECTIONAL_COUNTING.md](docs/BI_DIRECTIONAL_COUNTING.md)

---

## ⚡ Quick Start

### Prerequisites

- **Docker** & **Docker Compose**
- **NVIDIA GPU** + **NVIDIA Container Toolkit** (for GPU acceleration)
- **Linux** (Ubuntu 20.04+, or similar)

### 🐳 Docker Deployment (Recommended)

```bash
# 1. Clone repository
git clone https://github.com/bigalex95/Smart-Counter.git
cd Smart-Counter

# 2. Allow X11 forwarding (for visualization)
xhost +local:docker

# 3. Build and run
docker compose up --build
```

The dashboard will be available at **`http://localhost:8501`**

**GPU Support:** See [docs/DOCKER_COMPOSE_GPU.md](docs/DOCKER_COMPOSE_GPU.md)  
**Configuration:** Edit `.env` file or use environment variables

### 🔧 Native Build (Development)

For development or edge deployment without Docker:

```bash
# 1. Build C++ engine
./scripts/build.sh

# 2. Run detector
./build/SmartCounter --model models/yolov8s.onnx \
                     --input data/videos/video.mp4 \
                     --db logs/analytics.db

# Or use combined script
./scripts/build_and_run.sh
```

**Available scripts:**

- `build.sh` – Build C++ project
- `run.sh` – Run the detector
- `build_and_run.sh` – Build and run in one step
- `check_cuda.sh` – Check CUDA availability
- `test_database.sh` – Test database connection

See [docs/CLI_USAGE.md](docs/CLI_USAGE.md) for all CLI options.

---

## 📂 Project Structure

```
Smart-Counter/
├── src/                    # C++ source code (Detector, Tracker, Database)
│   ├── main.cpp            # Main application entry
│   ├── detector.cpp        # YOLO inference engine
│   ├── tracker.cpp         # Centroid tracking algorithm
│   └── database.cpp        # SQLite analytics logger
├── include/                # C++ headers
├── dashboard/              # Python Streamlit analytics dashboard
│   ├── app.py              # Real-time dashboard UI
│   └── Dockerfile          # Dashboard container
├── python/                 # Python utilities
│   ├── prototype.py        # Python prototype (testing)
│   └── convert.py          # ONNX model conversion
├── scripts/                # Build and deployment automation
├── models/                 # ONNX models (YOLOv8)
├── data/                   # Videos and output
├── logs/                   # SQLite database (analytics.db)
├── docs/                   # Comprehensive documentation
├── docker-compose.yml      # Multi-container orchestration
├── Dockerfile              # C++ backend container
└── CMakeLists.txt          # Build configuration
```

---

## 🎯 Usage Examples

### Docker Compose Deployment

```bash
# Run with custom video and settings
MODEL_PATH=models/yolov8s.onnx \
INPUT_VIDEO=data/videos/my_video.mp4 \
HEADLESS_MODE=true \
docker compose up
```

### Standalone C++ Detector

```bash
./build/SmartCounter \
  --model models/yolov8s.onnx \
  --input data/videos/video.mp4 \
  --output data/output/result.mp4 \
  --db logs/analytics.db \
  --headless \
  --loop
```

### Python Dashboard (Standalone)

```bash
cd dashboard
streamlit run app.py -- --db ../logs/analytics.db
```

### Python Prototype (Testing)

```bash
source venv/bin/activate
python python/prototype.py
```

---

## 🔧 Configuration

### Environment Variables (Docker)

```bash
MODEL_PATH=models/yolov8s.onnx     # Model path
INPUT_VIDEO=data/videos/video.mp4  # Input video
OUTPUT_VIDEO=data/output/out.mp4   # Output video
DB_PATH=logs/analytics.db          # Database path
HEADLESS_MODE=true                 # No GUI display
LOOP_VIDEO=true                    # Loop video playback
USE_CPU=false                      # Force CPU inference
```

### CLI Arguments (Native)

```bash
./build/SmartCounter --help

Options:
  --model PATH      Path to ONNX model
  --input PATH      Input video file
  --output PATH     Output video file (optional)
  --db PATH         SQLite database path
  --headless        Run without GUI
  --loop            Loop video playback
  --cpu             Use CPU instead of GPU
```

See [docs/CLI_USAGE.md](docs/CLI_USAGE.md) for advanced configuration.

---

## 📊 Performance Benchmarks

| Component           | Performance           |
| ------------------- | --------------------- |
| **Inference (GPU)** | ~100 FPS (YOLOv8s)    |
| **Full Pipeline**   | ~50-80 FPS            |
| **Latency**         | < 20ms per frame      |
| **Memory**          | ~2GB GPU / ~500MB CPU |

_Tested on NVIDIA GeForce RTX 3060 Laptop GPU with YOLOv8s_

### Model Comparison

| Model     | Speed      | Accuracy     | Recommended For        |
| --------- | ---------- | ------------ | ---------------------- |
| `yolov8n` | ⚡⚡⚡⚡⚡ | ⭐⭐⭐       | Edge devices, high FPS |
| `yolov8s` | ⚡⚡⚡⚡   | ⭐⭐⭐⭐     | **Balanced (default)** |
| `yolov8m` | ⚡⚡⚡     | ⭐⭐⭐⭐⭐   | Higher accuracy        |
| `yolov8l` | ⚡⚡       | ⭐⭐⭐⭐⭐⭐ | Maximum accuracy       |

---

## 🚀 Deployment Options

### 🐳 Docker (Production)

```bash
# CPU-only deployment
docker compose up

# GPU deployment
docker compose -f docker-compose-gpu.yml up
```

### 🌐 Cloud Platforms

- **AWS**: ECS/EKS with GPU instances
- **GCP**: Cloud Run / GKE with T4/V100
- **Azure**: Container Instances with GPU

### 🔌 Edge Devices

- **NVIDIA Jetson** (Nano, Xavier, Orin) – Optimized for edge AI
- **Intel NUC** – CPU inference mode
- **Custom hardware** – Via ONNX Runtime compatibility

See [docs/DEPLOYMENT.md](docs/DEPLOYMENT.md) for detailed deployment guides.

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

## 📚 Documentation

- **[Quick Start Guide](docs/QUICKSTART.md)** – Get running in 5 minutes
- **[Architecture Overview](docs/ARCHITECTURE.md)** – System design and data flow
- **[Bi-Directional Counting](docs/BI_DIRECTIONAL_COUNTING.md)** – How counting works
- **[Database Schema](docs/DATABASE.md)** – Analytics storage structure
- **[CLI Usage](docs/CLI_USAGE.md)** – Command-line reference
- **[Docker Compose Guide](docs/DOCKER_COMPOSE.md)** – Container deployment
- **[Deployment Guide](docs/DEPLOYMENT.md)** – Production deployment strategies
- **[Tech Stack Details](docs/TECH_STACK.md)** – Technology deep dive

---

## 🎯 What Makes This Different?

Most computer vision projects are:

- ❌ Python-only (slow, not production-ready)
- ❌ No tracking (just detection)
- ❌ No persistence (analytics lost on restart)
- ❌ No deployment story (hard to run)

**Smart-Counter is:**

- ✅ **Production C++** – Optimized for real-world performance
- ✅ **Complete Pipeline** – Detection → Tracking → Counting → Analytics
- ✅ **Data Persistence** – SQLite with automatic logging
- ✅ **Deploy Anywhere** – Docker, cloud, edge devices

---

## ✅ Current Implementation

**What's Working Now:**

- ✅ **C++ Inference Engine** – YOLOv8 ONNX Runtime with GPU/CPU support
- ✅ **Custom Centroid Tracker** – Simple, fast tracking algorithm
- ✅ **Bi-Directional Counting** – Tracks IN/OUT flows across counting line
- ✅ **SQLite Database** – Persistent analytics storage with drift protection
- ✅ **Streamlit Dashboard** – Real-time visualization and historical data
- ✅ **Docker Deployment** – Multi-container setup with docker-compose
- ✅ **CLI Interface** – Full command-line control with multiple options
- ✅ **Video Recording** – Output processed video with annotations

---

## 🚧 Roadmap (Coming Soon)

**Planned Improvements:**

- 🔜 **Advanced Tracking** – Replace simple tracker with BoTSORT/ByteTrack
- 🔜 **Multi-Zone Support** – Define multiple counting zones
- 🔜 **Heatmap Generation** – Visualize traffic patterns
- 🔜 **REST API** – HTTP API for integration with other systems
- 🔜 **WebSocket Streaming** – Real-time video feed to dashboard
- 🔜 **Model Optimization** – TensorRT support for even faster inference
- 🔜 **Multi-Camera Support** – Process multiple video streams
- 🔜 **Alert System** – Notifications for crowd thresholds
- 🔜 **Time-Series Analytics** – Advanced statistical analysis
- 🔜 **Cloud Storage Integration** – S3/GCS for video archival

**Contributions welcome!** See [Contributing](#-contributing) section.

---

**Built with ❤️ for production ML deployment**
