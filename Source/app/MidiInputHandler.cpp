/// @file MidiInputHandler.cpp
/// @brief Implementation of the MidiInputHandler class

#include <MidiInputHandler.h>

namespace
{
/// Message thread: moves the control target drives to the controller's position
void applyToGui(MainGui &gui, const MidiParameterMapping::Target &target, double normalised)
{
    const auto value = target.minimum + (normalised * (target.maximum - target.minimum));
    switch (target.target.kind)
    {
        case MidiTarget::Kind::latent:
            gui.setLatentValueFromMidi(target.target.index, value);
            break;
        case MidiTarget::Kind::xMax:
            gui.setXMaxFromMidi(value);
            break;
        case MidiTarget::Kind::sClip:
            gui.setSClipFromMidi(value);
            break;
        case MidiTarget::Kind::mapAxis:
            gui.setMapAxisFromMidi(target.target.index / 2, target.target.index % 2, normalised);
            break;
    }
}
} // namespace

MidiInputHandler::MidiInputHandler(
    juce::AudioDeviceManager &deviceManager, MainGui &gui, MidiParameterMapping &mapping, MidiLearn &learn
)
    : mDeviceManager(deviceManager)
    , mGui(gui)
    , mMapping(mapping)
    , mLearn(learn)
{
    mDeviceManager.addMidiInputDeviceCallback({}, this);
    mDeviceManager.addChangeListener(this);
}

MidiInputHandler::~MidiInputHandler()
{
    mDeviceManager.removeChangeListener(this);
    mDeviceManager.removeMidiInputDeviceCallback({}, this);
}

void MidiInputHandler::initialize()
{
    const auto midiInputs = juce::MidiInput::getAvailableDevices();

    for (const auto &input : midiInputs)
    {
        if (mDeviceManager.isMidiInputDeviceEnabled(input.identifier))
        {
            enableOnlyDevice(input.identifier);
            return;
        }
    }

    if (!midiInputs.isEmpty())
    {
        setMidiInput(0);
    }
}

void MidiInputHandler::setMidiInput(int index)
{
    const auto list = juce::MidiInput::getAvailableDevices();

    if (list.isEmpty() || index < 0 || index >= list.size())
    {
        return;
    }

    enableOnlyDevice(list[index].identifier);
}

void MidiInputHandler::enableOnlyDevice(const juce::String &identifier)
{
    const auto midiInputs = juce::MidiInput::getAvailableDevices();

    mIsAdjustingMidiDevices = true;

    for (const auto &input : midiInputs)
    {
        const bool shouldEnable = input.identifier == identifier;
        mDeviceManager.setMidiInputDeviceEnabled(input.identifier, shouldEnable);

        if (shouldEnable)
        {
            mSelectedDeviceIdentifier = identifier;
            mSelectedDeviceName       = input.name;
        }
    }

    mIsAdjustingMidiDevices = false;
}

void MidiInputHandler::changeListenerCallback([[maybe_unused]] juce::ChangeBroadcaster *source)
{
    if (mIsAdjustingMidiDevices)
    {
        return;
    }

    const auto midiInputs = juce::MidiInput::getAvailableDevices();

    juce::Array<juce::String> enabledIdentifiers;
    enabledIdentifiers.ensureStorageAllocated(midiInputs.size());

    for (const auto &input : midiInputs)
    {
        if (mDeviceManager.isMidiInputDeviceEnabled(input.identifier))
        {
            enabledIdentifiers.add(input.identifier);
        }
    }

    if (enabledIdentifiers.isEmpty())
    {
        mSelectedDeviceIdentifier.clear();
        mSelectedDeviceName.clear();
        return;
    }

    if (enabledIdentifiers.contains(mSelectedDeviceIdentifier))
    {
        enableOnlyDevice(mSelectedDeviceIdentifier);
        return;
    }

    enableOnlyDevice(enabledIdentifiers.getFirst());
}

void
MidiInputHandler::handleIncomingMidiMessage([[maybe_unused]] juce::MidiInput *source, const juce::MidiMessage &message)
{
    const juce::ScopedValueSetter<bool> scopedInputFlag(mIsAddingFromMidiInput, true);

    if (message.isController())
    {
        mLastController.store(message.getControllerNumber(), std::memory_order_relaxed);
    }
    if (message.isController() && !mLearn.offer(message.getControllerNumber()))
    {
        const auto targets = mMapping.getTargetsForController(message.getControllerNumber());
        if (targets.empty())
        {
            return;
        }
        const auto normalised = juce::jlimit(0.0, 1.0, static_cast<double>(message.getControllerValue()) / 127.0);
        juce::MessageManager::callAsync([safeGui = juce::Component::SafePointer<MainGui>(&mGui), targets, normalised] {
            if (auto *gui = safeGui.getComponent())
            {
                for (const auto &target : targets)
                {
                    applyToGui(*gui, target, normalised);
                }
            }
        });
    }

#if JUCE_DEBUG
    if (source != nullptr)
    {
        juce::Logger::writeToLog(source->getName() + ": " + message.getDescription());
    }
    else
    {
        juce::Logger::writeToLog(message.getDescription());
    }
#endif
}
