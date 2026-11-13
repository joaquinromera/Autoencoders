/// @file MainComponent.cpp
/// @brief Implementation of the MainComponent class

#include <MainComponent.h>

#include <AutoencoderFactory.h>
#include <ControlRanges.h>
#include <StartingLatent.h>
#include <StringDefinitions.h>

constexpr int DEFAULT_BUFFER_SIZE = 512;
constexpr int kGuiRefreshHz       = 60; ///< Glides advance at this rate; the spectrum at half
constexpr int kWindowWidth        = 1'280;
constexpr int kWindowHeight       = 780;

//==============================================================================
MainComponent::MainComponent()
    : mMidiMapping()
    , mMidiLearn(
          mMidiMapping,
          [](std::function<void()> call) {
              juce::MessageManager::callAsync(std::move(call));
          }
      )
    , mMainGui(deviceManager, mLatentState)
    , mMidiInputHandler(deviceManager, mMainGui, mMidiMapping, mMidiLearn)
{
    addAndMakeVisible(mMainGui);
    mLatentState.addListener(this);

    mMainGui.onOpenButtonClicked = [this]() {
        openButtonClicked();
    };
    mMainGui.onMuteChanged = [this](bool muted) {
        mMuted = muted;
        if (mAutoencoder)
        {
            mAutoencoder->setMuted(muted);
        }
    };
    deviceManager.addChangeListener(this);

    mMainGui.setMidiAssignments(&mMidiLearn);
    mMidiLearn.onLearned = [this](MidiTarget, int) {
        mMainGui.refreshCcTags();
    };

    mMidiInputHandler.initialize();

    setSize(kWindowWidth, kWindowHeight);
    startTimerHz(kGuiRefreshHz);

    if (juce::RuntimePermissions::isRequired(juce::RuntimePermissions::recordAudio) &&
        !juce::RuntimePermissions::isGranted(juce::RuntimePermissions::recordAudio))
    {
        juce::RuntimePermissions::request(juce::RuntimePermissions::recordAudio, [&](bool /*granted*/) {
            setAudioChannels(0, 2);
        });
    }
    else
    {
        setAudioChannels(0, 2);
    }

    showAudioStatus();

    juce::StringArray commandLineArguments = juce::JUCEApplication::getCommandLineParameterArray();

    if (!commandLineArguments.isEmpty())
    {
        loadAutoencoder(commandLineArguments[0]);
    }
}

MainComponent::~MainComponent()
{
    stopTimer();
    deviceManager.removeChangeListener(this);
    mBackgroundDecoder.reset();
    shutdownAudio();
    mLatentState.removeListener(this);
}

//==============================================================================
void MainComponent::prepareToPlay([[maybe_unused]] int samplesPerBlockExpected, [[maybe_unused]] double sampleRate)
{
    DBG(Strings::Debug::MainComponent::SampleRatePrefix << sampleRate);
    DBG(Strings::Debug::MainComponent::BufferSizePrefix << samplesPerBlockExpected);

    auto ad       = deviceManager.getAudioDeviceSetup();
    ad.bufferSize = DEFAULT_BUFFER_SIZE;
    deviceManager.setAudioDeviceSetup(ad, true);
}

void MainComponent::getNextAudioBlock(const juce::AudioSourceChannelInfo &bufferToFill)
{
    if (mAutoencoder)
    {
        mAutoencoder->getNextAudioBlock(bufferToFill);
    }
    else
    {
        bufferToFill.clearActiveBufferRegion();
    }
}

void MainComponent::releaseResources()
{
    // This will be called when the audio device stops, or when it is being
    // restarted due to a setting change.

    // For more details, see the help for AudioProcessor::releaseResources()
}

//==============================================================================
void MainComponent::paint(juce::Graphics &g)
{
    g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
}

void MainComponent::resized()
{
    mMainGui.setBounds(getLocalBounds());
}

void MainComponent::openButtonClicked()
{
    juce::FileChooser chooser(
        juce::String(Strings::DialogTitles::SelectFileToLoad),
        {},
        juce::String(Strings::FilePatterns::AllFiles)
    );

    if (chooser.browseForFileToOpen())
    {
        const auto file = chooser.getResult();
        loadAutoencoder(file.getFullPathName());
    }
}

void MainComponent::loadAutoencoder(const juce::String &path)
{
    try
    {
        DBG(Strings::Debug::MainComponent::ChosenFilePrefix + path);

        // Parsing and loading the network take long: do them while the old
        // engine keeps playing, and hold the audio lock only for the swap.
        auto model = AutoencoderFactory::createFromFile(path.toStdString());
        model.engine->setLatent(startingLatent(ControlRanges::forModel(model.descriptor), model.track.get()));
        model.engine->setMuted(mMuted);
        swapEngine(model.engine);
        startBackgroundDecoder(model);
        startModel(model);
    }
    catch (std::exception &e)
    {
        DBG(Strings::Debug::MainComponent::ErrorLoadingModelPrefix << e.what());

        juce::AlertWindow::showMessageBoxAsync(
            juce::AlertWindow::WarningIcon,
            juce::String(Strings::DialogTitles::ErrorLoadingModel),
            juce::String(Strings::Errors::FailedToLoadModel) + e.what(),
            juce::String(Strings::Buttons::OK)
        );
    }
}

void MainComponent::swapEngine(std::unique_ptr<Autoencoder> &engine)
{
    const juce::ScopedLock lock(deviceManager.getAudioCallbackLock());
    std::swap(mAutoencoder, engine);
}

void MainComponent::startBackgroundDecoder(const LoadedModel &model)
{
    mBackgroundDecoder.reset();
    if (model.track == nullptr)
    {
        return; // without a track there is no piece to compare the sweeps with
    }
    const auto                  &d       = model.descriptor;
    auto                         safeGui = juce::Component::SafePointer<MainGui>(&mMainGui);
    BackgroundDecoder::Callbacks callbacks {
        [safeGui](const SweepResult &result) {
            if (auto *gui = safeGui.getComponent())
            {
                gui->showSweeps(result);
            }
        },
        [safeGui](std::size_t map, const SoundGridResult &result) {
            if (auto *gui = safeGui.getComponent())
            {
                gui->showSoundGrid(map, result);
            }
        },
    };
    mBackgroundDecoder =
        std::make_unique<BackgroundDecoder>(model.decoderSource, model.track, d.zRange, std::move(callbacks));
}

void MainComponent::startModel(const LoadedModel &model)
{
    auto                *worker = mBackgroundDecoder.get();
    BackgroundSubmitters submitters;
    if (worker != nullptr)
    {
        submitters.sweeps = [worker](const SweepJob &job) {
            worker->submit(job);
        };
        submitters.soundGrids = [worker](std::size_t map, const SoundGridJob &job) {
            worker->submitSoundGrid(map, job);
        };
    }
    mMainGui.setModel(model.descriptor, model.track, std::move(submitters));
    showAudioStatus();
    mMidiMapping.applyDefaults(model.descriptor.ccDefaults, model.descriptor.zRange);
    mMainGui.refreshCcTags();
}

void MainComponent::timerCallback()
{
    mSpectrumTick = !mSpectrumTick;
    if (mSpectrumTick && mAutoencoder && mAutoencoder->readLatestSpectrum(mSpectrum))
    {
        mMainGui.showSpectrum(mSpectrum.bins, mSpectrum.rms);
    }
    mMainGui.frameTick();

    constexpr int kMidiStatusTicks = 15; // 4 times a second at 60 Hz
    if (++mMidiStatusTick >= kMidiStatusTicks)
    {
        mMidiStatusTick = 0;
        mMainGui.setMidiStatus(mMidiInputHandler.deviceName(), mMidiInputHandler.lastController());
    }
}

void MainComponent::changeListenerCallback(juce::ChangeBroadcaster *)
{
    showAudioStatus();
}

void MainComponent::showAudioStatus()
{
    if (auto *device = deviceManager.getCurrentAudioDevice())
    {
        mMainGui.setAudioStatus(device->getCurrentSampleRate(), device->getCurrentBufferSizeSamples());
    }
}

void MainComponent::latentStateChanged(const LatentState &state)
{
    if (mAutoencoder && state.latent().size() == mAutoencoder->getLatentDim())
    {
        mAutoencoder->setLatent(state.latent());
        mAutoencoder->setXMax(state.xMax());
        mAutoencoder->setSClip(state.sClip());
    }
}
