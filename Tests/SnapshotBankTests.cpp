#include <SnapshotBank.h>

#include <gtest/gtest.h>

namespace
{
class CountingListener : public SnapshotBank::Listener
{
public:
    void snapshotsChanged(const SnapshotBank &) override
    {
        ++calls;
    }

    int calls = 0;
};
} // namespace

TEST(SnapshotBank, StoreFillsOnlyThatSlotAndNotifies)
{
    SnapshotBank     bank;
    CountingListener listener;
    bank.addListener(&listener);

    bank.store(2, {0.5f, 1.5f});

    ASSERT_TRUE(bank.at(2).has_value());
    EXPECT_EQ(*bank.at(2), (std::vector<float> {0.5f, 1.5f}));
    EXPECT_FALSE(bank.at(3).has_value());
    EXPECT_EQ(listener.calls, 1);
}

TEST(SnapshotBank, ResetOnModelChange)
{
    SnapshotBank bank;
    bank.store(0, {1.0f});
    bank.store(9, {2.0f});

    bank.clear();

    for (std::size_t slot = 0; slot < SnapshotBank::kSlots; ++slot)
    {
        EXPECT_FALSE(bank.at(slot).has_value());
    }
}
