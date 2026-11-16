cmake_minimum_required(VERSION 3.14)
project(SmartCounter LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(SMARTCOUNTER_BUILD_APP "Build the SmartCounter executable (needs ONNX Runtime, SQLite3)" ON)
option(SMARTCOUNTER_BUILD_TESTS "Build unit tests (needs GoogleTest; no ONNX Runtime required)" OFF)

find_package(OpenCV REQUIRED)

# Core logic (tracker + line crossing). Depends on OpenCV types only, so it can be
# built and unit-tested without ONNX Runtime or a GPU.
add_library(smartcounter_core STATIC
    src/tracker.cpp
    src/line_counter.cpp
)
target_include_directories(smartcounter_core PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${OpenCV_INCLUDE_DIRS}
)
target_link_libraries(smartcounter_core PUBLIC ${OpenCV_LIBS})

if(SMARTCOUNTER_BUILD_APP)
    find_package(SQLite3 REQUIRED)

    # ONNX Runtime (prebuilt GPU package) is expected under third_party/.
    # See docs/DOCKER.md / README for the download command.
    set(ORT_FOLDER_NAME "onnxruntime-linux-x64-gpu-1.23.2")
    set(ORT_ROOT "${CMAKE_SOURCE_DIR}/third_party/${ORT_FOLDER_NAME}")

    include_directories(${ORT_ROOT}/include)
    link_directories(${ORT_ROOT}/lib)

    # RPATH so the binary finds the ONNX Runtime .so files
    set(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)
    set(CMAKE_INSTALL_RPATH "${ORT_ROOT}/lib")
    set(CMAKE_BUILD_RPATH "${ORT_ROOT}/lib")

    add_executable(SmartCounter src/main.cpp src/detector.cpp src/database.cpp)
    target_link_libraries(SmartCounter
        smartcounter_core
        onnxruntime
        onnxruntime_providers_cuda
        onnxruntime_providers_shared
        SQLite::SQLite3
    )
endif()

if(SMARTCOUNTER_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
