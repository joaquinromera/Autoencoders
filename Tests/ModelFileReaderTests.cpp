#include <ModelFileReader.h>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <nlohmann/json.hpp>

using ::testing::ElementsAre;

namespace
{
nlohmann::json syntheticModel()
{
    return nlohmann::json::parse(R"({
        "parameters": {
            "latent_dim": 2, "win_length": 2048, "hop_length": 256, "sr": 16000,
            "xMax": "97.5", "sClip": -55,
            "zRange": [{"min": "-1.0", "max": "2.0"}, {"min": 0, "max": 3}]
        },
        "model": {"architecture": "irrelevant to the reader"},
        "automation": {
            "default_slider_cc": [10, 11], "default_xmax_cc": 20, "default_sclip_cc": 21,
            "source_files": ["/audio/piece.wav"]
        },
        "ztrack": [[0.1, 0.2], [0.3, 0.4], [0.5, 0.6]]
    })");
}

ModelFile parse(const nlohmann::json &json, const std::string &path = "/models/duhast.json")
{
    return ModelFileReader::parse(json.dump(), path);
}
} // namespace

TEST(ModelFileReader, ReadsDescriptorFields)
{
    const auto d = parse(syntheticModel()).descriptor;

    EXPECT_EQ(d.name, "duhast");
    EXPECT_EQ(d.backend, ModelBackend::FrugallyDeep);
    EXPECT_EQ(d.latentDim, 2U);
    EXPECT_EQ(d.winLength, 2'048U);
    EXPECT_EQ(d.hopLength, 256U);
    EXPECT_DOUBLE_EQ(d.sampleRate, 16000.0);
    EXPECT_EQ(d.sourceFile, "/audio/piece.wav");
    EXPECT_DOUBLE_EQ(d.durationSeconds, 3.0 * 256.0 / 16000.0);
}

TEST(ModelFileReader, ParsesStringNumbers)
{
    const auto d = parse(syntheticModel()).descriptor;

    EXPECT_FLOAT_EQ(d.defaultXMax, 97.5f);
    EXPECT_FLOAT_EQ(d.defaultSClip, -55.0f);
    ASSERT_EQ(d.zRange.size(), 2U);
    EXPECT_FLOAT_EQ(d.zRange[0].min, -1.0f);
    EXPECT_FLOAT_EQ(d.zRange[0].max, 2.0f);
    EXPECT_FLOAT_EQ(d.zRange[1].max, 3.0f);
}

TEST(ModelFileReader, ReadsCcDefaults)
{
    const auto cc = parse(syntheticModel()).descriptor.ccDefaults;

    EXPECT_THAT(cc.sliderCcs, ElementsAre(10, 11));
    EXPECT_EQ(cc.xMaxCc, 20);
    EXPECT_EQ(cc.sClipCc, 21);
}

TEST(ModelFileReader, MissingAutomationFallsBackTo1ToN16And17)
{
    auto json = syntheticModel();
    json.erase("automation");

    const auto cc = parse(json).descriptor.ccDefaults;

    EXPECT_THAT(cc.sliderCcs, ElementsAre(1, 2));
    EXPECT_EQ(cc.xMaxCc, 16);
    EXPECT_EQ(cc.sClipCc, 17);
}

TEST(ModelFileReader, SliderCcListOfWrongLengthFallsBack)
{
    auto json                               = syntheticModel();
    json["automation"]["default_slider_cc"] = {10, 11, 12};

    EXPECT_THAT(parse(json).descriptor.ccDefaults.sliderCcs, ElementsAre(1, 2));
}

TEST(ModelFileReader, MissingZtrackGivesNoTrack)
{
    auto json = syntheticModel();
    json.erase("ztrack");

    const auto model = parse(json);

    EXPECT_EQ(model.track, nullptr);
    EXPECT_DOUBLE_EQ(model.descriptor.durationSeconds, 0.0);
}

TEST(ModelFileReader, ZtrackFramesKeepTheirOrder)
{
    const auto track = parse(syntheticModel()).track;

    ASSERT_NE(track, nullptr);
    EXPECT_EQ(track->frames(), 3U);
    EXPECT_THAT(track->frame(1), ElementsAre(0.3f, 0.4f));
}

TEST(ModelFileReader, ZtrackWithWrongFrameSizeGivesNoTrack)
{
    auto json      = syntheticModel();
    json["ztrack"] = {
        {0.1, 0.2},
        {0.3}
    };

    EXPECT_EQ(parse(json).track, nullptr);
}

TEST(ModelFileReader, TorchScriptModelIsLookedUpNextToTheJsonFile)
{
    auto json = syntheticModel();
    json.erase("model");
    json["model_name"] = "decoder.pt";

    const auto model = parse(json, "/models/debussy.json");

    EXPECT_EQ(model.descriptor.backend, ModelBackend::TorchScript);
    EXPECT_EQ(model.decoderSource.torchScriptPath, "/models/decoder.pt");
    EXPECT_EQ(model.decoderSource.latentDim, 2U);
}

TEST(ModelFileReader, MissingModelThrows)
{
    auto json = syntheticModel();
    json.erase("model");

    EXPECT_THROW(parse(json), std::runtime_error);
}
