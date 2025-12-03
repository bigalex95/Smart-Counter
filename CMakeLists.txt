cmake_minimum_required(VERSION 3.10)
project(SmartCounter)

# Используем современный стандарт C++17
set(CMAKE_CXX_STANDARD 17)

# Находим OpenCV (она обычно установлена в системе)
find_package(OpenCV REQUIRED)

# Находим SQLite3
find_package(SQLite3 REQUIRED)

# Настраиваем ONNX Runtime
# ВАЖНО: Убедись, что имя папки совпадает с твоим!
set(ORT_FOLDER_NAME "onnxruntime-linux-x64-gpu-1.23.2")
set(ORT_ROOT "${CMAKE_SOURCE_DIR}/third_party/${ORT_FOLDER_NAME}")

include_directories(${ORT_ROOT}/include)
link_directories(${ORT_ROOT}/lib)

# Настраиваем RPATH (чтобы программа знала, где искать .so файлы)
set(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)
set(CMAKE_INSTALL_RPATH "${ORT_ROOT}/lib")
set(CMAKE_BUILD_RPATH "${ORT_ROOT}/lib")

# Собираем исполняемый файл
add_executable(SmartCounter src/main.cpp src/detector.cpp src/tracker.cpp src/database.cpp)

# Подключаем заголовки
target_include_directories(SmartCounter PUBLIC
    ${CMAKE_SOURCE_DIR}/include
    ${OpenCV_INCLUDE_DIRS}
)

# Линкуем библиотеки
target_link_libraries(SmartCounter
    ${OpenCV_LIBS}
    onnxruntime
    onnxruntime_providers_cuda
    onnxruntime_providers_shared
    SQLite::SQLite3
)
