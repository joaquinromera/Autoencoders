#include <LatestValueMailbox.h>

#include <gtest/gtest.h>

#include <algorithm>
#include <array>
#include <atomic>
#include <thread>

TEST(LatestValueMailbox, ReadWithoutWriteReturnsFalse)
{
    LatestValueMailbox<int> mailbox(7);
    int                     value = 0;

    EXPECT_FALSE(mailbox.readLatest(value));
    EXPECT_EQ(value, 0);
}

TEST(LatestValueMailbox, ReadReturnsLatestOfSeveralWrites)
{
    LatestValueMailbox<int> mailbox(0);
    mailbox.write(1);
    mailbox.write(2);
    mailbox.write(3);

    int value = 0;
    EXPECT_TRUE(mailbox.readLatest(value));
    EXPECT_EQ(value, 3);
    EXPECT_FALSE(mailbox.readLatest(value));
}

TEST(LatestValueMailbox, ConcurrentWriterReaderNeverTears)
{
    using Block           = std::array<int, 64>;
    constexpr int kWrites = 200'000;

    LatestValueMailbox<Block> mailbox(Block {});
    std::atomic<bool>         done {false};

    std::thread writer([&] {
        for (int i = 1; i <= kWrites; ++i)
        {
            Block block;
            block.fill(i);
            mailbox.write(block);
        }
        done = true;
    });

    int   lastSeen = 0;
    Block block {};
    bool  torn = false;
    while (true)
    {
        // Read done first: if it was set and nothing is fresh, nothing will be.
        const bool writerFinished = done;
        if (mailbox.readLatest(block))
        {
            torn     |= std::any_of(block.begin(), block.end(), [&](int v) {
                return v != block.front();
            });
            torn     |= block.front() < lastSeen;
            lastSeen  = block.front();
        }
        else if (writerFinished)
        {
            break;
        }
    }
    writer.join();

    EXPECT_FALSE(torn);
    EXPECT_EQ(lastSeen, kWrites);
}
