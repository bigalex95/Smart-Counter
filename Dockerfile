# 1. Базовый образ с CUDA (важно, чтобы версия совпадала с твоей или была свежее)
# Мы берем devel-версию, в ней есть компилятор GCC и заголовки
FROM nvidia/cuda:12.9.1-cudnn-devel-ubuntu22.04

# 2. Отключаем интерактивные вопросы при установке (tzdata и т.д.)
ENV DEBIAN_FRONTEND=noninteractive

# 3. Устанавливаем системные зависимости
# cmake, git, build-essential - для сборки
# libopencv-dev - готовый OpenCV из репозитория Ubuntu (v4.5.4)
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    git \
    wget \
    libopencv-dev \
    && rm -rf /var/lib/apt/lists/*

# 4. Создаем рабочую директорию
WORKDIR /app

# 5. Копируем исходный код и зависимости
# .dockerignore поможет не копировать мусор (build, .git)
COPY CMakeLists.txt .
COPY src/ ./src/
COPY include/ ./include/
COPY models/ ./models/
COPY data/ ./data/
# Важно: копируем наш скачанный ONNX Runtime
COPY third_party/ ./third_party/
# Копируем служебные скрипты (check_cuda, build/run helpers)
COPY scripts/ ./scripts/

# 6. Настраиваем сборку
RUN mkdir build && cd build && \
    cmake -DCMAKE_BUILD_TYPE=Release .. && \
    make -j$(nproc) && \
    chmod +x SmartCounter

# 7. Указываем команду запуска
# Нам нужно прописать путь к библиотекам, чтобы программа их нашла
ENV LD_LIBRARY_PATH="/app/third_party/onnxruntime-linux-x64-gpu-1.23.2/lib:${LD_LIBRARY_PATH}"

# Оставляем WORKDIR в /app чтобы относительные пути (models/, data/) работали
WORKDIR /app

# Ensure check script is executable and run with shell so `&&` works
RUN chmod +x /app/scripts/check_cuda.sh || true
CMD ["bash","-lc","/app/scripts/check_cuda.sh && ./build/SmartCounter --headless"]