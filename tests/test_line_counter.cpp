#include <gtest/gtest.h>
#include "line_counter.h"
#include "tracker.h"
#include "test_helpers.h"

namespace
{
TrackedObject track(int id, int prev_y, int cur_y)
{
    TrackedObject t;
    t.id = id;
    t.previous_center = cv::Point(100, prev_y);
    t.center = cv::Point(100, cur_y);
    t.box = cv::Rect(90, cur_y - 20, 20, 40);
    t.frames_since_seen = 0;
    return t;
}
} // namespace

TEST(LineCounter, StartsAtZero)
{
    LineCounter counter(200);
    EXPECT_EQ(counter.count_in(), 0);
    EXPECT_EQ(counter.count_out(), 0);
    EXPECT_EQ(counter.total(), 0);
    EXPECT_EQ(counter.line_y(), 200);
}

TEST(LineCounter, DownwardCrossingCountsIn)
{
    LineCounter counter(200);
    EXPECT_EQ(counter.update(track(1, 190, 210)), Crossing::In);
    EXPECT_EQ(counter.count_in(), 1);
    EXPECT_EQ(counter.count_out(), 0);
}

TEST(LineCounter, UpwardCrossingCountsOut)
{
    LineCounter counter(200);
    EXPECT_EQ(counter.update(track(1, 210, 190)), Crossing::Out);
    EXPECT_EQ(counter.count_in(), 0);
    EXPECT_EQ(counter.count_out(), 1);
}

TEST(LineCounter, MovingWithoutCrossingIsNotCounted)
{
    LineCounter counter(200);
    EXPECT_EQ(counter.update(track(1, 100, 150)), Crossing::None);  // stays above
    EXPECT_EQ(counter.update(track(2, 300, 250)), Crossing::None);  // stays below
    EXPECT_EQ(counter.total(), 0);
}

TEST(LineCounter, StationaryObjectIsNotCounted)
{
    LineCounter counter(200);
    EXPECT_EQ(counter.update(track(1, 200, 200)), Crossing::None);
    EXPECT_EQ(counter.total(), 0);
}

TEST(LineCounter, LandingExactlyOnLineFromAboveCountsIn)
{
    LineCounter counter(200);
    EXPECT_EQ(counter.update(track(1, 190, 200)), Crossing::In);
}

TEST(LineCounter, LandingExactlyOnLineFromBelowCountsOut)
{
    LineCounter counter(200);
    EXPECT_EQ(counter.update(track(1, 210, 200)), Crossing::Out);
}

TEST(LineCounter, StartingOnLineAndMovingAwayIsNotCounted)
{
    LineCounter counter(200);
    EXPECT_EQ(counter.update(track(1, 200, 220)), Crossing::None);
    EXPECT_EQ(counter.update(track(2, 200, 180)), Crossing::None);
    EXPECT_EQ(counter.total(), 0);
}

TEST(LineCounter, SameIdIsCountedOnlyOnce)
{
    LineCounter counter(200);
    EXPECT_EQ(counter.update(track(7, 190, 210)), Crossing::In);
    // Jitters back and forth across the line afterwards.
    EXPECT_EQ(counter.update(track(7, 210, 190)), Crossing::None);
    EXPECT_EQ(counter.update(track(7, 190, 210)), Crossing::None);
    EXPECT_EQ(counter.count_in(), 1);
    EXPECT_EQ(counter.count_out(), 0);
}

TEST(LineCounter, DifferentIdsAreCountedIndependently)
{
    LineCounter counter(200);
    counter.update(track(1, 190, 210));
    counter.update(track(2, 190, 210));
    counter.update(track(3, 210, 190));
    EXPECT_EQ(counter.count_in(), 2);
    EXPECT_EQ(counter.count_out(), 1);
    EXPECT_EQ(counter.total(), 3);
}

// --- tracker + counter on synthetic trajectories -----------------------------

namespace
{
// Runs a vertical walk (x fixed) through tracker + counter, one detection per frame.
void walk(SimpleTracker &tracker, LineCounter &counter, int x, int y_from, int y_to, int step)
{
    int dir = (y_to >= y_from) ? 1 : -1;
    for (int y = y_from; dir * (y_to - y) >= 0; y += dir * step)
        for (const auto &obj : tracker.update({person_at(x, y)}))
            counter.update(obj);
}
} // namespace

TEST(TrackerWithLineCounter, PersonWalkingDownIsCountedOnceAsIn)
{
    SimpleTracker tracker;
    LineCounter counter(240);
    walk(tracker, counter, 300, 100, 400, 10);
    EXPECT_EQ(counter.count_in(), 1);
    EXPECT_EQ(counter.count_out(), 0);
}

TEST(TrackerWithLineCounter, PersonWalkingUpIsCountedOnceAsOut)
{
    SimpleTracker tracker;
    LineCounter counter(240);
    walk(tracker, counter, 300, 400, 100, 10);
    EXPECT_EQ(counter.count_in(), 0);
    EXPECT_EQ(counter.count_out(), 1);
}

TEST(TrackerWithLineCounter, PersonWalkingAlongTheLineSideIsNotCounted)
{
    SimpleTracker tracker;
    LineCounter counter(240);
    for (int x = 50; x <= 500; x += 10)
        for (const auto &obj : tracker.update({person_at(x, 100)}))
            counter.update(obj);
    EXPECT_EQ(counter.total(), 0);
}

TEST(TrackerWithLineCounter, PersonTurningBackBeforeLineIsNotCounted)
{
    SimpleTracker tracker;
    LineCounter counter(240);
    walk(tracker, counter, 300, 100, 220, 10);
    walk(tracker, counter, 300, 220, 100, 10);
    EXPECT_EQ(counter.total(), 0);
}

TEST(TrackerWithLineCounter, TwoPeopleCrossingInOppositeDirections)
{
    SimpleTracker tracker;
    LineCounter counter(240);
    for (int step = 0; step <= 30; ++step)
    {
        // Person A walks down at x=100, person B walks up at x=500.
        auto tracks = tracker.update({person_at(100, 100 + step * 10), person_at(500, 400 - step * 10)});
        for (const auto &obj : tracks)
            counter.update(obj);
    }
    EXPECT_EQ(counter.count_in(), 1);
    EXPECT_EQ(counter.count_out(), 1);
}
