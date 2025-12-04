# Docker Compose Usage Guide

This guide explains how to use Docker Compose to run Smart-Counter with configurable parameters.

## 🚀 Quick Start

```bash
# Start all services (detector + dashboard)
./run compose-up

# View logs
./run compose-logs

# Stop all services
./run compose-down
```

---

## 📋 Prerequisites

1. **Docker and Docker Compose** installed
2. **NVIDIA Docker runtime** (for GPU support)
3. **X11 forwarding** configured (for display mode)

```bash
# Allow X11 connections (if using display)
xhost +local:docker
```

---

## ⚙️ Configuration

### Method 1: Environment Variables (Recommended)

Create a `.env` file in the project root:

```bash
# Copy example configuration
cp .env.example .env

# Edit configuration
nano .env
```

Example `.env` file:

```bash
# Detector settings
MODEL_PATH=models/yolov8n.onnx
INPUT_VIDEO=data/videos/my_video.mp4
OUTPUT_VIDEO=data/output/result.mp4
DB_PATH=logs/analytics.db
HEADLESS_MODE=true
USE_CPU=false

# Dashboard settings
DASHBOARD_PORT=8501
REFRESH_INTERVAL=2
DATA_LIMIT=100
```

### Method 2: Inline Environment Variables

```bash
# Set variables before command
MODEL_PATH=models/yolov8n.onnx INPUT_VIDEO=data/videos/test.mp4 ./run compose-up
```

### Method 3: Modify docker-compose.yml

Edit `docker-compose.yml` directly to change default values.

---

## 🎯 Common Use Cases

### 1. Basic Usage (Default Settings)

```bash
# Start with defaults (yolov8s.onnx, default video, headless)
./run compose-up

# Check logs
./run compose-logs detector

# Access dashboard
open http://localhost:8501
```

### 2. Custom Video Processing

```bash
# Create .env file
cat > .env << EOF
MODEL_PATH=models/yolov8n.onnx
INPUT_VIDEO=data/videos/custom_video.mp4
OUTPUT_VIDEO=data/output/custom_result.mp4
HEADLESS_MODE=true
EOF

# Start services
./run compose-up
```

### 3. Display Mode (With X11)

```bash
# Allow X11 connections
xhost +local:docker

# Create .env file
cat > .env << EOF
HEADLESS_MODE=false
DISPLAY=${DISPLAY}
EOF

# Start services
./run compose-up

# View live detection window
```

### 4. CPU-Only Processing

```bash
# Create .env file
cat > .env << EOF
USE_CPU=true
MODEL_PATH=models/yolov8n.onnx
EOF

# Start services
./run compose-up
```

### 5. Custom Dashboard Port

```bash
# Use different port for dashboard
echo "DASHBOARD_PORT=8080" > .env

# Start services
./run compose-up

# Access at http://localhost:8080
```

### 6. High-Frequency Monitoring

```bash
# Create .env file for real-time monitoring
cat > .env << EOF
REFRESH_INTERVAL=1
DATA_LIMIT=500
DASHBOARD_PORT=8501
EOF

# Start services
./run compose-up
```

---

## 🔧 Service Management

### Start Services

```bash
# Start all services in background
./run compose-up

# Or using docker compose directly
docker compose up -d
```

### Stop Services

```bash
# Stop all services
./run compose-down

# Or using docker compose directly
docker compose down
```

### View Logs

```bash
# All services
./run compose-logs

# Specific service (detector or dashboard)
./run compose-logs detector
./run compose-logs dashboard

# Follow logs in real-time
docker compose logs -f
```

### Restart Services

```bash
# Restart all services
./run compose-restart

# Restart specific service
./run compose-restart detector
./run compose-restart dashboard
```

### Rebuild Services

```bash
# Rebuild all services
./run compose-build

# Rebuild and restart
./run compose-build && ./run compose-up
```

---

## 📊 Service Architecture

```
┌─────────────────────────────────────────────────────────┐
│                    Host Machine                          │
│                                                          │
│  ┌────────────────────────────────────────────────┐    │
│  │           Docker Compose Network                │    │
│  │                                                 │    │
│  │  ┌──────────────────┐    ┌──────────────────┐ │    │
│  │  │    Detector      │    │    Dashboard     │ │    │
│  │  │  (C++ Backend)   │    │  (Python/Streamlit)│ │    │
│  │  │                  │    │                  │ │    │
│  │  │  - YOLOv8 ONNX  │    │  - Web UI        │ │    │
│  │  │  - Object Track  │    │  - Real-time     │ │    │
│  │  │  - People Count  │    │    Charts        │ │    │
│  │  │  - SQLite DB     │    │  - Metrics       │ │    │
│  │  │                  │    │                  │ │    │
│  │  └────────┬─────────┘    └────────┬─────────┘ │    │
│  │           │                       │           │    │
│  │           └───────────┬───────────┘           │    │
│  │                       │                       │    │
│  │              ┌────────▼────────┐              │    │
│  │              │  Shared Volume  │              │    │
│  │              │  logs/analytics.db │            │    │
│  │              └─────────────────┘              │    │
│  └────────────────────────────────────────────────┘    │
│                       │                                │
│                       │ Port 8501                      │
└───────────────────────┼────────────────────────────────┘
                        │
                   Browser Access
                http://localhost:8501
```

---

## 🔍 Troubleshooting

### GPU Not Working

```bash
# Check NVIDIA Docker runtime
docker run --rm --gpus all nvidia/cuda:12.9.1-base-ubuntu22.04 nvidia-smi

# Check docker-compose GPU settings
docker compose config
```

### Display Not Showing

```bash
# Allow X11 connections
xhost +local:docker

# Check DISPLAY variable
echo $DISPLAY

# Set in .env file
echo "DISPLAY=${DISPLAY}" >> .env
```

### Database Not Updating

```bash
# Check detector logs
./run compose-logs detector

# Verify volume mounts
docker compose config

# Check file permissions
ls -la ./logs/
```

### Port Already in Use

```bash
# Change dashboard port in .env
echo "DASHBOARD_PORT=8080" > .env

# Restart services
./run compose-down
./run compose-up
```

### Services Not Starting

```bash
# Check service status
docker compose ps

# View full logs
docker compose logs

# Rebuild images
./run compose-build
./run compose-up
```

---

## 📝 Environment Variables Reference

### Detector Service

| Variable        | Default                                     | Description          |
| --------------- | ------------------------------------------- | -------------------- |
| `MODEL_PATH`    | `models/yolov8s.onnx`                       | Path to ONNX model   |
| `INPUT_VIDEO`   | `data/videos/853889-hd_1920_1080_25fps.mp4` | Input video path     |
| `OUTPUT_VIDEO`  | `data/output/output.mp4`                    | Output video path    |
| `DB_PATH`       | `logs/analytics.db`                         | Database path        |
| `HEADLESS_MODE` | `true`                                      | Run without display  |
| `USE_CPU`       | `false`                                     | Force CPU usage      |
| `DISPLAY`       | `:0`                                        | X11 display variable |

### Dashboard Service

| Variable            | Default                  | Description                |
| ------------------- | ------------------------ | -------------------------- |
| `DASHBOARD_PORT`    | `8501`                   | Dashboard port (host)      |
| `DASHBOARD_DB_PATH` | `/app/logs/analytics.db` | Database path              |
| `REFRESH_INTERVAL`  | `2`                      | Refresh interval (seconds) |
| `DATA_LIMIT`        | `100`                    | Max records to display     |

---

## 🎬 Complete Examples

### Example 1: Development Setup

```bash
# .env file
cat > .env << EOF
# Use smaller model for faster processing
MODEL_PATH=models/yolov8n.onnx
# Test video
INPUT_VIDEO=data/videos/test.mp4
# Headless mode
HEADLESS_MODE=true
# Fast dashboard updates
REFRESH_INTERVAL=1
EOF

./run compose-up
```

### Example 2: Production Setup

```bash
# .env file
cat > .env << EOF
# Production model
MODEL_PATH=models/yolov8s.onnx
# Production video source
INPUT_VIDEO=data/videos/production_feed.mp4
# Save output
OUTPUT_VIDEO=data/output/production_output.mp4
# Headless for server
HEADLESS_MODE=true
# Use GPU
USE_CPU=false
# Standard refresh
REFRESH_INTERVAL=2
# More data for analysis
DATA_LIMIT=500
EOF

./run compose-up
```

### Example 3: Debugging with Display

```bash
# Allow X11
xhost +local:docker

# .env file
cat > .env << EOF
# Small model for faster iteration
MODEL_PATH=models/yolov8n.onnx
# Test video
INPUT_VIDEO=data/videos/debug.mp4
# Show display for debugging
HEADLESS_MODE=false
DISPLAY=${DISPLAY}
EOF

./run compose-up
./run compose-logs detector
```

---

## 🔗 Related Documentation

- [CLI Usage Guide](CLI_USAGE.md) - Command-line arguments for all scripts
- [Deployment Guide](DEPLOYMENT.md) - Production deployment
- [Quick Start](QUICKSTART.md) - Getting started guide
- [Architecture](ARCHITECTURE.md) - System architecture

---

## 💡 Tips

1. **Always use `.env` file** for configuration instead of modifying `docker-compose.yml`
2. **Check logs first** when debugging: `./run compose-logs`
3. **Rebuild after code changes**: `./run compose-build`
4. **Use smaller models** (`yolov8n.onnx`) for faster testing
5. **Monitor resources**: `docker stats` to check CPU/GPU usage
6. **Clean up**: `./run compose-down` when not in use to free resources
