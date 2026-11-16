#pragma once
#include <vector>
#include "detection.h"

// Person detection (COCO class 0) whose box centre is exactly (cx, cy).
inline Detection person_at(int cx, int cy, int class_id = 0)
{
    return Detection{class_id, 0.9f, cv::Rect(cx - 10, cy - 20, 20, 40)};
}
