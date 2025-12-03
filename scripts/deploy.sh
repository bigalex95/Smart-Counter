#!/bin/bash
# MLOps Deployment Script for Smart-Counter
# Handles: build, deploy, run, and monitor

set -e

# Colors
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m'

# Configuration
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "${SCRIPT_DIR}/.." && pwd)"
IMAGE_NAME="${IMAGE_NAME:-smart-counter-cpp}"
IMAGE_TAG="${IMAGE_TAG:-latest}"
CONTAINER_NAME="${CONTAINER_NAME:-smart-counter-runner}"

# Functions
show_usage() {
    echo -e "${GREEN}Smart-Counter MLOps Deployment${NC}"
    echo ""
    echo "Usage: $0 [command] [options]"
    echo ""
    echo -e "${YELLOW}Commands:${NC}"
    echo "  build       Build Docker image"
    echo "  run         Run container (one-time execution)"
    echo "  deploy      Deploy as persistent service"
    echo "  stop        Stop running container/service"
    echo "  logs        Show container logs"
    echo "  status      Check container status"
    echo "  shell       Open shell in running container"
    echo "  clean       Remove stopped containers and dangling images"
    echo ""
    echo -e "${YELLOW}Options:${NC}"
    echo "  --gpu          Enable GPU support (default: auto-detect)"
    echo "  --cpu          Force CPU-only mode"
    echo "  --input PATH   Input video file or directory"
    echo "  --output PATH  Output video file path (default: data/output/output.mp4)"
    echo "  --model PATH   Path to ONNX model file (default: models/yolov8s.onnx)"
    echo "  --db PATH      Path to SQLite database (default: logs/analytics.db)"
    echo "  --display      Show display window (default: headless in Docker)"
    echo "  --dev          Mount source code for development"
    echo ""
    echo -e "${YELLOW}Examples:${NC}"
    echo "  $0 build"
    echo "  $0 run --gpu --display"
    echo "  $0 run --input /path/to/video.mp4 --output result.mp4"
    echo "  $0 run --model models/yolov8n.onnx --cpu"
    echo "  $0 deploy --gpu"
    echo "  $0 logs"
    echo "  $0 stop"
    echo ""
}

log_info() {
    echo -e "${BLUE}[INFO]${NC} $1"
}

log_success() {
    echo -e "${GREEN}[SUCCESS]${NC} $1"
}

log_warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1"
}

log_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

# Detect GPU support
detect_gpu() {
    if command -v nvidia-smi &> /dev/null && nvidia-smi &> /dev/null; then
        return 0
    fi
    return 1
}

# Build Docker image
build_image() {
    log_info "Building Docker image: ${IMAGE_NAME}:${IMAGE_TAG}"
    cd "${PROJECT_ROOT}"
    
    docker build \
        --build-arg BUILDKIT_INLINE_CACHE=1 \
        -t "${IMAGE_NAME}:${IMAGE_TAG}" \
        -f Dockerfile \
        .
    
    log_success "Image built successfully"
}

# Run container (one-time)
run_container() {
    local gpu_flag=""
    local input_mount=""
    local input_path=""
    local output_mount="-v ${PROJECT_ROOT}/data/output:/app/data/output"
    local output_path=""
    local model_mount=""
    local model_path=""
    local db_mount="-v ${PROJECT_ROOT}/logs:/app/logs"
    local db_path=""
    local dev_mount=""
    local run_mode="--headless"
    local cpu_flag=""
    local display_flags=""
    local app_args=""
    
    # Parse options
    while [[ $# -gt 0 ]]; do
        case $1 in
            --gpu)
                gpu_flag="--gpus all"
                shift
                ;;
            --cpu)
                gpu_flag=""
                cpu_flag="--cpu"
                shift
                ;;
            --input)
                input_path="$2"
                if [[ -f "$2" ]]; then
                    # Single file - mount it directly
                    input_mount="-v $(realpath $2):/app/input_video.mp4"
                    app_args="${app_args} --input /app/input_video.mp4"
                elif [[ -d "$2" ]]; then
                    # Directory - mount the whole directory
                    input_mount="-v $(realpath $2):/app/data/videos"
                    app_args="${app_args} --input /app/data/videos"
                else
                    log_error "Input path does not exist: $2"
                    return 1
                fi
                shift 2
                ;;
            --output)
                output_path="$2"
                mkdir -p "$(dirname $(realpath $2))" 2>/dev/null || true
                output_mount="-v $(realpath $(dirname $2)):/app/output_dir"
                app_args="${app_args} --output /app/output_dir/$(basename $2)"
                shift 2
                ;;
            --model)
                model_path="$2"
                if [[ -f "$2" ]]; then
                    model_mount="-v $(realpath $2):/app/model.onnx"
                    app_args="${app_args} --model /app/model.onnx"
                else
                    log_error "Model file does not exist: $2"
                    return 1
                fi
                shift 2
                ;;
            --db)
                db_path="$2"
                mkdir -p "$(dirname $(realpath $2))" 2>/dev/null || true
                db_mount="-v $(realpath $(dirname $2)):/app/db_dir"
                app_args="${app_args} --db /app/db_dir/$(basename $2)"
                shift 2
                ;;
            --dev)
                dev_mount="-v ${PROJECT_ROOT}/src:/app/src -v ${PROJECT_ROOT}/include:/app/include"
                shift
                ;;
            --display)
                run_mode=""
                display_flags="-e DISPLAY=${DISPLAY} -v /tmp/.X11-unix:/tmp/.X11-unix"
                shift
                ;;
            *)
                shift
                ;;
        esac
    done
    
    # Auto-detect GPU if not specified
    if [ -z "$gpu_flag" ] && detect_gpu; then
        log_info "GPU detected, enabling GPU support"
        gpu_flag="--gpus all"
    fi
    
    # If display flags are set, allow X11 connections
    if [ -n "$display_flags" ]; then
        log_info "Enabling display output..."
        xhost +local:docker > /dev/null 2>&1 || log_warning "Could not run xhost (might not be needed)"
    fi
    
    log_info "Running container..."
    if [ -n "$input_path" ]; then
        log_info "Input: $input_path"
    fi
    if [ -n "$output_path" ]; then
        log_info "Output: $output_path"
    fi
    if [ -n "$model_path" ]; then
        log_info "Model: $model_path"
    fi
    if [ -n "$db_path" ]; then
        log_info "Database: $db_path"
    fi
    
    docker run --rm -it \
        --name "${CONTAINER_NAME}" \
        ${gpu_flag} \
        ${display_flags} \
        ${input_mount} \
        ${output_mount} \
        ${model_mount} \
        ${db_mount} \
        ${dev_mount} \
        "${IMAGE_NAME}:${IMAGE_TAG}" \
        bash -c "/app/scripts/check_cuda.sh && ./build/SmartCounter ${run_mode} ${cpu_flag} ${app_args}"
    
    log_success "Container execution completed"
}

# Deploy as service
deploy_service() {
    local gpu_flag=""
    
    if detect_gpu; then
        log_info "GPU detected, enabling GPU support"
        gpu_flag="--gpus all"
    fi
    
    log_info "Deploying as persistent service..."
    
    docker run -d \
        --name "${CONTAINER_NAME}" \
        --restart unless-stopped \
        ${gpu_flag} \
        -v "${PROJECT_ROOT}/data/videos:/app/data/videos" \
        -v "${PROJECT_ROOT}/data/output:/app/data/output" \
        -v "${PROJECT_ROOT}/logs:/app/logs" \
        "${IMAGE_NAME}:${IMAGE_TAG}"
    
    log_success "Service deployed: ${CONTAINER_NAME}"
    log_info "Check status with: $0 status"
    log_info "View logs with: $0 logs"
}

# Stop container/service
stop_container() {
    if docker ps -q -f name="${CONTAINER_NAME}" > /dev/null 2>&1; then
        log_info "Stopping container: ${CONTAINER_NAME}"
        docker stop "${CONTAINER_NAME}"
        docker rm "${CONTAINER_NAME}"
        log_success "Container stopped and removed"
    else
        log_warning "No running container found: ${CONTAINER_NAME}"
    fi
}

# Show logs
show_logs() {
    if docker ps -a -q -f name="${CONTAINER_NAME}" > /dev/null 2>&1; then
        docker logs -f "${CONTAINER_NAME}"
    else
        log_error "Container not found: ${CONTAINER_NAME}"
        exit 1
    fi
}

# Check status
check_status() {
    if docker ps -q -f name="${CONTAINER_NAME}" > /dev/null 2>&1; then
        log_success "Container is running: ${CONTAINER_NAME}"
        docker ps -f name="${CONTAINER_NAME}" --format "table {{.Names}}\t{{.Status}}\t{{.Ports}}"
        echo ""
        log_info "Resource usage:"
        docker stats --no-stream "${CONTAINER_NAME}"
    else
        log_warning "Container is not running: ${CONTAINER_NAME}"
    fi
}

# Open shell
open_shell() {
    if docker ps -q -f name="${CONTAINER_NAME}" > /dev/null 2>&1; then
        log_info "Opening shell in container..."
        docker exec -it "${CONTAINER_NAME}" /bin/bash
    else
        log_error "Container is not running: ${CONTAINER_NAME}"
        exit 1
    fi
}

# Clean up
cleanup() {
    log_info "Cleaning up Docker resources..."
    
    # Remove stopped containers
    docker container prune -f
    
    # Remove dangling images
    docker image prune -f
    
    log_success "Cleanup completed"
}

# Main
main() {
    cd "${PROJECT_ROOT}"
    
    if [ $# -eq 0 ]; then
        show_usage
        exit 0
    fi
    
    COMMAND=$1
    shift
    
    case $COMMAND in
        build)
            build_image
            ;;
        run)
            run_container "$@"
            ;;
        deploy)
            deploy_service "$@"
            ;;
        stop)
            stop_container
            ;;
        logs)
            show_logs
            ;;
        status)
            check_status
            ;;
        shell)
            open_shell
            ;;
        clean)
            cleanup
            ;;
        help|--help|-h)
            show_usage
            ;;
        *)
            log_error "Unknown command: $COMMAND"
            show_usage
            exit 1
            ;;
    esac
}

main "$@"
