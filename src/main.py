import time
import cv2
from ultralytics import YOLO  # type: ignore
from utils.fps_utils import (
    measure_fps_from_seconds,
    measure_fps_from_milliseconds,
    ms_sum,
)

# 1. Load the model
model = YOLO("models/yolov8s.pt")

# 2. Open video file
cap = cv2.VideoCapture("data/videos/853889-hd_1920_1080_25fps.mp4")

# Check if video opened successfully
if not cap.isOpened():
    raise FileNotFoundError("Failed to open video file. Check the file path.")

# Get video dimensions
frame_width = int(cap.get(cv2.CAP_PROP_FRAME_WIDTH))
frame_height = int(cap.get(cv2.CAP_PROP_FRAME_HEIGHT))

# Define counting line position (middle of the frame)
line_y = frame_height // 2
line_tolerance = 20  # Zone around the line for detection

# Create set to store counted IDs
counted_ids = set()

# Lists to store timings
model_times_ms = []  # per-frame model time (ms) from results[0].speed
system_times_s = []  # per-frame full-loop time (seconds)
frame_count = 0

# 3. Start infinite loop
while True:

    loop_start = time.time()

    # Read frame
    ret, frame = cap.read()

    # If frame was not read (end of video) - break the loop
    if not ret:
        break

    # Run frame through model.track with persist=True
    results = model.track(frame, persist=True)

    # Try to extract per-stage times from results (ms) and accumulate
    speed = getattr(results[0], "speed", None)
    if speed is not None:
        total_model_ms = ms_sum(speed)
        model_times_ms.append(total_model_ms)
        print(
            f"Speed => pre: {speed.get('preprocess', 0.0):.2f}ms | "
            f"inf: {speed.get('inference', 0.0):.2f}ms | "
            f"post: {speed.get('postprocess', 0.0):.2f}ms | "
            f"total: {total_model_ms:.2f}ms"
        )
    else:
        print("Speed info not available in results[0].speed")

    # Get annotated frame
    annotated_frame = results[0].plot()

    # Default line color (green)
    line_color = (0, 255, 0)

    # Calculate centers of detected objects
    if results[0].boxes is not None:
        for box in results[0].boxes:
            # Check if track ID exists
            if box.id is None:
                continue

            track_id = int(box.id.item())

            x1, y1, x2, y2 = box.xyxy[0].cpu().numpy()
            cx = int((x1 + x2) / 2)
            cy = int((y1 + y2) / 2)

            # Draw center point
            cv2.circle(annotated_frame, (cx, cy), 5, (0, 255, 0), -1)

            # Check if center crossed the counting line
            if (
                line_y - line_tolerance < cy < line_y + line_tolerance
            ) and track_id not in counted_ids:
                # Add ID to counted set
                counted_ids.add(track_id)
                # Change line color to red for visual feedback
                line_color = (0, 0, 255)

    # Draw counting line
    cv2.line(annotated_frame, (0, line_y), (frame_width, line_y), line_color, 2)

    # Draw counting zone (tolerance area)
    cv2.line(
        annotated_frame,
        (0, line_y - line_tolerance),
        (frame_width, line_y - line_tolerance),
        (255, 255, 0),
        1,
    )
    cv2.line(
        annotated_frame,
        (0, line_y + line_tolerance),
        (frame_width, line_y + line_tolerance),
        (255, 255, 0),
        1,
    )

    # Display counter on frame
    cv2.putText(
        annotated_frame,
        f"Count: {len(counted_ids)}",
        (10, 50),
        cv2.FONT_HERSHEY_SIMPLEX,
        1.5,
        (0, 255, 255),
        3,
    )

    # Display result
    cv2.imshow("YOLOv8 Tracking", annotated_frame)

    # Wait for 'q' key press to exit
    if cv2.waitKey(1) & 0xFF == ord("q"):
        loop_end = time.time()
        system_times_s.append(loop_end - loop_start)
        break

    # End of loop timing
    loop_end = time.time()
    system_times_s.append(loop_end - loop_start)
    frame_count += 1

    # Periodic running averages
    if frame_count > 0 and frame_count % 60 == 0:
        running_model_fps = (
            measure_fps_from_milliseconds(model_times_ms) if model_times_ms else 0.0
        )
        running_system_fps = (
            measure_fps_from_seconds(system_times_s) if system_times_s else 0.0
        )
        print(
            f"Running (last {frame_count} frames) — model FPS: {running_model_fps:.2f}, system FPS: {running_system_fps:.2f}"
        )

# Release resources
cap.release()
cv2.destroyAllWindows()

# Compute and print averages
total_frames = max(1, frame_count)
avg_model_fps = measure_fps_from_milliseconds(model_times_ms) if model_times_ms else 0.0
avg_system_fps = measure_fps_from_seconds(system_times_s) if system_times_s else 0.0
print("--- Summary ---")
print(f"Frames processed: {frame_count}")
print(f"Average model FPS (from results[0].speed): {avg_model_fps:.2f}")
print(f"Average system FPS (full loop): {avg_system_fps:.2f}")
