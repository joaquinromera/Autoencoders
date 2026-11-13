/// @file TopBarPresenter.cpp
/// @brief Implementation of TopBarPresenter

#include <TopBarPresenter.h>

#include <StringDefinitions.h>

#include <cmath>
#include <cstdio>
#include <filesystem>

namespace
{
namespace Ui = Strings::Ui;

std::string minutesAndSeconds(double seconds)
{
    const auto total = static_cast<int>(std::lround(seconds));
    char       text[16];
    std::snprintf(text, sizeof text, "%d:%02d", total / 60, total % 60);
    return text;
}

std::string trainingSource(const ModelDescriptor &d)
{
    if (d.sourceFile.empty())
    {
        return {};
    }
    auto text = Ui::Separator + std::string(Ui::TrainedOn) + std::filesystem::path(d.sourceFile).filename().string();
    return d.durationSeconds > 0.0 ? text + " (" + minutesAndSeconds(d.durationSeconds) + ")" : text;
}
} // namespace

TopBarPresenter::TopBarPresenter(ITopBarView &view)
    : mView(view)
{
    mView.showNoModel();
    mView.showMuted(false);
    setMidiStatus({}, std::nullopt);
}

void TopBarPresenter::setModel(const ModelDescriptor &descriptor)
{
    mModelSampleRate = descriptor.sampleRate;
    mView.showModel(descriptor.name, detailsOf(descriptor));
    setAudioStatus(mDeviceSampleRate, -1);
}

void TopBarPresenter::setAudioStatus(double sampleRate, int bufferSize)
{
    constexpr double kSameRateHz = 0.5;
    mDeviceSampleRate            = sampleRate;
    if (bufferSize >= 0)
    {
        mView.showOutputStatus(kilohertz(sampleRate) + Ui::Separator + std::to_string(bufferSize));
    }
    const bool differs =
        mModelSampleRate > 0.0 && sampleRate > 0.0 && std::abs(sampleRate - mModelSampleRate) > kSameRateHz;
    mView.showRateWarning(differs ? Ui::TrainedAt + kilohertz(mModelSampleRate) : std::string {});
}

void TopBarPresenter::setMidiStatus(const std::string &device, std::optional<int> lastController)
{
    auto text = device.empty() ? std::string(Ui::NoMidiInput) : device;
    if (!device.empty() && lastController)
    {
        text += Ui::Separator + std::string(Ui::CcPrefix) + std::to_string(*lastController);
    }
    mView.showMidiStatus(text);
}

std::string TopBarPresenter::kilohertz(double sampleRate)
{
    char text[32];
    std::snprintf(text, sizeof text, "%.2f", sampleRate / 1'000.0);
    std::string number = text;
    number.erase(number.find_last_not_of('0') + 1); // 22.05, 44.1, 48.
    if (number.back() == '.')
    {
        number.pop_back();
    }
    return number + Ui::Kilohertz;
}

std::string TopBarPresenter::detailsOf(const ModelDescriptor &d)
{
    const auto *backend = d.backend == ModelBackend::TorchScript ? Ui::TorchScript : Ui::FrugallyDeep;
    return std::to_string(d.latentDim) + Ui::LatentDims + Ui::Separator + backend + trainingSource(d);
}

void TopBarPresenter::showMidiMapMode(bool on)
{
    mView.showMidiMapMode(on);
}

void TopBarPresenter::onMidiMapToggled()
{
    if (onMidiMapToggleRequested)
    {
        onMidiMapToggleRequested();
    }
}

void TopBarPresenter::onMuteToggled()
{
    mMuted = !mMuted;
    mView.showMuted(mMuted);
    if (onMuteChanged)
    {
        onMuteChanged(mMuted);
    }
}

void TopBarPresenter::onOpenModel()
{
    if (onOpenModelRequested)
    {
        onOpenModelRequested();
    }
}

void TopBarPresenter::onSettings()
{
    if (onSettingsRequested)
    {
        onSettingsRequested();
    }
}
