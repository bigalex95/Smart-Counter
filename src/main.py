import cv2
from ultralytics import YOLO  # type: ignore

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

# 3. Start infinite loop
while True:
    # Read frame
    ret, frame = cap.read()

    # If frame was not read (end of video) - break the loop
    if not ret:
        break

    # Run frame through model.track with persist=True
    results = model.track(frame, persist=True)

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
        break

# Release resources
cap.release()
cv2.destroyAllWindows()
