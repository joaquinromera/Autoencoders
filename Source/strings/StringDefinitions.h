/// @file StringDefinitions.h
/// @brief String constants used throughout the autoencoder application
/// @author Autoencoder Project
/// @date 2025

#pragma once

/// @brief Namespace containing all string constants used in the application
namespace Strings
{
// UI - Buttons
/// @brief Button text constants
namespace Buttons
{
inline constexpr const char *OK = "OK"; ///< Text for the OK button
}

// UI - Placeholder Text
/// @brief Placeholder text constants
namespace Placeholders
{
inline constexpr const char *NoMidiInputsEnabled = "No MIDI Inputs Enabled"; ///< Text when no MIDI inputs available
}

// UI - Dialog/Window Titles
/// @brief Dialog and window title constants
namespace DialogTitles
{
inline constexpr const char *Settings          = "Settings";                 ///< Settings dialog title
inline constexpr const char *SelectFileToLoad  = "Select a file to load..."; ///< File chooser dialog title
inline constexpr const char *ErrorLoadingModel = "Error Loading Model";      ///< Error dialog title
}

// JSON Field Names
/// @brief JSON field name constants for model configuration files
namespace JsonFields
{
inline constexpr const char *Parameters      = "parameters";        ///< JSON field for parameters object
inline constexpr const char *WinLength       = "win_length";        ///< JSON field for window length
inline constexpr const char *ZRange          = "zRange";            ///< JSON field for parameter ranges
inline constexpr const char *Min             = "min";               ///< JSON field for minimum value
inline constexpr const char *Max             = "max";               ///< JSON field for maximum value
inline constexpr const char *XMax            = "xMax";              ///< JSON field for default magnitude scaling
inline constexpr const char *SClip           = "sClip";             ///< JSON field for default spectral floor
inline constexpr const char *ModelName       = "model_name";        ///< JSON field for model file name
inline constexpr const char *Model           = "model";             ///< JSON field for embedded model
inline constexpr const char *LatentDim       = "latent_dim";        ///< JSON field for latent dimension
inline constexpr const char *ZTrack          = "ztrack";            ///< JSON field for the training audio's latent path
inline constexpr const char *SampleRate      = "sr";                ///< JSON field for the training sample rate
inline constexpr const char *HopLength       = "hop_length";        ///< JSON field for the STFT hop length
inline constexpr const char *Automation      = "automation";        ///< JSON field for the automation export metadata
inline constexpr const char *DefaultSliderCc = "default_slider_cc"; ///< JSON field for per-slider CCs
inline constexpr const char *DefaultXMaxCc   = "default_xmax_cc";   ///< JSON field for the xMax CC
inline constexpr const char *DefaultSClipCc  = "default_sclip_cc";  ///< JSON field for the sClip CC
inline constexpr const char *SourceFiles     = "source_files";      ///< JSON field for the training audio files
} // namespace JsonFields

// Error Messages
/// @brief Error message constants
namespace Errors
{
inline constexpr const char *FailedToOpenJsonFile =
    "Failed to open JSON file: "; ///< Error when JSON file cannot be opened
inline constexpr const char *ErrorLoadingTorchScriptModel =
    "Error loading TorchScript model from "; ///< Error when TorchScript model fails to load
inline constexpr const char *ErrorDuringModelPrediction =
    "Error during model prediction: "; ///< Error during model inference
inline constexpr const char *JsonMissingModelFields =
    "JSON missing both 'model' and 'model_name' fields";                   ///< Error when JSON lacks model info
inline constexpr const char *FailedToLoadModel = "Failed to load model: "; ///< General model loading error
} // namespace Errors

// Debug Log Messages - TorchWrapper
/// @brief Debug logging message constants
namespace Debug
{
/// @brief Debug messages for TorchWrapper class
namespace TorchWrapper
{
inline constexpr const char *ModelLoadedSuccessfully = "[TORCHWRAPPER] Model loaded successfully"; ///< Success message
inline constexpr const char *InputDepthPrefix = "[TORCHWRAPPER] Input depth (latent_dim): "; ///< Input depth log prefix
inline constexpr const char *OutputDepthPrefix =
    "[TORCHWRAPPER] Output depth (rfft size): "; ///< Output depth log prefix
}

/// @brief Debug messages for Autoencoder class
namespace Autoencoder
{
inline constexpr const char *InputDepthPrefix  = "[AUTOENCODER] INPUT DEPTH: ";  ///< Input depth log prefix
inline constexpr const char *OutputDepthPrefix = "[AUTOENCODER] OUTPUT DEPTH: "; ///< Output depth log prefix
inline constexpr const char *Destroying        = "[AUTOENCODER] Destroying...";  ///< Destructor log message
inline constexpr const char *Play              = "[AUTOENCODER] Play";           ///< Play state log message
inline constexpr const char *InvalidZTrackIndexPrefix =
    "[AUTOENCODER] Invalid ztrack index: "; ///< Invalid ztrack index error
} // namespace Autoencoder

/// @brief Debug messages for MainComponent class
namespace MainComponent
{
inline constexpr const char *SampleRatePrefix = "[MAINCOMPONENT] Sample rate ";   ///< Sample rate log prefix
inline constexpr const char *BufferSizePrefix = "[MAINCOMPONENT] Buffer Size ";   ///< Buffer size log prefix
inline constexpr const char *ChosenFilePrefix = "[MAIN_COMPONENT] Chosen file: "; ///< File selection log prefix
inline constexpr const char *ErrorLoadingModelPrefix =
    "[MAINCOMPONENT] Error loading model: "; ///< Model loading error prefix
} // namespace MainComponent

/// @brief Debug messages for MIDI functionality
namespace Midi
{
inline constexpr const char *Prefix           = "[MIDI] "; ///< General MIDI log prefix
inline constexpr const char *TimeSeparator    = "  -  ";   ///< Time separator in MIDI logs
inline constexpr const char *SourceOpenParen  = " (";      ///< Opening parenthesis for source info
inline constexpr const char *SourceCloseParen = ")";       ///< Closing parenthesis for source info
}
} // namespace Debug

// File Patterns and Formats
/// @brief File pattern constants
namespace FilePatterns
{
inline constexpr const char *AllFiles = "*"; ///< Pattern for all files
}

/// @brief Format string constants
namespace Formats
{
inline constexpr const char *TimeFormat = "%02d:%02d:%02d.%03d"; ///< Time format string
}

// Texts of the main window
/// @brief Strings the main window shows
namespace Ui
{
inline constexpr const char *FrugallyDeep  = "frugally-deep";
inline constexpr const char *TorchScript   = "TorchScript";
inline constexpr const char *LatentDims    = " latent dims";
inline constexpr const char *Separator     = " \u00b7 "; // " · "
inline constexpr const char *TrainedOn     = "trained on ";
inline constexpr const char *NoModel       = "no model";
inline constexpr const char *OpenHint      = "open a model file to start";
inline constexpr const char *Times         = " \u00d7 "; // " × "
inline constexpr const char *OfVariance    = "% of the piece's variance";
inline constexpr const char *RawDims       = "raw latent dims";
inline constexpr const char *OnThePiece    = "on the piece";
inline constexpr const char *NearThePiece  = "near the piece \u00b7 ";
inline constexpr const char *OffThePiece   = "off the piece";
inline constexpr const char *PrincipalAxis = "PC";
inline constexpr const char *LatentAxis    = "z";
inline constexpr const char *Output        = "OUTPUT";
inline constexpr const char *OutputHint =
    "what the decoder makes of the current z \u00b7 drag the band: level \u00b7 drag an edge: contrast";
inline constexpr const char *Floor           = "Floor";
inline constexpr const char *Range           = "Range";
inline constexpr const char *Decibels        = "dB";
inline constexpr const char *ModelDefaults   = "\u25cf model defaults";
inline constexpr const char *FloorLine       = "floor";
inline constexpr const char *CeilingLine     = "floor + range";
inline constexpr const char *Out             = "out";
inline constexpr const char *Latent          = "LATENT";
inline constexpr const char *KeyPiece        = "the piece";
inline constexpr const char *KeySound        = "noise \u2192 tone"; // "noise → tone"
inline constexpr const char *KeyYou          = "you";
inline constexpr const char *KeyOutside      = "outside the piece";
inline constexpr const char *LegendCloud     = "the piece, given the sliders";
inline constexpr const char *LegendYou       = "you";
inline constexpr const char *LegendSnapshot  = "snapshot";
inline constexpr const char *Snapshots       = "SNAPSHOTS";
inline constexpr const char *SnapshotsHint   = "1\u20130 go there \u00b7 hold a slot to store";
inline constexpr const char *Glide           = "Glide";
inline constexpr const char *Milliseconds    = " ms";
inline constexpr const char *View            = "View \u25be"; // "View ▾"
inline constexpr const char *ShowSection     = "Show";
inline constexpr const char *ShowPiece       = "The piece (training audio)";
inline constexpr const char *ShowSound       = "The sound (what the decoder makes)";
inline constexpr const char *LegendQuietLoud = "quiet \u2192 loud";
inline constexpr const char *LegendNoiseTone = "noise \u2192 tone";
inline constexpr const char *PrincipalPairs  = "Principal components";
inline constexpr const char *RawPairs        = "Raw latent dims";
inline constexpr const char *SettingsIcon    = "\u2699"; // ⚙
inline constexpr const char *MidiMap         = "MIDI map  M";
inline constexpr const char *Mute            = "Mute";
inline constexpr const char *MidiStatus      = "MIDI  ";
inline constexpr const char *OutStatus       = "Out  ";
inline constexpr const char *NoMidiInput     = "no input";
inline constexpr const char *Kilohertz       = " kHz";
inline constexpr const char *TrainedAt       = "model trained at ";
inline constexpr const char *CcPrefix        = "CC ";
inline constexpr const char *CcNone          = "CC \u2013";
inline constexpr const char *CcLearning      = "learn\u2026";
} // namespace Ui
} // namespace Strings
