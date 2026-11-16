# Docker

Two services are defined in `docker-compose.yml` / `docker-compose-gpu.yml`:

- `detector`: the C++ app (`Dockerfile`, base image `nvidia/cuda:12.9.1-cudnn-devel-ubuntu22.04`).
- `dashboard`: Streamlit (`dashboard/Dockerfile`, `python:3.9-slim`), port 8501.

They share `./logs` (SQLite file); `./data` and `./models` are mounted into the detector.

> Status: the Docker setup was written for an NVIDIA machine. It was not re-built or run as part of the latest cleanup, so treat it as unverified on your hardware.

## Before `docker compose up`

Two inputs are not stored in git and must exist locally before the image is built:

1. **ONNX Runtime (GPU build)** under `third_party/` (the `Dockerfile` copies it into the image and `CMakeLists.txt` links against it):

   ```bash
   mkdir -p third_party
   wget https://github.com/microsoft/onnxruntime/releases/download/v1.23.2/onnxruntime-linux-x64-gpu-1.23.2.tgz
   tar -xzf onnxruntime-linux-x64-gpu-1.23.2.tgz -C third_party/
   ```

   The GPU package is required at link time even when you run with `--cpu` / `USE_CPU=true`.

2. **The ONNX model** `models/yolov8s.onnx` (see "Model" in the README: export with `python/convert.py`).

3. A **video** at `data/videos/` (default name `853889-hd_1920_1080_25fps.mp4`; override with `INPUT_VIDEO`). No video ships with the repo.

## Run

```bash
cp .env.example .env            # optional, see variables below
docker compose up --build       # detector + dashboard
docker compose -f docker-compose-gpu.yml up --build   # same, without the --cpu switch
```

Dashboard: <http://localhost:8501>. Stop with `docker compose down`.

The helper `./run compose-up | compose-up-gpu | compose-down | compose-logs [service] | compose-restart | compose-build` wraps the same commands.

Differences between the two compose files: `docker-compose.yml` always passes `--cpu` to the detector (`USE_CPU` defaults to `false`, which is a non-empty string, and the command only tests whether the variable is non-empty), so it runs CPU inference despite reserving a GPU; `docker-compose-gpu.yml` never passes `--cpu`. Use the `-gpu` file for GPU inference. Both reserve one NVIDIA GPU, so the NVIDIA Container Toolkit is needed in either case, and the image is CUDA-based.

## Environment variables

Detector:

| Variable | Default | Meaning |
| --- | --- | --- |
| `MODEL_PATH` | `models/yolov8s.onnx` | ONNX model |
| `INPUT_VIDEO` | `data/videos/853889-hd_1920_1080_25fps.mp4` | input video |
| `OUTPUT_VIDEO` | `data/output/output.mp4` | annotated output (headless mode) |
| `DB_PATH` | `logs/analytics.db` | SQLite file |
| `HEADLESS_MODE` | `true` | no window, write the output video |
| `LOOP_VIDEO` | `true` | restart the video when it ends |
| `USE_CPU` | `false` | only used by `docker-compose.yml`; any non-empty value (including `false`) adds `--cpu` |
| `DISPLAY` | `:0` | X11 display (for non-headless runs; run `xhost +local:docker`) |

Dashboard:

| Variable | Default | Meaning |
| --- | --- | --- |
| `DASHBOARD_PORT` | `8501` | host port |
| `DASHBOARD_DB_PATH` | `/app/logs/analytics.db` | SQLite path inside the container |
| `REFRESH_INTERVAL` | `2` | seconds between refreshes |
| `DATA_LIMIT` | `100` | rows loaded from the DB |

`HEADLESS_MODE` and `LOOP_VIDEO` behave the same way: they default to `true` and any non-empty value (including `false`) enables the flag, so to run with a window or without looping, edit the `command:` in the compose file or run the binary directly.

## Troubleshooting

- GPU not visible in the container: `docker run --rm --gpus all nvidia/cuda:12.9.1-base-ubuntu22.04 nvidia-smi`.
- `COPY third_party/` fails during build: ONNX Runtime was not extracted to `third_party/` (see above).
- Dashboard shows no data: the detector writes a row only on a counted crossing; check `docker compose logs detector` and that `./logs` is mounted.
- No window in non-headless mode: set `DISPLAY`, run `xhost +local:docker`.
