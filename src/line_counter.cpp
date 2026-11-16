#include "line_counter.h"

LineCounter::LineCounter(int line_y) : line_y_(line_y) {}

Crossing LineCounter::update(const TrackedObject &obj)
{
    if (counted_ids_.count(obj.id))
        return Crossing::None;

    // Was above the line, is now on/below it -> IN (downward)
    if (obj.previous_center.y < line_y_ && obj.center.y >= line_y_)
    {
        ++count_in_;
        counted_ids_.insert(obj.id);
        return Crossing::In;
    }

    // Was below the line, is now on/above it -> OUT (upward)
    if (obj.previous_center.y > line_y_ && obj.center.y <= line_y_)
    {
        ++count_out_;
        counted_ids_.insert(obj.id);
        return Crossing::Out;
    }

    return Crossing::None;
}
