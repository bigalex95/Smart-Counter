# Smart-Counter MLOps Deployment Guide

Complete guide for deploying Smart-Counter in production environments with GPU acceleration.

## 🚀 Quick Start

### Local Setup

```bash
# Clone and setup
git clone https://github.com/bigalex95/Smart-Counter.git
cd Smart-Counter

# Run automated setup (interactive)
./scripts/setup_mlops.sh

# Or non-interactive for servers
DEPLOY_MODE=server SKIP_INTERACTIVE=true ./scripts/setup_mlops.sh
```

### Docker Deployment

```bash
# Build image
docker build -t smart-counter-cpp .

# Run with GPU (recommended)
docker run --rm --gpus all \
  -v $(pwd)/data/output:/app/data/output \
  smart-counter-cpp

# Run CPU-only
docker run --rm \
  -v $(pwd)/data/output:/app/data/output \
  smart-counter-cpp
```

## 📋 Prerequisites

### System Requirements

- **OS**: Ubuntu 20.04+, Debian 11+, or compatible Linux
- **CPU**: Multi-core processor (4+ cores recommended)
- **RAM**: 8GB minimum, 16GB recommended
- **GPU**: NVIDIA GPU with CUDA support (optional but recommended)
- **Storage**: 10GB free space

### Software Dependencies

- **Docker**: 20.10+
- **NVIDIA Driver**: 525+ (for GPU support)
- **NVIDIA Container Toolkit**: Latest (for GPU in Docker)
- **CMake**: 3.10+
- **GCC/G++**: 9+
- **Python**: 3.8+

## 🔧 Installation

### Option 1: Automated Setup (Recommended)

The `setup_mlops.sh` script handles everything:

```bash
# Interactive mode (asks for confirmations)
./scripts/setup_mlops.sh

# Server mode (auto-installs with defaults)
DEPLOY_MODE=server SKIP_INTERACTIVE=true ./scripts/setup_mlops.sh

# Docker mode (prepares for containerization)
DEPLOY_MODE=docker ./scripts/setup_mlops.sh
```

### Option 2: Manual Setup

1. **Install System Dependencies**

```bash
# Ubuntu/Debian
sudo apt-get update
sudo apt-get install -y build-essential cmake git wget curl libopencv-dev

# Fedora/RHEL
sudo dnf install -y gcc-c++ cmake git wget curl opencv-devel
```

2. **Install CUDA (for GPU support)**

```bash
# Check if already installed
nvidia-smi

# Install from NVIDIA repository
# Follow: https://developer.nvidia.com/cuda-downloads
```

3. **Download Dependencies**

```bash
# ONNX Runtime GPU
wget https://github.com/microsoft/onnxruntime/releases/download/v1.23.2/onnxruntime-linux-x64-gpu-1.23.2.tgz
tar -xzf onnxruntime-linux-x64-gpu-1.23.2.tgz -C third_party/

# YOLOv8 Model
wget -O models/yolov8s.pt https://github.com/ultralytics/assets/releases/download/v8.0.0/yolov8s.pt
```

4. **Build Project**

```bash
mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
make -j$(nproc)
```

## 🐳 Docker Setup

### GPU Support Setup

1. **Install NVIDIA Container Toolkit**

```bash
# Add repository
curl -fsSL https://nvidia.github.io/libnvidia-container/gpgkey | \
  sudo gpg --dearmor -o /usr/share/keyrings/nvidia-container-toolkit-keyring.gpg

curl -s -L https://nvidia.github.io/libnvidia-container/stable/deb/nvidia-container-toolkit.list | \
  sed 's#deb https://#deb [signed-by=/usr/share/keyrings/nvidia-container-toolkit-keyring.gpg] https://#g' | \
  sudo tee /etc/apt/sources.list.d/nvidia-container-toolkit.list

# Install
sudo apt-get update
sudo apt-get install -y nvidia-container-toolkit

# Configure Docker
sudo nvidia-ctk runtime configure --runtime=docker
sudo systemctl restart docker
```

2. **Add User to Docker Group**

```bash
sudo usermod -aG docker $USER
# Log out and back in, or run:
newgrp docker
```

3. **Test GPU Access**

```bash
docker run --rm --gpus all nvidia/cuda:12.9.1-base-ubuntu22.04 nvidia-smi
```

### Using Deployment Script

```bash
# Make executable
chmod +x scripts/deploy.sh

# Build image
./scripts/deploy.sh build

# Run once with GPU
./scripts/deploy.sh run --gpu

# Deploy as persistent service
./scripts/deploy.sh deploy

# Check status
./scripts/deploy.sh status

# View logs
./scripts/deploy.sh logs

# Stop service
./scripts/deploy.sh stop

# Clean up
./scripts/deploy.sh clean
```

## 🎯 Usage Examples

### Processing Single Video

```bash
# Using Docker
docker run --rm --gpus all \
  -v /path/to/video.mp4:/app/data/videos/input.mp4 \
  -v $(pwd)/data/output:/app/data/output \
  smart-counter-cpp

# Using deployment script
./scripts/deploy.sh run --gpu --input /path/to/video.mp4
```

### Processing Video Directory

```bash
docker run --rm --gpus all \
  -v /path/to/videos:/app/data/videos \
  -v $(pwd)/data/output:/app/data/output \
  smart-counter-cpp
```

### Development Mode

```bash
# Mount source code for live development
./scripts/deploy.sh run --dev --gpu
```

## 📊 Performance

### Benchmark Results (RTX 3060 Laptop)

| Mode       | FPS  | Processing Time (341 frames) |
| ---------- | ---- | ---------------------------- |
| GPU (CUDA) | 58.4 | ~12 seconds                  |
| CPU Only   | 5.0  | ~78 seconds                  |

**Speedup: ~11.6x with GPU**

### Optimization Tips

1. **Use GPU**: Always use `--gpus all` flag for Docker
2. **Batch Processing**: Process multiple videos in sequence
3. **Model Size**: Use smaller models (yolov8n) for faster inference
4. **Input Resolution**: Lower resolution = faster processing
5. **Multi-threading**: Utilize all CPU cores with `-j$(nproc)` during build

## 🔍 Verification

### Check CUDA Setup

```bash
# Host system
./scripts/check_cuda.sh

# Inside Docker
docker run --rm --gpus all smart-counter-cpp bash -c "/app/scripts/check_cuda.sh"
```

Expected output:

```
✔ CUDA Toolkit Found: 12.9
✔ cuDNN Header Found: 9.10.2
✔ GPU Detected: NVIDIA GeForce RTX 3060 Laptop GPU
```

### Test Inference

```bash
# Should show "Model loaded with CUDA acceleration"
docker run --rm --gpus all smart-counter-cpp
```

## 🚨 Troubleshooting

### GPU Not Detected

**Issue**: `WARNING: The NVIDIA Driver was not detected`

**Solutions**:

1. Check driver: `nvidia-smi`
2. Verify Docker GPU access: `docker run --rm --gpus all nvidia/cuda:12.9.1-base-ubuntu22.04 nvidia-smi`
3. Reinstall Container Toolkit: See GPU Support Setup above
4. Switch to native Docker (not Docker Desktop):
   ```bash
   docker context use default
   ```

### CUDA Version Mismatch

**Issue**: `CUDA driver version is insufficient`

**Solution**: Update NVIDIA driver or use matching CUDA container version:

```bash
# Check your CUDA version
nvcc --version

# Use matching Docker image in Dockerfile
FROM nvidia/cuda:XX.X.X-cudnn-devel-ubuntu22.04
```

### Permission Denied (Docker)

**Issue**: `permission denied while trying to connect to docker socket`

**Solution**:

```bash
sudo usermod -aG docker $USER
newgrp docker
```

### Model Loading Fails

**Issue**: `Load model from models/yolov8s.onnx failed`

**Solutions**:

1. Ensure model exists: `ls -lh models/`
2. Convert model:
   ```bash
   source venv/bin/activate
   python python/convert.py
   ```
3. Download manually from releases

## 📁 Project Structure

```
Smart-Counter/
├── src/                    # C++ source code
├── include/                # Header files
├── models/                 # ML models (ONNX, PyTorch)
├── data/
│   ├── videos/            # Input videos
│   └── output/            # Processed results
├── scripts/
│   ├── setup_mlops.sh     # MLOps setup script
│   ├── deploy.sh          # Deployment automation
│   ├── check_cuda.sh      # CUDA verification
│   └── build.sh           # Local build script
├── third_party/           # ONNX Runtime libraries
├── Dockerfile             # Container definition
└── CMakeLists.txt         # Build configuration
```

## 🔐 Security Notes

- **Container Isolation**: Run containers with minimal privileges
- **Volume Mounts**: Mount only necessary directories
- **Network**: Use `--network none` if internet not needed
- **User**: Consider running as non-root user in production

## 📚 Advanced Topics

### CI/CD Integration

Example GitHub Actions workflow:

```yaml
name: Build and Deploy

on: [push]

jobs:
  build:
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v3
      - name: Build Docker image
        run: docker build -t smart-counter:${{ github.sha }} .
      - name: Push to registry
        run: docker push smart-counter:${{ github.sha }}
```

### Kubernetes Deployment

Coming soon: Helm charts and Kubernetes manifests for orchestrated deployments.

### Monitoring

Use Docker stats or Prometheus for monitoring:

```bash
docker stats smart-counter-runner
```

## 🤝 Contributing

See [CONTRIBUTING.md](CONTRIBUTING.md) for development setup and guidelines.

## 📄 License

See [LICENSE](LICENSE) file.

## 🆘 Support

- **Issues**: https://github.com/bigalex95/Smart-Counter/issues
- **Discussions**: https://github.com/bigalex95/Smart-Counter/discussions
