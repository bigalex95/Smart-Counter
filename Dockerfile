# 1. Base image with CUDA (the version should match yours or be newer)
# We use the devel variant, which includes the GCC compiler and headers
FROM nvidia/cuda:12.9.1-cudnn-devel-ubuntu22.04

# 2. Disable interactive prompts during installation (tzdata, etc.)
ENV DEBIAN_FRONTEND=noninteractive

# 3. Install system dependencies
# cmake, git, build-essential - for building
# libopencv-dev - prebuilt OpenCV from the Ubuntu repository (v4.5.4)
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    git \
    wget \
    libopencv-dev \
    libsqlite3-dev \
    && rm -rf /var/lib/apt/lists/*

# 4. Create the working directory
WORKDIR /app

# 5. Copy source code and dependencies
# .dockerignore keeps junk (build, .git) out of the image
COPY CMakeLists.txt .
COPY src/ ./src/
COPY include/ ./include/
COPY models/ ./models/
COPY data/ ./data/
# Important: copy our downloaded ONNX Runtime
COPY third_party/ ./third_party/
# Copy helper scripts (check_cuda, build/run helpers)
COPY scripts/ ./scripts/

# 6. Create the directory for logs and the database
RUN mkdir -p logs

# 7. Configure the build
RUN mkdir build && cd build && \
    cmake -DCMAKE_BUILD_TYPE=Release .. && \
    make -j$(nproc) && \
    chmod +x SmartCounter

# 8. Set the run command
# We need to set the library path so the program can find them
ENV LD_LIBRARY_PATH="/app/third_party/onnxruntime-linux-x64-gpu-1.23.2/lib:${LD_LIBRARY_PATH}"

# Keep WORKDIR at /app so relative paths (models/, data/) work
WORKDIR /app

# Ensure check script is executable and run with shell so `&&` works
RUN chmod +x /app/scripts/check_cuda.sh || true
CMD ["bash","-lc","/app/scripts/check_cuda.sh && ./build/SmartCounter --headless"]