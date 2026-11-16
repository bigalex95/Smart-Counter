#pragma once
#include <set>
#include <vector>
#include "tracker.h"

enum class Crossing
{
    None,
    In,  // moved from above the line to on/below it (downward)
    Out  // moved from below the line to on/above it (upward)
};

// Counts tracked objects crossing a horizontal line at y = line_y.
// Each track ID is counted at most once, in whichever direction it crosses first.
class LineCounter
{
public:
    explicit LineCounter(int line_y);

    // Classifies one object's movement (previous_center -> center) and updates counters.
    Crossing update(const TrackedObject &obj);

    int line_y() const { return line_y_; }
    int count_in() const { return count_in_; }
    int count_out() const { return count_out_; }
    int total() const { return count_in_ + count_out_; }

private:
    int line_y_;
    int count_in_ = 0;
    int count_out_ = 0;
    std::set<int> counted_ids_;
};
