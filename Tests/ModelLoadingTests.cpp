// Loads the example models shipped in Extras/models through the real
// decoders. Skipped when the files are not checked out.

#include <AutoencoderFactory.h>
#include <ControlRanges.h>
#include <StartingLatent.h>

#include <gtest/gtest.h>

#include <filesystem>

namespace
{
std::string modelPath(const std::string &name)
{
    return std::string(AUTOENCODERS_MODELS_DIR) + "/" + name;
}

float rmsAfterLoading(const LoadedModel &model)
{
    const auto ranges = ControlRanges::forModel(model.descriptor);
    model.engine->setLatent(startingLatent(ranges, model.track.get()));

    juce::AudioBuffer<float>     buffer(2, 512);
    juce::AudioSourceChannelInfo info {&buffer, 0, 512};
    SpectrumFrame                frame;
    for (int block = 0; block < 8; ++block)
    {
        model.engine->getNextAudioBlock(info);
    }
    model.engine->readLatestSpectrum(frame);
    return frame.rms;
}
} // namespace

class ModelLoading : public ::testing::TestWithParam<const char *>
{
};

TEST_P(ModelLoading, LoadsAndIsAudibleAtTheModelDefaults)
{
    const auto path = modelPath(GetParam());
    if (!std::filesystem::exists(path))
    {
        GTEST_SKIP() << path << " not found";
    }

    const auto model = AutoencoderFactory::createFromFile(path);

    EXPECT_EQ(model.engine->getLatentDim(), model.descriptor.latentDim);
    EXPECT_GT(model.descriptor.defaultXMax, 90.0f);
    EXPECT_GT(rmsAfterLoading(model), 1e-3f);
}

INSTANTIATE_TEST_SUITE_P(
    ExampleModels, ModelLoading, ::testing::Values("duhast.json", "debupytorch.json", "beethoven.json")
);

TEST(ModelLoadingDetails, DuhastCarriesItsTrackAndCcDefaults)
{
    const auto path = modelPath("duhast.json");
    if (!std::filesystem::exists(path))
    {
        GTEST_SKIP() << path << " not found";
    }

    const auto model = AutoencoderFactory::createFromFile(path);

    ASSERT_NE(model.track, nullptr);
    EXPECT_EQ(model.track->frames(), 5'168U);
    EXPECT_NEAR(model.descriptor.defaultXMax, 107.55f, 0.01f);
    EXPECT_EQ(model.descriptor.ccDefaults.sliderCcs.size(), 8U);
    EXPECT_NEAR(model.descriptor.durationSeconds, 5'168 * 512 / 22'050.0, 1e-6);
}

TEST(ModelLoadingDetails, BeethovenHasNoTrack)
{
    const auto path = modelPath("beethoven.json");
    if (!std::filesystem::exists(path))
    {
        GTEST_SKIP() << path << " not found";
    }

    EXPECT_EQ(AutoencoderFactory::createFromFile(path).track, nullptr);
}
