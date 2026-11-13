#include <NearnessLabel.h>

#include <gtest/gtest.h>

TEST(NearnessLabel, Thresholds)
{
    EXPECT_EQ(NearnessLabel::from(0.5).kind, NearnessLabel::Kind::on);
    EXPECT_EQ(NearnessLabel::from(0.24).kind, NearnessLabel::Kind::near);
    EXPECT_EQ(NearnessLabel::from(0.24).percent, 24);
    EXPECT_EQ(NearnessLabel::from(0.049).kind, NearnessLabel::Kind::off);
}
