/// @file Main.cpp
/// @brief Application entry point and main window management
/// @author Autoencoder Project
/// @date 2025

#include <AppLookAndFeel.h>
#include <MainComponent.h>

#include <JuceHeader.h>

//==============================================================================
/// @brief Main application class for the Autoencoder synthesizer
///
/// This class handles application lifecycle, window management, and basic
/// application properties following the JUCE application framework.
class SynthApplication : public juce::JUCEApplication
{
public:
    //==============================================================================
    /// @brief Default constructor
    SynthApplication() = default;

    /// @brief Gets the application name
    /// @return The application name from ProjectInfo
    const juce::String getApplicationName() override
    {
        return ProjectInfo::projectName;
    }

    /// @brief Gets the application version
    /// @return The application version from ProjectInfo
    const juce::String getApplicationVersion() override
    {
        return ProjectInfo::versionString;
    }

    /// @brief Determines if multiple instances are allowed
    /// @return True to allow multiple instances
    bool moreThanOneInstanceAllowed() override
    {
        return true;
    }

    //==============================================================================
    /// @brief Initializes the application and creates the main window
    /// @param commandLine Command line arguments (unused)
    void initialise(const juce::String & /*commandLine*/) override
    {
        juce::LookAndFeel::setDefaultLookAndFeel(&mLookAndFeel);
        mMainWindow.reset(new MainWindow(getApplicationName()));
    }

    /// @brief Shuts down the application and cleans up resources
    void shutdown() override
    {
        mMainWindow = nullptr; // (deletes our window)
        juce::LookAndFeel::setDefaultLookAndFeel(nullptr);
    }

    //==============================================================================
    /// @brief Handles system quit requests
    void systemRequestedQuit() override
    {
        // This is called when the app is being asked to quit: you can ignore this
        // request and let the app carry on running, or call quit() to allow the app to close.
        quit();
    }

    /// @brief Handles additional instance startup
    /// @param commandLine Command line arguments for the new instance
    void anotherInstanceStarted(const juce::String & /*commandLine*/) override
    {
        // When another instance of the app is launched while this one is running,
        // this method is invoked, and the commandLine parameter tells you what
        // the other instance's command-line arguments were.
    }

    //==============================================================================
    /*
        This class implements the desktop window that contains an instance of
        our MainComponent class.
    */
    class MainWindow : public juce::DocumentWindow
    {
    public:
        explicit MainWindow(const juce::String &name)
            : DocumentWindow(
                  name,
                  juce::Desktop::getInstance().getDefaultLookAndFeel().findColour(
                      juce::ResizableWindow::backgroundColourId
                  ),
                  DocumentWindow::allButtons
              )
        {
            setUsingNativeTitleBar(true);
            setContentOwned(new MainComponent(), true); // NOLINT(cppcoreguidelines-owning-memory)

#if JUCE_IOS || JUCE_ANDROID
            setFullScreen(true);
#else
            setResizable(false, false);
            centreWithSize(getWidth(), getHeight());
#endif

            setVisible(true);
        }

        void closeButtonPressed() override
        {
            // This is called when the user tries to close this window. Here, we'll just
            // ask the app to quit when this happens, but you can change this to do
            // whatever you need.
            JUCEApplication::getInstance()->systemRequestedQuit();
        }

        /* Note: Be careful if you override any DocumentWindow methods - the base
           class uses a lot of them, so by overriding you might break its functionality.
           It's best to do all your work in your content component instead, but if
           you really have to override any DocumentWindow methods, make sure your
           subclass also calls the superclass's method.
        */

    private:
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainWindow)
    };

private:
    AppLookAndFeel              mLookAndFeel; ///< Default look of every window; outlives them
    std::unique_ptr<MainWindow> mMainWindow;
};

//==============================================================================
// This macro generates the main() routine that launches the app.
START_JUCE_APPLICATION(SynthApplication)
