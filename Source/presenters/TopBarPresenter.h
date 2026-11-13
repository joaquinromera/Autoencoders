/// @file TopBarPresenter.h
/// @brief Describes the loaded model in the top bar

#pragma once

#include <ITopBarPresenter.h>
#include <ITopBarView.h>
#include <ModelDescriptor.h>

#include <functional>
#include <optional>
#include <string>

/// @brief Presenter of the top bar
class TopBarPresenter : public ITopBarPresenter
{
public:
    explicit TopBarPresenter(ITopBarView &view);

    void setModel(const ModelDescriptor &descriptor);

    void showMidiMapMode(bool on);

    /// @brief The audio device runs at sampleRate with blocks of bufferSize;
    /// warns when the model was trained at another rate
    void setAudioStatus(double sampleRate, int bufferSize);

    /// @param device Name of the MIDI input, empty when none
    /// @param lastController The controller number last received, if any
    void setMidiStatus(const std::string &device, std::optional<int> lastController);

    /// @brief "22.05 kHz", "48 kHz"
    static std::string kilohertz(double sampleRate);

    /// @brief e.g. "8 latent dims · frugally-deep · trained on piece.wav (2:00)"
    static std::string detailsOf(const ModelDescriptor &descriptor);

    void onOpenModel() override;
    void onSettings() override;
    void onMidiMapToggled() override;
    void onMuteToggled() override;

    std::function<void()>     onOpenModelRequested;
    std::function<void()>     onSettingsRequested;
    std::function<void()>     onMidiMapToggleRequested;
    std::function<void(bool)> onMuteChanged;

private:
    ITopBarView &mView;
    double       mModelSampleRate  = 0.0;
    double       mDeviceSampleRate = 0.0;
    bool         mMuted            = false;
};
