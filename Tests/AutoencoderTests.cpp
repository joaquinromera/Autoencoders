#include <Autoencoder.h>
#include <MockDecoder.h>
#include <Parameters.h>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

using ::testing::_;
using ::testing::AtLeast;
using ::testing::ElementsAreArray;
using ::testing::NiceMock;
using ::testing::Return;
using ::testing::SetArgReferee;

class AutoencoderTest : public ::testing::Test
{
protected:
    static constexpr std::size_t latentDim = 8;
    static constexpr std::size_t outputDim = 2'049;
    static constexpr unsigned    winLength = 4'096;
    static constexpr int         blockSize = 512;

    void SetUp() override
    {
        makeEngine(Parameters {winLength, 100.0f, -60.0f});
    }

    void makeEngine(Parameters params, std::size_t decoderOutputDim = outputDim)
    {
        auto decoder = std::make_unique<NiceMock<MockDecoder>>();
        mockDecoder  = decoder.get();
        ON_CALL(*mockDecoder, latentDim()).WillByDefault(Return(latentDim));
        ON_CALL(*mockDecoder, outputDim()).WillByDefault(Return(decoderOutputDim));
        decodeTo(0.1f);
        autoencoder = std::make_unique<Autoencoder>(std::move(decoder), params, ModelBackend::FrugallyDeep);
    }

    void decodeTo(float value)
    {
        ON_CALL(*mockDecoder, decodeInto(_, _)).WillByDefault(SetArgReferee<1>(std::vector<float>(outputDim, value)));
    }

    float playBlockAndReadRms()
    {
        juce::AudioBuffer<float>     buffer(2, blockSize);
        juce::AudioSourceChannelInfo info {&buffer, 0, blockSize};
        autoencoder->getNextAudioBlock(info);

        SpectrumFrame frame;
        EXPECT_TRUE(autoencoder->readLatestSpectrum(frame));
        return frame.rms;
    }

    NiceMock<MockDecoder>       *mockDecoder = nullptr;
    std::unique_ptr<Autoencoder> autoencoder;
};

TEST_F(AutoencoderTest, InitializesWithMockDecoder)
{
    EXPECT_EQ(autoencoder->getLatentDim(), latentDim);
    EXPECT_EQ(autoencoder->getBackend(), ModelBackend::FrugallyDeep);
}

TEST_F(AutoencoderTest, RejectsDecoderWhoseOutputDoesNotMatchTheWindow)
{
    EXPECT_THROW(makeEngine(Parameters {winLength, 100.0f, -60.0f}, outputDim - 1), std::invalid_argument);
}

TEST_F(AutoencoderTest, SetLatentReachesDecoderAsOneVector)
{
    const std::vector<float> z {0.1f, 0.2f, 0.3f, 0.4f, 0.5f, 0.6f, 0.7f, 0.8f};
    autoencoder->setLatent(z);

    EXPECT_CALL(*mockDecoder, decodeInto(ElementsAreArray(z), _)).Times(1);
    autoencoder->calculate(blockSize);
}

TEST_F(AutoencoderTest, LatestLatentWinsWithinOneBlock)
{
    const std::vector<float> first(latentDim, 1.0f);
    const std::vector<float> second(latentDim, 2.0f);
    autoencoder->setLatent(first);
    autoencoder->setLatent(second);

    EXPECT_CALL(*mockDecoder, decodeInto(ElementsAreArray(second), _)).Times(1);
    autoencoder->calculate(blockSize);
}

TEST_F(AutoencoderTest, KeepsDecodingTheLastLatentWhenNothingNewArrives)
{
    const std::vector<float> z(latentDim, 0.5f);
    autoencoder->setLatent(z);

    EXPECT_CALL(*mockDecoder, decodeInto(ElementsAreArray(z), _)).Times(2);
    autoencoder->calculate(blockSize);
    autoencoder->calculate(blockSize);
}

TEST_F(AutoencoderTest, SetLatentRejectsWrongSize)
{
    EXPECT_THROW(autoencoder->setLatent(std::vector<float>(latentDim + 1, 0.0f)), std::invalid_argument);
}

TEST_F(AutoencoderTest, SpectrumTapPublishesLastPrediction)
{
    decodeTo(0.25f);
    EXPECT_GT(playBlockAndReadRms(), 0.0f);

    decodeTo(0.75f);
    juce::AudioBuffer<float>     buffer(2, blockSize);
    juce::AudioSourceChannelInfo info {&buffer, 0, blockSize};
    autoencoder->getNextAudioBlock(info);

    SpectrumFrame frame;
    ASSERT_TRUE(autoencoder->readLatestSpectrum(frame));
    EXPECT_EQ(frame.bins.size(), outputDim);
    EXPECT_FLOAT_EQ(frame.bins.front(), 0.75f);
    EXPECT_FALSE(autoencoder->readLatestSpectrum(frame));
}

TEST_F(AutoencoderTest, XMaxTargetAppliedOnAudioThread)
{
    // pred 1.0 with xMax 0 and sClip -120 is -120 dB: silence.
    makeEngine(Parameters {winLength, 0.0f, -120.0f});
    decodeTo(1.0f);
    EXPECT_LT(playBlockAndReadRms(), 1e-3f);

    autoencoder->setXMax(200.0f);
    float rms = 0.0f;
    for (int block = 0; block < 10; ++block)
    {
        rms = playBlockAndReadRms();
    }
    EXPECT_GT(rms, 0.1f);
}

TEST_F(AutoencoderTest, ConsecutiveAudioBlocksDecodeEachTime)
{
    EXPECT_CALL(*mockDecoder, decodeInto(_, _)).Times(AtLeast(2));

    juce::AudioBuffer<float>     buffer(2, 256);
    juce::AudioSourceChannelInfo info {&buffer, 0, 256};
    autoencoder->getNextAudioBlock(info);
    autoencoder->getNextAudioBlock(info);
}

TEST_F(AutoencoderTest, StereoChannelsAreIdentical)
{
    juce::AudioBuffer<float> buffer(2, blockSize);
    buffer.clear();
    juce::AudioSourceChannelInfo channelInfo {&buffer, 0, blockSize};

    autoencoder->getNextAudioBlock(channelInfo);

    for (int sample = 0; sample < blockSize; ++sample)
    {
        EXPECT_FLOAT_EQ(buffer.getSample(0, sample), buffer.getSample(1, sample));
    }
}

TEST_F(AutoencoderTest, AudioIsClippedWithTanh)
{
    decodeTo(10.0f);
    juce::AudioBuffer<float> buffer(2, blockSize);
    buffer.clear();
    juce::AudioSourceChannelInfo channelInfo {&buffer, 0, blockSize};

    autoencoder->getNextAudioBlock(channelInfo);

    for (int channel = 0; channel < 2; ++channel)
    {
        for (int sample = 0; sample < blockSize; ++sample)
        {
            EXPECT_GE(buffer.getSample(channel, sample), -1.0f);
            EXPECT_LE(buffer.getSample(channel, sample), 1.0f);
        }
    }
}

TEST_F(AutoencoderTest, BlockStartingMidBufferLeavesEarlierSamplesUntouched)
{
    decodeTo(10.0f);
    juce::AudioBuffer<float> buffer(2, blockSize);
    buffer.clear();
    juce::AudioSourceChannelInfo channelInfo {&buffer, blockSize / 2, blockSize / 2};

    autoencoder->getNextAudioBlock(channelInfo);

    EXPECT_EQ(buffer.getMagnitude(0, 0, blockSize / 2), 0.0f);
    EXPECT_GT(buffer.getMagnitude(0, blockSize / 2, blockSize / 2), 0.0f);
}

TEST_F(AutoencoderTest, MutedOutputStillPublishesSpectrum)
{
    decodeTo(0.5f);
    autoencoder->setMuted(true);

    juce::AudioBuffer<float>     buffer(2, blockSize);
    juce::AudioSourceChannelInfo info {&buffer, 0, blockSize};
    autoencoder->getNextAudioBlock(info);

    SpectrumFrame frame;
    ASSERT_TRUE(autoencoder->readLatestSpectrum(frame));
    EXPECT_FLOAT_EQ(frame.bins.front(), 0.5f);
    EXPECT_EQ(frame.rms, 0.0f);
    EXPECT_EQ(buffer.getMagnitude(0, blockSize), 0.0f);
}
