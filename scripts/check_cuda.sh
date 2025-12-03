#!/bin/bash
# Robust CUDA and cuDNN version checking script

# Colors for formatting
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

echo -e "${BLUE}==========================================${NC}"
echo -e "${BLUE}     CUDA & cuDNN Version Information     ${NC}"
echo -e "${BLUE}==========================================${NC}"

# ---------------------------------------------------------
# 1. CHECK CUDA TOOLKIT (NVCC)
# ---------------------------------------------------------
echo -e "\n[1] Checking CUDA Toolkit (NVCC)..."
if command -v nvcc &> /dev/null; then
    NVCC_VERSION=$(nvcc --version | grep "release" | sed 's/.*release //; s/,.*//')
    NVCC_PATH=$(which nvcc)
    echo -e "  ${GREEN}✔ CUDA Toolkit Found${NC}"
    echo -e "  Version: $NVCC_VERSION"
    echo -e "  Path:    $NVCC_PATH"
elif [ -f "/usr/local/cuda/bin/nvcc" ]; then
    # Fallback if not in PATH but exists in default location
    NVCC_VERSION=$(/usr/local/cuda/bin/nvcc --version | grep "release" | sed 's/.*release //; s/,.*//')
    echo -e "  ${YELLOW}✔ CUDA Toolkit Found (Not in PATH)${NC}"
    echo -e "  Version: $NVCC_VERSION"
    echo -e "  Path:    /usr/local/cuda/bin/nvcc"
    echo -e "  ${YELLOW}Suggestion: Add /usr/local/cuda/bin to your PATH${NC}"
else
    echo -e "  ${RED}✘ NVCC not found (CUDA Toolkit might not be installed)${NC}"
fi

# ---------------------------------------------------------
# 2. CHECK CUDNN (Header Search)
# ---------------------------------------------------------
echo -e "\n[2] Checking cuDNN Headers..."

# List of common paths to search. 
# Your specific path (/usr/include/x86_64-linux-gnu) is added here.
POSSIBLE_PATHS=(
    "/usr/include/cudnn_version.h"                 # Standard Linux
    "/usr/include/x86_64-linux-gnu/cudnn_version.h" # Ubuntu/Debian Multiarch (YOURS)
    "/usr/local/cuda/include/cudnn_version.h"      # Tarball install
    "/usr/include/cudnn.h"                         # Legacy v7
    "/usr/local/cuda/include/cudnn.h"              # Legacy v7 Tarball
)

CUDNN_FOUND=false

for HEADER_PATH in "${POSSIBLE_PATHS[@]}"; do
    if [ -f "$HEADER_PATH" ]; then
        echo -e "  ${GREEN}✔ cuDNN Header Found${NC}"
        echo -e "  Location: $HEADER_PATH"
        
        # Parse version
        MAJOR=$(grep "#define CUDNN_MAJOR" "$HEADER_PATH" | head -1 | awk '{print $3}')
        MINOR=$(grep "#define CUDNN_MINOR" "$HEADER_PATH" | head -1 | awk '{print $3}')
        PATCH=$(grep "#define CUDNN_PATCHLEVEL" "$HEADER_PATH" | head -1 | awk '{print $3}')
        
        echo -e "  Version:  ${GREEN}${MAJOR}.${MINOR}.${PATCH}${NC}"
        CUDNN_FOUND=true
        break
    fi
done

if [ "$CUDNN_FOUND" = false ]; then
    echo -e "  ${RED}✘ cuDNN headers not found in standard locations.${NC}"
    echo -e "  (Make sure libcudnn*-dev is installed)"
fi

# ---------------------------------------------------------
# 3. CHECK GPU DRIVER
# ---------------------------------------------------------
echo -e "\n[3] Checking GPU Driver..."
if command -v nvidia-smi &> /dev/null; then
    echo -e "  ${GREEN}✔ GPU Detected${NC}"
    nvidia-smi --query-gpu=driver_version,name,memory.total,compute_cap --format=csv,noheader | \
    awk -F', ' '{printf "  Driver:   %s\n  GPU:      %s\n  Memory:   %s\n  Compute:  %s\n", $1, $2, $3, $4}'
else
    echo -e "  ${RED}✘ nvidia-smi not available${NC}"
    echo "  (GPU not detected or driver not loaded)"
fi

echo -e "${BLUE}==========================================${NC}"