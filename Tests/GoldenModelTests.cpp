// The analysis on duhast.json against the numpy reference in
// Extras/ui-proposals/fable5.1-max/no-playback (build_data.py, conditional.py).
// Skipped when the model is not checked out. Tolerances cover the different
// random frames std::mt19937 picks compared with numpy.

#include <DecodedLevel.h>
#include <DecoderFactory.h>
#include <ModelFileReader.h>
#include <PieceSpectra.h>
#include <SoundSweep.h>
#include <TrackAnalysis.h>

#include <gtest/gtest.h>

#include <chrono>
#include <filesystem>

class GoldenDuhast : public ::testing::Test
{
protected:
    static void SetUpTestSuite()
    {
        const auto path = std::string(AUTOENCODERS_MODELS_DIR) + "/duhast.json";
        if (std::filesystem::exists(path))
        {
            file     = std::make_unique<ModelFile>(ModelFileReader::read(path));
            analysis = std::make_unique<TrackAnalysis>(file->track, file->descriptor.zRange);
            decoder  = DecoderFactory::create(file->decoderSource);
        }
    }

    static void TearDownTestSuite()
    {
        analysis.reset();
        decoder.reset();
        file.reset();
    }

    void SetUp() override
    {
        if (!file)
        {
            GTEST_SKIP() << "duhast.json not found";
        }
    }

    static double levelAt(const std::vector<float> &z)
    {
        std::vector<float> pred;
        decoder->decodeInto(z, pred);
        return levelDb(pred, file->descriptor.defaultXMax, file->descriptor.defaultSClip);
    }

    static std::vector<float> frame(std::size_t f)
    {
        const auto values = file->track->frame(f);
        return {values.begin(), values.end()};
    }

    static inline std::unique_ptr<ModelFile>     file;
    static inline std::unique_ptr<TrackAnalysis> analysis;
    static inline std::unique_ptr<IDecoder>      decoder;
};

TEST_F(GoldenDuhast, VarianceRatios)
{
    const auto &p = analysis->projection();
    EXPECT_NEAR(p.varianceRatio(0), 0.7605, 0.01);
    EXPECT_NEAR(p.varianceRatio(1), 0.1182, 0.01);
    EXPECT_NEAR(p.varianceRatio(2), 0.0506, 0.01);
    EXPECT_NEAR(p.varianceRatio(3), 0.0426, 0.01);
}

TEST_F(GoldenDuhast, MapBoundsAreThePieceRobustSpan)
{
    EXPECT_NEAR(analysis->projection().bounds(0).lo, -3.3567, 1e-3);
    EXPECT_NEAR(analysis->projection().bounds(0).hi, 10.9594, 1e-3);
    EXPECT_NEAR(analysis->projection().bounds(1).lo, -1.4225, 1e-3);
    EXPECT_NEAR(analysis->projection().bounds(1).hi, 3.7962, 1e-3);
}

TEST_F(GoldenDuhast, MapReferences)
{
    EXPECT_NEAR(analysis->sliceFor({MapAxes::Space::principal, 0, 1}).reference(), 0.057, 0.015);
    EXPECT_NEAR(analysis->sliceFor({MapAxes::Space::principal, 2, 3}).reference(), 0.280, 0.015);
}

TEST_F(GoldenDuhast, LevelOfFrame2060AndWithZ5AtItsMaximum)
{
    auto z = frame(2'060);
    EXPECT_NEAR(levelAt(z), 42.87, 0.5);

    z[4] = file->descriptor.zRange[4].max;
    EXPECT_NEAR(levelAt(z), 12.44, 1.0);
}

TEST_F(GoldenDuhast, PieceLevelRange)
{
    const auto &d     = file->descriptor;
    const auto  range = PieceSpectra::decode(*decoder, *file->track).levels(d.defaultXMax, d.defaultSClip);

    EXPECT_NEAR(range.lo, 29.66, 1.5);
    EXPECT_NEAR(range.hi, 45.73, 1.0);
}

TEST_F(GoldenDuhast, SweepOfAllSlidersTakesLittleTime)
{
    const auto &d     = file->descriptor;
    const auto  z     = frame(2'060);
    const auto  start = std::chrono::steady_clock::now();
    for (std::size_t dim = 0; dim < d.latentDim; ++dim)
    {
        SoundSweep::along(*decoder, z, dim, d.zRange[dim], d.defaultXMax, d.defaultSClip);
    }
    const auto ms = std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - start).count();

    RecordProperty("sweepMs", std::to_string(ms));
    std::cout << "[ info     ] 256 decodes took " << ms << " ms\n";
    EXPECT_LT(ms, 500.0);
}
