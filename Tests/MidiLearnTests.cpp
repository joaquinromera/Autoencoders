#include <MidiLearn.h>

#include <gtest/gtest.h>

class MidiLearnTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        mapping.applyDefaults(
            MidiCcDefaults::fallback(2),
            {
                {0.0f, 1.0f},
                {0.0f, 1.0f}
        }
        );
        learn.onLearned = [this](MidiTarget target, int cc) {
            learned.emplace_back(target, cc);
        };
    }

    MidiParameterMapping                    mapping;
    std::vector<std::function<void()>>      posted; ///< Stands in for the message thread
    MidiLearn                               learn {mapping, [this](std::function<void()> f) {
                         posted.push_back(std::move(f));
                                                   }};
    std::vector<std::pair<MidiTarget, int>> learned;
};

TEST_F(MidiLearnTest, NextCcBindsLearningTarget)
{
    learn.beginLearning(MidiTarget::latentDim(1));

    learn.offer(42);
    for (auto &call : posted)
    {
        call();
    }

    EXPECT_EQ(mapping.controllerFor(MidiTarget::latentDim(1)), 42);
    ASSERT_EQ(learned.size(), 1U);
    EXPECT_EQ(learned.front().second, 42);
    EXPECT_FALSE(learn.learning().has_value());
}

TEST_F(MidiLearnTest, IdleOfferIsNotConsumed)
{
    EXPECT_FALSE(learn.offer(1));
    EXPECT_TRUE(posted.empty());
    EXPECT_EQ(mapping.controllerFor(MidiTarget::latentDim(0)), 1);
}

TEST_F(MidiLearnTest, LearningConsumesTheCc)
{
    learn.beginLearning(MidiTarget::xMaxControl());

    EXPECT_TRUE(learn.offer(5));
    EXPECT_FALSE(learn.offer(6)); // learning ends with the first controller
}

TEST_F(MidiLearnTest, CancelStopsLearning)
{
    learn.beginLearning(MidiTarget::xMaxControl());
    learn.cancelLearning();

    EXPECT_FALSE(learn.offer(5));
}
