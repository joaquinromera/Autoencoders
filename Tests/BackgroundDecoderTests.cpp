// Runs the worker on a frugally-deep and a TorchScript model; skipped when
// the models are not checked out.

#include <AutoencoderFactory.h>
#include <BackgroundDecoder.h>
#include <ModelFileReader.h>

#include <gtest/gtest.h>

#include <array>
#include <atomic>
#include <chrono>
#include <cmath>
#include <filesystem>
#include <iostream>
#include <thread>

namespace
{
std::string modelPath(const std::string &name)
{
    return std::string(AUTOENCODERS_MODELS_DIR) + "/" + name;
}

/// Runs the message loop until done() or the timeout passes
bool dispatchUntil(const std::function<bool()> &done, int timeoutMs = 10'000)
{
    const auto end = juce::Time::getMillisecondCounter() + static_cast<juce::uint32>(timeoutMs);
    while (!done() && juce::Time::getMillisecondCounter() < end)
    {
        juce::MessageManager::getInstance()->runDispatchLoopUntil(10);
    }
    return done();
}
} // namespace

class BackgroundDecoderOnModel : public ::testing::TestWithParam<const char *>
{
protected:
    void SetUp() override
    {
        if (!std::filesystem::exists(modelPath(GetParam())))
        {
            GTEST_SKIP() << GetParam() << " not found";
        }
    }
};

TEST_P(BackgroundDecoderOnModel, AnswersASweepWithThePiecesLevelsAtItsFloorAndRange)
{
    const auto  file = ModelFileReader::read(modelPath(GetParam()));
    const auto &d    = file.descriptor;

    std::optional<SweepResult> sweeps;
    BackgroundDecoder          worker(file.decoderSource, file.track, d.zRange, {[&](const SweepResult &r) {
                                 sweeps = r;
                                      }});
    const auto                 frame   = file.track->frame(2'060);
    const auto                 sweepAt = [&](std::uint64_t generation, float sClip) {
        sweeps.reset();
        worker.submit({
            generation,
            {frame.begin(), frame.end()},
            d.defaultXMax,
            sClip
        });
        return dispatchUntil([&] {
            return sweeps.has_value();
        });
    };

    ASSERT_TRUE(sweepAt(7, d.defaultSClip));
    EXPECT_EQ(sweeps->generation, 7U);
    ASSERT_EQ(sweeps->profiles.size(), d.latentDim);
    for (const auto &profile : sweeps->profiles)
    {
        EXPECT_EQ(profile.levels.size(), 32U);
        EXPECT_EQ(profile.tonality.size(), 32U);
    }
    EXPECT_LT(sweeps->pieceLevels.lo, sweeps->pieceLevels.hi);
    const auto atDefault = sweeps->pieceLevels;

    ASSERT_TRUE(sweepAt(8, d.defaultSClip - 20.0f));
    EXPECT_NEAR(sweeps->pieceLevels.hi, atDefault.hi - 20.0, 1e-6);
}

TEST_P(BackgroundDecoderOnModel, SweepsWhileTheEngineDecodes)
{
    // The engine and the worker each have their own decoder; both run at once.
    const auto loaded = AutoencoderFactory::createFromFile(modelPath(GetParam()));
    const auto frame  = loaded.track->frame(loaded.track->frames() / 3);
    loaded.engine->setLatent({frame.begin(), frame.end()});

    std::atomic<bool> playing {true};
    std::thread       audio([&] {
        juce::AudioBuffer<float> buffer(2, 512);
        while (playing)
        {
            juce::AudioSourceChannelInfo info {&buffer, 0, 512};
            loaded.engine->getNextAudioBlock(info);
        }
    });

    const auto                &d = loaded.descriptor;
    std::optional<SweepResult> sweeps;
    BackgroundDecoder          worker(loaded.decoderSource, loaded.track, d.zRange, {[&](const SweepResult &r) {
                                 sweeps = r;
                                      }});
    const auto                 start = std::chrono::steady_clock::now();
    for (std::uint64_t generation = 1; generation <= 3; ++generation)
    {
        sweeps.reset();
        worker.submit({
            generation,
            {frame.begin(), frame.end()},
            d.defaultXMax,
            d.defaultSClip
        });
        ASSERT_TRUE(dispatchUntil([&] {
            return sweeps.has_value();
        }));
        EXPECT_EQ(sweeps->generation, generation);
    }
    const auto ms = std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - start).count();
    playing       = false;
    audio.join();

    std::cout << "[ info     ] " << GetParam() << ": piece decode and 3 sweeps beside the engine took " << ms
              << " ms\n";
    for (const auto &profile : sweeps->profiles)
    {
        for (std::size_t k = 0; k < profile.levels.size(); ++k)
        {
            EXPECT_TRUE(std::isfinite(profile.levels[k]));
            EXPECT_GE(profile.tonality[k], 0.0);
            EXPECT_LE(profile.tonality[k], 1.0);
        }
    }
}

TEST_P(BackgroundDecoderOnModel, AnswersEachMapsGrid)
{
    const auto                                    file = ModelFileReader::read(modelPath(GetParam()));
    const auto                                   &d    = file.descriptor;
    std::array<std::optional<SoundGridResult>, 2> grids;
    BackgroundDecoder
               worker(file.decoderSource, file.track, d.zRange, {{}, [&](std::size_t map, const SoundGridResult &r) {
                                                              grids.at(map) = r;
                                                                 }});
    const auto frame = file.track->frame(file.track->frames() / 2);
    std::vector<std::vector<float>> points(9, {frame.begin(), frame.end()});
    worker.submitSoundGrid(0, {4, 3, points, d.defaultXMax, d.defaultSClip});
    worker.submitSoundGrid(1, {5, 3, points, d.defaultXMax, d.defaultSClip});

    ASSERT_TRUE(dispatchUntil([&] {
        return grids[0].has_value() && grids[1].has_value();
    }));
    EXPECT_EQ(grids[0]->generation, 4U);
    EXPECT_EQ(grids[1]->generation, 5U);
    ASSERT_EQ(grids[1]->levels.size(), 9U);
    EXPECT_DOUBLE_EQ(grids[1]->levels.front(), grids[1]->levels.back()); // same point, same sound
    EXPECT_LT(grids[1]->pieceLevels.lo, grids[1]->pieceLevels.hi);
}

INSTANTIATE_TEST_SUITE_P(
    FdeepAndTorchScript, BackgroundDecoderOnModel, ::testing::Values("duhast.json", "debupytorch.json")
);

TEST(BackgroundDecoder, RefusesToStartWithoutATrack)
{
    EXPECT_THROW(BackgroundDecoder(DecoderSource {}, nullptr, {}, {}), std::invalid_argument);
}
