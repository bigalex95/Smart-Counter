#include <gtest/gtest.h>
#include "tracker.h"
#include "test_helpers.h"

TEST(SimpleTracker, EmptyInputGivesNoTracks)
{
    SimpleTracker tracker;
    EXPECT_TRUE(tracker.update({}).empty());
}

TEST(SimpleTracker, NewDetectionsGetUniqueSequentialIds)
{
    SimpleTracker tracker;
    auto tracks = tracker.update({person_at(100, 100), person_at(400, 300)});
    ASSERT_EQ(tracks.size(), 2u);
    EXPECT_EQ(tracks[0].id, 0);
    EXPECT_EQ(tracks[1].id, 1);
}

TEST(SimpleTracker, NewTrackHasPreviousCenterEqualToCenter)
{
    SimpleTracker tracker;
    auto tracks = tracker.update({person_at(100, 100)});
    ASSERT_EQ(tracks.size(), 1u);
    EXPECT_EQ(tracks[0].center, cv::Point(100, 100));
    EXPECT_EQ(tracks[0].previous_center, tracks[0].center);
}

TEST(SimpleTracker, IgnoresNonPersonClasses)
{
    SimpleTracker tracker;
    auto tracks = tracker.update({person_at(100, 100, /*class_id=*/2), person_at(300, 100, /*class_id=*/0)});
    ASSERT_EQ(tracks.size(), 1u);
    EXPECT_EQ(tracks[0].center, cv::Point(300, 100));
}

TEST(SimpleTracker, IdPersistsWhileObjectMovesWithinThreshold)
{
    SimpleTracker tracker; // threshold 50 px
    int id = tracker.update({person_at(100, 100)})[0].id;
    for (int step = 1; step <= 20; ++step)
    {
        auto tracks = tracker.update({person_at(100, 100 + step * 10)});
        ASSERT_EQ(tracks.size(), 1u);
        EXPECT_EQ(tracks[0].id, id);
        EXPECT_EQ(tracks[0].center, cv::Point(100, 100 + step * 10));
        EXPECT_EQ(tracks[0].previous_center, cv::Point(100, 100 + (step - 1) * 10));
    }
}

TEST(SimpleTracker, TwoSeparatedObjectsKeepTheirOwnIds)
{
    SimpleTracker tracker;
    auto first = tracker.update({person_at(100, 100), person_at(500, 100)});
    int left_id = first[0].id;
    int right_id = first[1].id;
    ASSERT_NE(left_id, right_id);

    for (int step = 1; step <= 10; ++step)
    {
        auto tracks = tracker.update({person_at(100 + step * 5, 100), person_at(500 - step * 5, 100)});
        ASSERT_EQ(tracks.size(), 2u);
        for (const auto &t : tracks)
        {
            if (t.center.x < 300)
                EXPECT_EQ(t.id, left_id);
            else
                EXPECT_EQ(t.id, right_id);
        }
    }
}

TEST(SimpleTracker, JumpBeyondThresholdCreatesNewId)
{
    SimpleTracker tracker(5, 50);
    int old_id = tracker.update({person_at(100, 100)})[0].id;
    // 200 px away, gate is 50 px: the old track is kept for its grace period and a new one is created.
    auto tracks = tracker.update({person_at(300, 100)});
    ASSERT_EQ(tracks.size(), 2u);
    for (const auto &t : tracks)
    {
        if (t.center == cv::Point(300, 100))
            EXPECT_NE(t.id, old_id);
        else
            EXPECT_EQ(t.id, old_id);
    }
}

TEST(SimpleTracker, CustomDistanceThresholdIsRespected)
{
    SimpleTracker tracker(5, 200);
    int id = tracker.update({person_at(100, 100)})[0].id;
    auto tracks = tracker.update({person_at(250, 100)}); // 150 px < 200 px gate
    ASSERT_EQ(tracks.size(), 1u);
    EXPECT_EQ(tracks[0].id, id);
}

TEST(SimpleTracker, MissedTrackIsReportedOnlyBrieflyThenHidden)
{
    SimpleTracker tracker;
    tracker.update({person_at(100, 100)});

    // 1st missed frame: still reported (grace period), 2nd missed frame: hidden.
    EXPECT_EQ(tracker.update({}).size(), 1u);
    EXPECT_TRUE(tracker.update({}).empty());
}

TEST(SimpleTracker, ObjectReappearingWithinGracePeriodKeepsId)
{
    SimpleTracker tracker(5, 50); // dropped when missed for more than 5 frames
    int id = tracker.update({person_at(100, 100)})[0].id;
    for (int i = 0; i < 5; ++i)
        tracker.update({});

    auto tracks = tracker.update({person_at(105, 100)});
    ASSERT_EQ(tracks.size(), 1u);
    EXPECT_EQ(tracks[0].id, id);
}

TEST(SimpleTracker, ObjectReappearingAfterTimeoutGetsNewId)
{
    SimpleTracker tracker(5, 50);
    int id = tracker.update({person_at(100, 100)})[0].id;
    for (int i = 0; i < 6; ++i)
        tracker.update({});

    auto tracks = tracker.update({person_at(100, 100)});
    ASSERT_EQ(tracks.size(), 1u);
    EXPECT_NE(tracks[0].id, id);
}

TEST(SimpleTracker, IdsAreNeverReused)
{
    SimpleTracker tracker(1, 50);
    int first = tracker.update({person_at(100, 100)})[0].id;
    for (int i = 0; i < 3; ++i)
        tracker.update({});
    int second = tracker.update({person_at(100, 100)})[0].id;
    EXPECT_GT(second, first);
}
