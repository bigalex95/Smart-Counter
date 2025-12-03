#!/bin/bash
# Smart-Counter MLOps Setup Script
# Supports: local development, Docker builds, and server deployment

set -e  # Exit on error
set -o pipefail  # Catch errors in pipelines

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Configuration
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "${SCRIPT_DIR}/.." && pwd)"
ORT_VERSION="1.23.2"
YOLOV8_MODEL="yolov8s"

# Deployment mode detection
DEPLOY_MODE="${DEPLOY_MODE:-local}"  # local, docker, server
SKIP_INTERACTIVE="${SKIP_INTERACTIVE:-false}"

echo -e "${GREEN}🚀 Smart-Counter MLOps Setup${NC}"
echo -e "${BLUE}Mode: ${DEPLOY_MODE}${NC}"
echo -e "${BLUE}Root: ${PROJECT_ROOT}${NC}"
echo ""

# Detect OS and architecture
if [ -f /etc/os-release ]; then
    . /etc/os-release
    OS=$ID
    VERSION=$VERSION_ID
else
    OS=$(uname -s)
fi

ARCH=$(uname -m)
echo -e "${BLUE}📋 System: ${OS} ${VERSION} (${ARCH})${NC}"
echo ""

# Helper function for prompts
prompt_yes_no() {
    local question="$1"
    local default="${2:-N}"
    
    if [ "${SKIP_INTERACTIVE}" = "true" ]; then
        echo -e "${YELLOW}Auto-answer: ${default}${NC}"
        [[ "${default}" =~ ^[Yy]$ ]]
        return $?
    fi
    
    read -p "${question} (y/N): " -n 1 -r
    echo
    [[ $REPLY =~ ^[Yy]$ ]]
}

# 1. Create Directory Structure
echo -e "${YELLOW}📁 Creating project directories...${NC}"
cd "${PROJECT_ROOT}"
mkdir -p data/videos data/output models build logs
echo -e "${GREEN}✅ Directories created${NC}"
echo ""

# 2. Install System Dependencies
echo -e "${YELLOW}📦 Checking system dependencies...${NC}"

install_dependencies() {
    case $OS in
        ubuntu|debian|pop)
            echo -e "${BLUE}Installing dependencies for Ubuntu/Debian...${NC}"
            sudo apt-get update
            sudo apt-get install -y \
                build-essential \
                cmake \
                git \
                wget \
                curl \
                libopencv-dev \
                libsqlite3-dev \
                python3 \
                python3-pip \
                python3-venv
            ;;
        fedora|rhel|centos)
            echo -e "${BLUE}Installing dependencies for Fedora/RHEL...${NC}"
            sudo dnf install -y \
                gcc-c++ \
                cmake \
                git \
                wget \
                curl \
                opencv-devel \
                sqlite-devel \
                python3 \
                python3-pip
            ;;
        arch|manjaro)
            echo -e "${BLUE}Installing dependencies for Arch Linux...${NC}"
            sudo pacman -S --needed --noconfirm \
                base-devel \
                cmake \
                git \
                wget \
                curl \
                opencv \
                sqlite \
                python \
                python-pip
            ;;
        *)
            echo -e "${RED}⚠️  Unknown distribution. Please install manually:${NC}"
            echo -e "  - CMake (>= 3.10)"
            echo -e "  - OpenCV"
            echo -e "  - SQLite3 development libraries"
            echo -e "  - C++ compiler (g++/clang)"
            echo -e "  - Python 3"
            ;;
    esac
}

# Skip system dependencies in Docker mode
if [ "${DEPLOY_MODE}" != "docker" ]; then
    if command -v apt-get &> /dev/null || command -v dnf &> /dev/null || command -v pacman &> /dev/null; then
        if prompt_yes_no "Install/update system dependencies?" "N"; then
            install_dependencies
        else
            echo -e "${YELLOW}⏭️  Skipping system dependencies installation${NC}"
        fi
    else
        echo -e "${YELLOW}⏭️  Package manager not detected, skipping system dependencies${NC}"
    fi
else
    echo -e "${BLUE}🐳 Docker mode: skipping host system dependencies${NC}"
fi
echo ""

# 3. Check for CUDA and cuDNN using check_cuda.sh
echo -e "${YELLOW}🎮 Checking CUDA installation...${NC}"
if [ -f "${SCRIPT_DIR}/check_cuda.sh" ]; then
    bash "${SCRIPT_DIR}/check_cuda.sh" || true
else
    if command -v nvcc &> /dev/null; then
        CUDA_VERSION=$(nvcc --version | grep "release" | sed -n 's/.*release \([0-9\.]*\).*/\1/p')
        echo -e "${GREEN}✅ CUDA ${CUDA_VERSION} detected${NC}"
    else
        echo -e "${YELLOW}⚠️  CUDA not detected. GPU acceleration will not be available.${NC}"
    fi
fi
echo ""

# 4. Download ONNX Runtime if not present
ORT_GPU_DIR="third_party/onnxruntime-linux-x64-gpu-${ORT_VERSION}"
ORT_CPU_DIR="third_party/onnxruntime-linux-x64-${ORT_VERSION}"

if [ ! -d "${ORT_GPU_DIR}" ] && [ ! -d "${ORT_CPU_DIR}" ]; then
    echo -e "${YELLOW}⬇️  Downloading ONNX Runtime...${NC}"
    
    # Auto-detect GPU support
    USE_GPU="N"
    if command -v nvidia-smi &> /dev/null; then
        USE_GPU="Y"
        echo -e "${GREEN}GPU detected, will download GPU version${NC}"
    fi
    
    if prompt_yes_no "Download GPU version (requires CUDA)?" "${USE_GPU}"; then
        ORT_URL="https://github.com/microsoft/onnxruntime/releases/download/v${ORT_VERSION}/onnxruntime-linux-x64-gpu-${ORT_VERSION}.tgz"
        ORT_FILE="onnxruntime-gpu.tgz"
    else
        ORT_URL="https://github.com/microsoft/onnxruntime/releases/download/v${ORT_VERSION}/onnxruntime-linux-x64-${ORT_VERSION}.tgz"
        ORT_FILE="onnxruntime-cpu.tgz"
    fi
    
    mkdir -p third_party
    if command -v wget &> /dev/null; then
        wget -q --show-progress -O "third_party/${ORT_FILE}" "${ORT_URL}"
    elif command -v curl &> /dev/null; then
        curl -L -o "third_party/${ORT_FILE}" "${ORT_URL}"
    else
        echo -e "${RED}❌ wget or curl required for download${NC}"
        exit 1
    fi
    
    tar -xzf "third_party/${ORT_FILE}" -C third_party/
    rm "third_party/${ORT_FILE}"
    echo -e "${GREEN}✅ ONNX Runtime downloaded${NC}"
else
    echo -e "${GREEN}✅ ONNX Runtime already exists${NC}"
fi
echo ""

# 5. Setup Python Environment for Model Conversion
echo -e "${YELLOW}🐍 Setting up Python environment...${NC}"
if [ ! -d "venv" ]; then
    python3 -m venv venv
    echo -e "${GREEN}✅ Virtual environment created${NC}"
fi

source venv/bin/activate
pip install --upgrade pip > /dev/null 2>&1
pip install -r requirements.txt 2>/dev/null || pip install ultralytics onnx onnxruntime opencv-python numpy
echo -e "${GREEN}✅ Python dependencies installed${NC}"
deactivate
echo ""

# 6. Download YOLOv8 Model if not present
if [ ! -f "models/${YOLOV8_MODEL}.pt" ]; then
    echo -e "${YELLOW}⬇️  Downloading YOLOv8 model (${YOLOV8_MODEL})...${NC}"
    
    MODEL_URL="https://github.com/ultralytics/assets/releases/download/v8.0.0/${YOLOV8_MODEL}.pt"
    if command -v wget &> /dev/null; then
        wget -q --show-progress -O "models/${YOLOV8_MODEL}.pt" "${MODEL_URL}"
    elif command -v curl &> /dev/null; then
        curl -L -o "models/${YOLOV8_MODEL}.pt" "${MODEL_URL}"
    else
        # Fallback to Python download
        python3 -c "from ultralytics import YOLO; YOLO('${YOLOV8_MODEL}.pt')" 2>/dev/null || true
        mv "${YOLOV8_MODEL}.pt" models/ 2>/dev/null || true
    fi
    
    if [ -f "models/${YOLOV8_MODEL}.pt" ]; then
        echo -e "${GREEN}✅ YOLOv8 model downloaded${NC}"
    else
        echo -e "${RED}❌ Failed to download model${NC}"
        exit 1
    fi
else
    echo -e "${GREEN}✅ YOLOv8 model already exists${NC}"
fi
echo ""

# 7. Convert model to ONNX if needed
if [ ! -f "models/${YOLOV8_MODEL}.onnx" ]; then
    echo -e "${YELLOW}🔄 Converting model to ONNX format...${NC}"
    
    if [ -f "venv/bin/activate" ]; then
        source venv/bin/activate
        python3 python/convert.py || {
            echo -e "${RED}❌ Conversion failed${NC}"
            deactivate
            exit 1
        }
        deactivate
        echo -e "${GREEN}✅ Model converted to ONNX${NC}"
    else
        echo -e "${YELLOW}⚠️  Virtual environment not found, skipping conversion${NC}"
        echo -e "${BLUE}Run this script first to create venv${NC}"
    fi
else
    echo -e "${GREEN}✅ ONNX model already exists${NC}"
fi
echo ""

# 8. Make scripts executable
echo -e "${YELLOW}🔐 Setting script permissions...${NC}"
chmod +x scripts/*.sh 2>/dev/null || true
echo -e "${GREEN}✅ Scripts are now executable${NC}"
echo ""

# 9. Docker setup (if requested)
if [ "${DEPLOY_MODE}" = "docker" ] || [ "${DEPLOY_MODE}" = "server" ]; then
    echo -e "${YELLOW}🐳 Checking Docker setup...${NC}"
    
    if ! command -v docker &> /dev/null; then
        echo -e "${RED}❌ Docker not found${NC}"
        echo -e "${BLUE}Install: https://docs.docker.com/engine/install/${NC}"
    else
        echo -e "${GREEN}✅ Docker installed${NC}"
        
        # Check NVIDIA Container Toolkit for GPU support
        if command -v nvidia-smi &> /dev/null; then
            if ! command -v nvidia-ctk &> /dev/null; then
                echo -e "${YELLOW}⚠️  NVIDIA Container Toolkit not found${NC}"
                if prompt_yes_no "Install NVIDIA Container Toolkit for GPU support?" "Y"; then
                    echo -e "${BLUE}Installing NVIDIA Container Toolkit...${NC}"
                    curl -fsSL https://nvidia.github.io/libnvidia-container/gpgkey | sudo gpg --dearmor -o /usr/share/keyrings/nvidia-container-toolkit-keyring.gpg
                    curl -s -L https://nvidia.github.io/libnvidia-container/stable/deb/nvidia-container-toolkit.list | \
                        sed 's#deb https://#deb [signed-by=/usr/share/keyrings/nvidia-container-toolkit-keyring.gpg] https://#g' | \
                        sudo tee /etc/apt/sources.list.d/nvidia-container-toolkit.list
                    sudo apt-get update
                    sudo apt-get install -y nvidia-container-toolkit
                    sudo nvidia-ctk runtime configure --runtime=docker
                    sudo systemctl restart docker
                    echo -e "${GREEN}✅ NVIDIA Container Toolkit installed${NC}"
                fi
            else
                echo -e "${GREEN}✅ NVIDIA Container Toolkit installed${NC}"
            fi
        fi
        
        # Add user to docker group if not already
        if ! groups | grep -q docker; then
            echo -e "${YELLOW}Adding user to docker group...${NC}"
            sudo usermod -aG docker "${USER}"
            echo -e "${GREEN}✅ User added to docker group${NC}"
            echo -e "${YELLOW}⚠️  Log out and back in for changes to take effect${NC}"
        fi
    fi
    echo ""
fi

# Final summary
echo -e "${GREEN}━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━${NC}"
echo -e "${GREEN}✅ Smart-Counter MLOps Setup Complete!${NC}"
echo -e "${GREEN}━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━${NC}"
echo ""
echo -e "${YELLOW}📝 Next steps:${NC}"

if [ "${DEPLOY_MODE}" = "docker" ] || [ "${DEPLOY_MODE}" = "server" ]; then
    echo -e "  ${BLUE}Docker Deployment:${NC}"
    echo -e "    Build:  ${GREEN}docker build -t smart-counter-cpp .${NC}"
    echo -e "    Run:    ${GREEN}docker run --rm --gpus all -v \$(pwd)/data/output:/app/data/output smart-counter-cpp${NC}"
    echo ""
else
    echo -e "  ${BLUE}Local Development:${NC}"
    echo -e "    Build:  ${GREEN}./scripts/build.sh${NC}"
    echo -e "    Run:    ${GREEN}./scripts/run.sh${NC}"
    echo ""
fi

echo -e "${BLUE}💡 Tips:${NC}"
echo -e "  - Place video files in ${GREEN}data/videos/${NC}"
echo -e "  - Results saved to ${GREEN}data/output/${NC}"
echo -e "  - Check CUDA: ${GREEN}./scripts/check_cuda.sh${NC}"
echo -e "  - Logs saved to ${GREEN}logs/${NC}"
echo ""
echo -e "${BLUE}🚀 Quick Start (Server Deployment):${NC}"
echo -e "  ${GREEN}DEPLOY_MODE=server SKIP_INTERACTIVE=true ./scripts/setup_mlops.sh${NC}"
echo ""
