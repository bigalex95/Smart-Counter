# 🚀 Smart-Counter - Quick Reference

## One-Command Deploy (Server)

```bash
git clone https://github.com/bigalex95/Smart-Counter.git
cd Smart-Counter
./run setup-server && ./run build-docker && ./run deploy
```

## Common Commands

### Local Development

```bash
./run setup          # Initial setup
./run build          # Build project
./run run            # Run application
./run check-cuda     # Verify GPU
```

### Docker Operations

```bash
./run build-docker   # Build image
./run run-docker     # Run with GPU
./run deploy         # Deploy service
./run status         # Check status
./run logs           # View logs
./run stop           # Stop service
```

### Manual Docker Commands

```bash
# Build
docker build -t smart-counter-cpp .

# Run with GPU
docker run --rm --gpus all -v $(pwd)/data/output:/app/data/output smart-counter-cpp

# Run CPU only
docker run --rm -v $(pwd)/data/output:/app/data/output smart-counter-cpp

# Custom video
docker run --rm --gpus all \
  -v /path/to/video.mp4:/app/data/videos/input.mp4 \
  -v $(pwd)/data/output:/app/data/output \
  smart-counter-cpp
```

## GPU Setup (One-time)

```bash
# Install NVIDIA Container Toolkit
curl -fsSL https://nvidia.github.io/libnvidia-container/gpgkey | \
  sudo gpg --dearmor -o /usr/share/keyrings/nvidia-container-toolkit-keyring.gpg

curl -s -L https://nvidia.github.io/libnvidia-container/stable/deb/nvidia-container-toolkit.list | \
  sed 's#deb https://#deb [signed-by=/usr/share/keyrings/nvidia-container-toolkit-keyring.gpg] https://#g' | \
  sudo tee /etc/apt/sources.list.d/nvidia-container-toolkit.list

sudo apt-get update && sudo apt-get install -y nvidia-container-toolkit
sudo nvidia-ctk runtime configure --runtime=docker
sudo systemctl restart docker

# Add user to docker group
sudo usermod -aG docker $USER
newgrp docker  # or log out/in
```

## Performance

| Mode | FPS | Time (341 frames) |
| ---- | --- | ----------------- |
| GPU  | 58  | 12 seconds        |
| CPU  | 5   | 78 seconds        |

**11.6x speedup with GPU!**

## Troubleshooting

### GPU not detected

```bash
# Check driver
nvidia-smi

# Check Docker GPU access
docker run --rm --gpus all nvidia/cuda:12.9.1-base-ubuntu22.04 nvidia-smi

# Use native Docker (not Desktop)
docker context use default
```

### Permission denied

```bash
sudo usermod -aG docker $USER
newgrp docker
```

### Model not found

```bash
# Rerun setup
./run setup
```

## Files Structure

```
Smart-Counter/
├── run                    # Task runner (./run help)
├── Dockerfile            # Container definition
├── DEPLOYMENT.md         # Full deployment guide
├── scripts/
│   ├── setup_mlops.sh   # MLOps setup
│   ├── deploy.sh        # Deployment automation
│   ├── check_cuda.sh    # GPU verification
│   ├── build.sh         # Local build
│   └── run.sh           # Local run
├── src/                 # C++ source
├── include/             # Headers
├── models/              # ML models
├── data/
│   ├── videos/         # Input (place videos here)
│   └── output/         # Results (generated)
└── third_party/        # Dependencies
```

## Environment Variables

```bash
# Deployment mode
DEPLOY_MODE=server ./run setup    # server, docker, local

# Skip interactive prompts
SKIP_INTERACTIVE=true ./run setup

# Custom image/container names
IMAGE_NAME=my-counter IMAGE_TAG=v1.0 ./run build-docker
CONTAINER_NAME=my-runner ./run deploy
```

## Advanced

### Development Mode (live source mount)

```bash
./scripts/deploy.sh run --dev
```

### Custom Input/Output

```bash
./scripts/deploy.sh run --input /custom/video.mp4 --output /custom/output
```

### Container Shell Access

```bash
./run shell
# or
docker exec -it smart-counter-runner bash
```

### Resource Monitoring

```bash
./run status
# or
docker stats smart-counter-runner
```

## Documentation

- **Full Guide**: [DEPLOYMENT.md](DEPLOYMENT.md)
- **Architecture**: [ARCHITECTURE.md](ARCHITECTURE.md)
- **Tech Stack**: [TECH_STACK.md](TECH_STACK.md)

## Support

- Issues: https://github.com/bigalex95/Smart-Counter/issues
- Questions: Use GitHub Discussions
