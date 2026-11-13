#include <SweepScheduler.h>

#include <gtest/gtest.h>

class SweepSchedulerTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        scheduler.setSubmitter([this](const SweepJob &job) {
            submitted.push_back(job);
        });
    }

    double                now = 0.0;
    SweepScheduler        scheduler {[this] {
        return now;
    }};
    std::vector<SweepJob> submitted;
};

TEST_F(SweepSchedulerTest, WaitsUntilRequestsStopForTheDebounce)
{
    scheduler.request({0, {1.0f}, 100.0f, -60.0f});
    now = 60.0;
    scheduler.request({0, {2.0f}, 100.0f, -60.0f});
    now = 120.0;
    scheduler.tick();
    EXPECT_TRUE(submitted.empty());

    now = 160.0;
    scheduler.tick();
    ASSERT_EQ(submitted.size(), 1U);
    EXPECT_FLOAT_EQ(submitted.front().z[0], 2.0f);
}

TEST_F(SweepSchedulerTest, GenerationsIncreaseAndOnlyTheLastIsLatest)
{
    scheduler.request({0, {1.0f}, 100.0f, -60.0f});
    now = 100.0;
    scheduler.tick();
    scheduler.request({0, {2.0f}, 100.0f, -60.0f});
    now = 200.0;
    scheduler.tick();

    ASSERT_EQ(submitted.size(), 2U);
    EXPECT_LT(submitted[0].generation, submitted[1].generation);
    EXPECT_FALSE(scheduler.isLatest(submitted[0].generation));
    EXPECT_TRUE(scheduler.isLatest(submitted[1].generation));
}

TEST_F(SweepSchedulerTest, NothingPendingSubmitsNothing)
{
    now = 1'000.0;
    scheduler.tick();

    EXPECT_TRUE(submitted.empty());
}

TEST_F(SweepSchedulerTest, InvalidateDropsThePendingAndMakesSubmittedStale)
{
    scheduler.request({0, {1.0f}, 100.0f, -60.0f});
    now = 200.0;
    scheduler.tick();
    scheduler.request({0, {2.0f}, 100.0f, -60.0f});

    scheduler.invalidate();
    now = 400.0;
    scheduler.tick();

    ASSERT_EQ(submitted.size(), 1U);
    EXPECT_FALSE(scheduler.isLatest(submitted.front().generation));
}
