# Docker Compose GPU vs CPU Mode

## Overview

The Smart Counter project now supports both CPU and GPU modes when using Docker Compose.

## Files Changed

### 1. `docker-compose.yml` (Default - CPU Mode)

- **Database Access**: Dashboard now has **read-write** access to the database (was read-only)
  - This allows the "Reset Counters" button in the dashboard to work properly
- **USE_CPU flag**: Set to `false` by default but the command includes `$${USE_CPU:+--cpu}` flag

### 2. `docker-compose-gpu.yml` (New - GPU Mode)

- Identical to `docker-compose.yml` but optimized for GPU usage
- **GPU settings**: Includes NVIDIA GPU reservation configuration
- **No CPU flag**: The `--cpu` flag is completely removed from the command
- **Database Access**: Dashboard has read-write access

### 3. `run` script updates

New commands added:

- `./run compose-up` - Start services in **CPU mode** (default)
- `./run compose-up-gpu` - Start services in **GPU mode** (new)
- `./run compose-down` - Stops both CPU and GPU compose instances
- `./run compose-logs` - Auto-detects which compose file is running
- `./run compose-restart` - Auto-detects which compose file is running
- `./run compose-build` - Builds both configurations

## Usage

### CPU Mode (Default)

```bash
./run compose-up
```

### GPU Mode (Recommended)

```bash
./run compose-up-gpu
```

### Stop Services

```bash
./run compose-down
```

### View Logs

```bash
./run compose-logs              # All services
./run compose-logs detector     # Detector only
./run compose-logs dashboard    # Dashboard only
```

## Problems Fixed

### Problem 1: Dashboard Database Read-Only Error ✅

**Issue**: Dashboard couldn't write to database when using "Reset Counters" button

**Solution**: Changed volume mount from `:ro` (read-only) to `:rw` (read-write) in both compose files:

```yaml
volumes:
  - ./logs:/app/logs:rw # Read-write access
```

### Problem 2: Docker Compose Running in CPU Mode ✅

**Issue**: Default `./run compose-up` command was using CPU instead of GPU

**Solution**:

1. Created separate `docker-compose-gpu.yml` for explicit GPU mode
2. Added `./run compose-up-gpu` command for GPU deployment
3. Kept default `compose-up` as CPU mode for compatibility
4. Removed `--cpu` flag from GPU compose file command

## Recommendations

For **production/development** with GPU available:

```bash
./run compose-up-gpu
```

For **testing without GPU** or on CPU-only machines:

```bash
./run compose-up
```

## Environment Variables

You can customize behavior using environment variables:

```bash
# Example: Use GPU with custom video
INPUT_VIDEO=data/videos/my_video.mp4 ./run compose-up-gpu

# Example: Custom dashboard port
DASHBOARD_PORT=8080 ./run compose-up-gpu
```

## Verification

Check if GPU is being used:

```bash
./run compose-logs detector
# Look for CUDA/GPU initialization messages

# Or check with nvidia-smi
nvidia-smi
# Should show SmartCounter process using GPU
```
