#!/bin/bash

# Exit on error
set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

echo -e "${GREEN}🚀 Setting up Smart-Counter...${NC}"
echo ""

# Detect OS
if [ -f /etc/os-release ]; then
    . /etc/os-release
    OS=$ID
    VERSION=$VERSION_ID
else
    OS=$(uname -s)
fi

echo -e "${BLUE}📋 Detected OS: ${OS} ${VERSION}${NC}"
echo ""

# 1. Create Directory Structure
echo -e "${YELLOW}📁 Creating project directories...${NC}"
mkdir -p data/videos
mkdir -p data/output
mkdir -p models
mkdir -p build
echo -e "${GREEN}✅ Directories created${NC}"
echo ""

# 2. Install System Dependencies
echo -e "${YELLOW}📦 Checking and installing system dependencies...${NC}"

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
                libopencv-dev \
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
                opencv-devel \
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
                opencv \
                python \
                python-pip
            ;;
        *)
            echo -e "${RED}⚠️  Unknown distribution. Please install manually:${NC}"
            echo -e "  - CMake (>= 3.10)"
            echo -e "  - OpenCV"
            echo -e "  - C++ compiler (g++/clang)"
            echo -e "  - Python 3"
            read -p "Press Enter to continue after installing dependencies..."
            ;;
    esac
}

if command -v apt-get &> /dev/null || command -v dnf &> /dev/null || command -v pacman &> /dev/null; then
    read -p "Install/update system dependencies? (y/N): " -n 1 -r
    echo
    if [[ $REPLY =~ ^[Yy]$ ]]; then
        install_dependencies
    else
        echo -e "${YELLOW}⏭️  Skipping system dependencies installation${NC}"
    fi
else
    echo -e "${YELLOW}⏭️  Package manager not detected, skipping system dependencies${NC}"
fi
echo ""

# 3. Check for CUDA and cuDNN
echo -e "${YELLOW}🎮 Checking CUDA installation...${NC}"
if command -v nvcc &> /dev/null; then
    CUDA_VERSION=$(nvcc --version | grep "release" | sed -n 's/.*release \([0-9\.]*\).*/\1/p')
    echo -e "${GREEN}✅ CUDA ${CUDA_VERSION} detected${NC}"
    
    # Check for cuDNN
    if ldconfig -p | grep -q libcudnn; then
        CUDNN_VERSION=$(ldconfig -p | grep libcudnn | grep -oP 'libcudnn\.so\.\K[0-9]+' | head -1)
        echo -e "${GREEN}✅ cuDNN ${CUDNN_VERSION} detected${NC}"
    else
        echo -e "${RED}⚠️  cuDNN not found!${NC}"
        echo -e "${YELLOW}For GPU support, install cuDNN:${NC}"
        echo -e "  1. Download from: https://developer.nvidia.com/cudnn"
        echo -e "  2. Or install via conda: conda install -c conda-forge cudnn"
    fi
else
    echo -e "${YELLOW}⚠️  CUDA not detected. GPU acceleration will not be available.${NC}"
    echo -e "${BLUE}The project will run on CPU.${NC}"
fi
echo ""

# 4. Download ONNX Runtime if not present
ORT_VERSION="1.23.2"
ORT_GPU_DIR="third_party/onnxruntime-linux-x64-gpu-${ORT_VERSION}"
ORT_CPU_DIR="third_party/onnxruntime-linux-x64-${ORT_VERSION}"

if [ ! -d "${ORT_GPU_DIR}" ] && [ ! -d "${ORT_CPU_DIR}" ]; then
    echo -e "${YELLOW}⬇️  Downloading ONNX Runtime...${NC}"
    
    read -p "Download GPU version (requires CUDA)? (y/N): " -n 1 -r
    echo
    if [[ $REPLY =~ ^[Yy]$ ]]; then
        ORT_URL="https://github.com/microsoft/onnxruntime/releases/download/v${ORT_VERSION}/onnxruntime-linux-x64-gpu-${ORT_VERSION}.tgz"
        ORT_FILE="onnxruntime-gpu.tgz"
    else
        ORT_URL="https://github.com/microsoft/onnxruntime/releases/download/v${ORT_VERSION}/onnxruntime-linux-x64-${ORT_VERSION}.tgz"
        ORT_FILE="onnxruntime-cpu.tgz"
    fi
    
    mkdir -p third_party
    wget -O "third_party/${ORT_FILE}" "${ORT_URL}"
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
pip install -r requirements.txt 2>/dev/null || pip install ultralytics onnx onnxruntime opencv-python
echo -e "${GREEN}✅ Python dependencies installed${NC}"
deactivate
echo ""

# 6. Download YOLOv8 Model if not present
if [ ! -f "models/yolov8s.pt" ]; then
    echo -e "${YELLOW}⬇️  Downloading YOLOv8 model...${NC}"
    python3 -c "from ultralytics import YOLO; YOLO('yolov8s.pt')" 2>/dev/null || wget -O models/yolov8s.pt https://github.com/ultralytics/assets/releases/download/v0.0.0/yolov8s.pt
    mv yolov8s.pt models/ 2>/dev/null || true
    echo -e "${GREEN}✅ YOLOv8 model downloaded${NC}"
else
    echo -e "${GREEN}✅ YOLOv8 model already exists${NC}"
fi
echo ""

# 7. Convert model to ONNX if needed
if [ ! -f "models/yolov8s.onnx" ]; then
    echo -e "${YELLOW}🔄 Converting model to ONNX format...${NC}"
    source venv/bin/activate
    python3 python/convert.py
    deactivate
    echo -e "${GREEN}✅ Model converted to ONNX${NC}"
else
    echo -e "${GREEN}✅ ONNX model already exists${NC}"
fi
echo ""

# 8. Make scripts executable
echo -e "${YELLOW}🔐 Setting script permissions...${NC}"
chmod +x scripts/*.sh 2>/dev/null || true
echo -e "${GREEN}✅ Scripts are now executable${NC}"
echo ""

# Final summary
echo -e "${GREEN}━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━${NC}"
echo -e "${GREEN}✅ Smart-Counter setup complete!${NC}"
echo -e "${GREEN}━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━${NC}"
echo ""
echo -e "${YELLOW}📝 Next steps:${NC}"
echo -e "  1. Build the project:    ${GREEN}./scripts/build.sh${NC}"
echo -e "  2. Run the application:  ${GREEN}./scripts/run.sh${NC}"
echo ""
echo -e "${BLUE}💡 Tips:${NC}"
echo -e "  - Place video files in ${GREEN}data/videos/${NC}"
echo -e "  - Results will be saved to ${GREEN}data/output/${NC}"
echo -e "  - Activate Python env: ${GREEN}source venv/bin/activate${NC}"
echo ""