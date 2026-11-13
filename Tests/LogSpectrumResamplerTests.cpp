#include <LogSpectrumResampler.h>

#include <gtest/gtest.h>

#include <vector>

TEST(LogSpectrumResampler, EndpointsAre55HzAndNyquist)
{
    const LogSpectrumResampler resampler(1'025, 22'050.0);

    ASSERT_EQ(resampler.frequencies().size(), 150U);
    EXPECT_NEAR(resampler.frequencies().front(), 55.0, 1e-9);
    EXPECT_NEAR(resampler.frequencies().back(), 11'025.0, 1e-6);
}

TEST(LogSpectrumResampler, InterpolatesBetweenBins)
{
    std::vector<float> ramp;
    for (int i = 0; i < 1'025; ++i)
    {
        ramp.push_back(static_cast<float>(i));
    }
    const LogSpectrumResampler resampler(1'025, 22'050.0);
    std::vector<float>         out;

    resampler.resampleInto(ramp, out);

    EXPECT_NEAR(out.front(), 55.0 / 11'025.0 * 1'024.0, 1e-3); // a ramp maps frequency to bin index
    EXPECT_NEAR(out.back(), 1'024.0, 1e-3);
}
