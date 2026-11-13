# TODO

Major Issues from an Audio Plugin Perspective:

1. It's not a plugin - it's a standalone application
CMakeLists.txt:35 uses juce_add_gui_app instead of juce_add_plugin
No VST3, AU, AAX, or any DAW-compatible plugin format
Can't be used in a DAW (Ableton, Logic, Pro Tools, etc.)
2. No proper AudioProcessor architecture
MainComponent.h extends AudioAppComponent instead of AudioProcessor
Missing plugin-specific methods: processBlock(), getStateInformation(), setStateInformation()
No parameter management system (APVTS - AudioProcessorValueTreeState)
3. No state save/recall
Cannot save/load plugin state in DAW sessions
Parameters/settings are lost when closing
No preset management
4. MIDI CC assignments (addressed for the standalone app)
Each model file's "automation" defaults are applied on load; MIDI map mode (M)
rebinds any control by moving a controller. Assignments are not saved between
runs and are not DAW-automatable.
5. File loading via GUI dialog
MainComponent.cpp:151-172 uses modal file chooser
Should be part of plugin state/presets, not runtime file selection
Not suitable for plugin automation/recall
6. Fixed buffer size enforcement
MainComponent.cpp:116 forces 512 sample buffer
DAW hosts expect plugins to work with any buffer size
Will cause issues in most professional workflows
7. No plugin metadata
Missing manufacturer, version, category, input/output configuration
No plugin description or proper naming
8. Thread safety (addressed)
The GUI publishes the whole latent vector through a lock-free triple buffer
(LatentInputBuffer) and xMax/sClip through atomics read at block start;
the audio thread publishes its spectrum back the same way (SpectrumTap).
Model loading parses outside the audio lock and only swaps the engine inside it.
9. No proper latency reporting
FFT processing likely introduces latency but it's not reported to host
10. Modal UI interactions
CMakeLists.txt:63 enables JUCE_MODAL_LOOPS_PERMITTED
File chooser blocks - unacceptable in plugin context
