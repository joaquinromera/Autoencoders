/// @file MainGui.cpp
/// @brief Implementation of the MainGui class

#include <MainGui.h>

#include <ControlRanges.h>
#include <LatentState.h>
#include <MainPresenter.h>
#include <StringDefinitions.h>
#include <Theme.h>

namespace
{
using Track = juce::Grid::TrackInfo;
using juce::operator""_px;
using juce::operator""_fr;

double nowMs()
{
    return juce::Time::getMillisecondCounterHiRes();
}
} // namespace

MainGui::MainGui(juce::AudioDeviceManager &audioDeviceManager, LatentState &state)
    : mDeviceManager(audioDeviceManager)
    , mState(state)
    , mMapA("A", true)
    , mMapB("B", false)
    , mPresenter(
          std::make_unique<MainPresenter>(
              state, mSnapshots, MainViews {mTopBar, mOutput, mMapA, mMapB, mLatentPanel, mSnapshotsView}, nowMs
          )
      )
{
    mTopBar.setPresenter(&mPresenter->topBar());
    mOutput.setPresenter(&mPresenter->output());
    mMapA.setPresenter(&mPresenter->mapA());
    mMapB.setPresenter(&mPresenter->mapB());
    mLatentPanel.setPresenter(&mPresenter->latentPanel());
    mSnapshotsView.setPresenter(&mPresenter->snapshots());

    mPresenter->topBar().onOpenModelRequested = [this] {
        if (onOpenButtonClicked)
        {
            onOpenButtonClicked();
        }
    };
    mPresenter->onMuteChanged = [this](bool muted) {
        if (onMuteChanged)
        {
            onMuteChanged(muted);
        }
    };
    mPresenter->topBar().onSettingsRequested = [this] {
        showSettingsWindow();
    };

    setWantsKeyboardFocus(true);
    for (auto *view :
         std::initializer_list<juce::Component *> {&mTopBar, &mOutput, &mMapA, &mMapB, &mLatentPanel, &mSnapshotsView})
    {
        addAndMakeVisible(view);
    }
}

MainGui::~MainGui()
{
    if (mSettingsWindow != nullptr)
    {
        delete mSettingsWindow.getComponent();
    }
}

void MainGui::paint(juce::Graphics &g)
{
    g.fillAll(Theme::background);
}

void MainGui::resized()
{
    // 1280 × 780: top bar, then output, maps and latent, snapshots, with 16 px
    // margins and 14 px between rows.
    juce::Grid grid;
    grid.templateRows = {
        Track(44_px),
        Track(14_px),
        Track(240_px),
        Track(14_px),
        Track(386_px),
        Track(14_px),
        Track(52_px),
        Track(1_fr)
    };
    grid.templateColumns =
        {Track(16_px), Track(360_px), Track(16_px), Track(360_px), Track(16_px), Track(1_fr), Track(16_px)};
    grid.items = {
        juce::GridItem(mTopBar).withArea(1, 1, 2, 8),
        juce::GridItem(mOutput).withArea(3, 2, 4, 7),
        juce::GridItem(mMapA).withArea(5, 2, 6, 3),
        juce::GridItem(mMapB).withArea(5, 4, 6, 5),
        juce::GridItem(mLatentPanel).withArea(5, 6, 6, 7),
        juce::GridItem(mSnapshotsView).withArea(7, 2, 8, 7),
    };
    grid.performLayout(getLocalBounds());
}

bool MainGui::keyPressed(const juce::KeyPress &key)
{
    const int code = key.getKeyCode();
    if (dynamic_cast<juce::TextEditor *>(getCurrentlyFocusedComponent()) != nullptr)
    {
        return false;
    }
    if (code == 'M' || code == 'm')
    {
        mPresenter->toggleMidiMapMode();
        return true;
    }
    if (code < '0' || code > '9')
    {
        return false;
    }
    const int slot = code == '0' ? 9 : code - '1'; // keys 1..9, 0 are slots 0..9
    mPresenter->onDigit(slot, key.getModifiers().isShiftDown());
    return true;
}

void MainGui::setModel(
    const ModelDescriptor &descriptor, std::shared_ptr<const LatentTrack> track, BackgroundSubmitters submitters
)
{
    mPresenter->setModel(descriptor, std::move(track), std::move(submitters));
}

void MainGui::showSpectrum(std::span<const float> bins, float rms)
{
    mPresenter->onSpectrum(bins, rms);
}

void MainGui::frameTick()
{
    mPresenter->onFrameTick();
}

void MainGui::showSweeps(const SweepResult &result)
{
    mPresenter->onSweeps(result);
}

void MainGui::showSoundGrid(std::size_t map, const SoundGridResult &result)
{
    mPresenter->onSoundGrid(map, result);
}

void MainGui::setMidiAssignments(IMidiAssignments *assignments)
{
    mPresenter->setMidiAssignments(assignments);
}

void MainGui::refreshCcTags()
{
    mPresenter->refreshCcTags();
}

void MainGui::setAudioStatus(double sampleRate, int bufferSize)
{
    mPresenter->setAudioStatus(sampleRate, bufferSize);
}

void MainGui::setMidiStatus(const juce::String &device, int lastController)
{
    mPresenter->setMidiStatus(
        device.toStdString(),
        lastController >= 0 ? std::optional<int>(lastController) : std::nullopt
    );
}

void MainGui::setMapAxisFromMidi(size_t map, size_t axis, double value)
{
    mPresenter->setMapAxisFromMidi(map, axis, static_cast<float>(value));
}

void MainGui::setLatentValueFromMidi(size_t index, double value)
{
    mPresenter->setLatentFromMidi(index, static_cast<float>(value));
}

void MainGui::setXMaxFromMidi(double value)
{
    mPresenter->setXMaxFromMidi(static_cast<float>(value));
}

void MainGui::setSClipFromMidi(double value)
{
    mPresenter->setSClipFromMidi(static_cast<float>(value));
}

void MainGui::showSettingsWindow()
{
    if (mSettingsWindow != nullptr)
    {
        mSettingsWindow->toFront(true);
        return;
    }

    auto       content = std::make_unique<SettingsView>(mDeviceManager);
    const auto width   = content->getPreferredWidth();
    const auto height  = content->getPreferredHeight();

    juce::DialogWindow::LaunchOptions options;
    options.dialogTitle                  = juce::String(Strings::DialogTitles::Settings);
    options.dialogBackgroundColour       = Theme::background;
    options.useNativeTitleBar            = true;
    options.resizable                    = true;
    options.escapeKeyTriggersCloseButton = true;
    options.content.setOwned(content.release());

    if (auto *window = options.launchAsync())
    {
        mSettingsWindow = window;
        window->centreAroundComponent(this, width, height);
    }
}
