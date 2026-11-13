/// @file ModelFileReader.cpp
/// @brief Implementation of ModelFileReader

#include <ModelFileReader.h>

#include <JsonFloat.h>
#include <LatentTrackParser.h>
#include <MidiCcDefaultsParser.h>
#include <StringDefinitions.h>

#include <nlohmann/json.hpp>

#include <filesystem>
#include <fstream>
#include <sstream>
#include <stdexcept>

namespace
{
using nlohmann::json;
namespace Fields = Strings::JsonFields;

// Used when an older export leaves the field out: librosa's defaults, which
// the training scripts use.
constexpr double kDefaultSampleRate = 22'050.0;
constexpr float  kDefaultXMax       = 0.0f;
constexpr float  kDefaultSClip      = -100.0f;

std::vector<SliderMinMax> readZRange(const json &parameters)
{
    std::vector<SliderMinMax> zRange;
    for (const auto &range : parameters.at(Fields::ZRange))
    {
        zRange.push_back({readJsonFloat(range.at(Fields::Min)), readJsonFloat(range.at(Fields::Max))});
    }
    return zRange;
}

DecoderSource readDecoderSource(const json &root, const std::filesystem::path &path, std::size_t latentDim)
{
    if (root.contains(Fields::ModelName))
    {
        const auto modelPath = path.parent_path() / root[Fields::ModelName].get<std::string>();
        return {ModelBackend::TorchScript, {}, modelPath.string(), latentDim};
    }
    if (root.contains(Fields::Model))
    {
        return {ModelBackend::FrugallyDeep, root[Fields::Model].dump(), {}, latentDim};
    }
    throw std::runtime_error(Strings::Errors::JsonMissingModelFields);
}

std::string readSourceFile(const json &root)
{
    const auto files = root.value(Fields::Automation, json::object()).value(Fields::SourceFiles, json::array());
    return files.empty() ? std::string {} : files.front().get<std::string>();
}

ModelDescriptor readDescriptor(const json &root, const std::filesystem::path &path)
{
    const auto     &parameters = root.at(Fields::Parameters);
    ModelDescriptor d;
    d.name      = path.stem().string();
    d.zRange    = readZRange(parameters);
    d.latentDim = parameters.value(Fields::LatentDim, d.zRange.size());
    d.winLength = parameters.at(Fields::WinLength).get<unsigned int>();
    d.hopLength = parameters.value(Fields::HopLength, d.winLength / 4U);
    d.sampleRate =
        parameters.contains(Fields::SampleRate) ? readJsonFloat(parameters[Fields::SampleRate]) : kDefaultSampleRate;
    d.defaultXMax  = parameters.contains(Fields::XMax) ? readJsonFloat(parameters[Fields::XMax]) : kDefaultXMax;
    d.defaultSClip = parameters.contains(Fields::SClip) ? readJsonFloat(parameters[Fields::SClip]) : kDefaultSClip;
    d.sourceFile   = readSourceFile(root);
    d.ccDefaults   = parseMidiCcDefaults(root, d.latentDim);
    return d;
}
} // namespace

ModelFile ModelFileReader::read(const std::string &path)
{
    std::ifstream file(path);
    if (!file.is_open())
    {
        throw std::runtime_error(std::string(Strings::Errors::FailedToOpenJsonFile) + path);
    }

    std::stringstream text;
    text << file.rdbuf();
    return parse(text.str(), path);
}

ModelFile ModelFileReader::parse(const std::string &jsonText, const std::string &path)
{
    const auto root = json::parse(jsonText);
    ModelFile  model {readDescriptor(root, path), {}, nullptr};
    auto      &d        = model.descriptor;
    model.decoderSource = readDecoderSource(root, path, d.latentDim);
    model.track         = parseLatentTrack(root, d.latentDim);
    d.backend           = model.decoderSource.backend;

    if (model.track != nullptr)
    {
        d.durationSeconds = static_cast<double>(model.track->frames() * d.hopLength) / d.sampleRate;
    }
    return model;
}
