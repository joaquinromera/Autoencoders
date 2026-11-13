/// @file TopBarView.cpp
/// @brief Implementation of TopBarView

#include <TopBarView.h>

#include <StringDefinitions.h>
#include <Theme.h>

namespace
{
constexpr int kGap = 14;

int textWidth(const juce::Font &font, const juce::String &text)
{
    return juce::GlyphArrangement::getStringWidthInt(font, text);
}

void showLit(juce::TextButton &button, bool lit, juce::Colour colour)
{
    button.setColour(juce::TextButton::buttonColourId, lit ? colour : Theme::raised);
    button.setColour(juce::TextButton::textColourOffId, lit ? Theme::background : Theme::text);
}
} // namespace

TopBarView::TopBarView()
{
    setUpButton(mModelButton, [](ITopBarPresenter &p) {
        p.onOpenModel();
    });
    setUpButton(mMidiMapButton, [](ITopBarPresenter &p) {
        p.onMidiMapToggled();
    });
    setUpButton(mMuteButton, [](ITopBarPresenter &p) {
        p.onMuteToggled();
    });
    setUpButton(mSettingsButton, [](ITopBarPresenter &p) {
        p.onSettings();
    });
    mMidiMapButton.setButtonText(Strings::Ui::MidiMap);
    mMuteButton.setButtonText(Strings::Ui::Mute);
    mSettingsButton.setButtonText(Theme::utf8(Strings::Ui::SettingsIcon));
    mDetails.setColour(juce::Label::textColourId, Theme::muted);
    mDetails.setFont(Theme::font(13.0f));
    addAndMakeVisible(mDetails);
}

void TopBarView::setUpButton(juce::TextButton &button, std::function<void(ITopBarPresenter &)> onClick)
{
    showLit(button, false, Theme::raised);
    button.onClick = [this, onClick] {
        if (mPresenter)
        {
            onClick(*mPresenter);
        }
    };
    addAndMakeVisible(button);
}

void TopBarView::showModel(const std::string &name, const std::string &details)
{
    mModelButton.setButtonText(Theme::utf8(name) + juce::String::fromUTF8("  ▾"));
    mDetails.setText(Theme::utf8(details), juce::dontSendNotification);
    resized();
}

void TopBarView::showNoModel()
{
    showModel(Strings::Ui::NoModel, Strings::Ui::OpenHint);
}

void TopBarView::showMidiMapMode(bool on)
{
    showLit(mMidiMapButton, on, Theme::you);
}

void TopBarView::showMidiStatus(const std::string &text)
{
    mMidiStatus = Theme::utf8(text);
    resized();
    repaint();
}

void TopBarView::showOutputStatus(const std::string &text)
{
    mOutputStatus = Theme::utf8(text);
    resized();
    repaint();
}

void TopBarView::showRateWarning(const std::string &text)
{
    mRateWarning = Theme::utf8(text);
    resized();
    repaint();
}

void TopBarView::showMuted(bool muted)
{
    showLit(mMuteButton, muted, Theme::warn);
}

void TopBarView::paint(juce::Graphics &g)
{
    g.fillAll(Theme::bar);
    g.setColour(Theme::line);
    g.fillRect(0, getHeight() - 1, getWidth(), 1);

    paintStatus(g, mMidiArea, Strings::Ui::MidiStatus, mMidiStatus);
    paintStatus(g, mOutputArea, Strings::Ui::OutStatus, mOutputStatus);
    if (mRateWarning.isNotEmpty())
    {
        g.setColour(Theme::warn.withAlpha(0.5f));
        g.drawRoundedRectangle(mWarningArea.toFloat().reduced(0.5f), 4.0f, 1.0f);
        g.setColour(Theme::warn);
        g.setFont(Theme::font(11.5f));
        g.drawText(mRateWarning, mWarningArea, juce::Justification::centred);
    }
}

void TopBarView::paintStatus(
    juce::Graphics &g, juce::Rectangle<int> area, const char *label, const juce::String &value
) const
{
    auto row = area.toFloat();
    g.setColour(Theme::ok);
    g.fillEllipse(row.removeFromLeft(6.0f).withSizeKeepingCentre(6.0f, 6.0f));
    row.removeFromLeft(6.0f);
    g.setFont(Theme::font(12.5f));
    g.setColour(Theme::muted);
    g.drawText(label, row, juce::Justification::centredLeft);
    g.setColour(Theme::text);
    g.drawText(
        value,
        row.withTrimmedLeft(static_cast<float>(textWidth(Theme::font(12.5f), label))),
        juce::Justification::centredLeft
    );
}

void TopBarView::resized()
{
    auto area = getLocalBounds().reduced(16, 8);
    mModelButton.setBounds(area.removeFromLeft(textWidth(Theme::font(14.0f), mModelButton.getButtonText()) + 32));
    area.removeFromLeft(12);

    mSettingsButton.setBounds(area.removeFromRight(32));
    area.removeFromRight(kGap);
    mMuteButton.setBounds(area.removeFromRight(60));
    area.removeFromRight(kGap);
    mMidiMapButton.setBounds(area.removeFromRight(110));
    area.removeFromRight(kGap);

    const auto statusWidth = [](const char *label, const juce::String &value) {
        return 12 + textWidth(Theme::font(12.5f), label) + textWidth(Theme::font(12.5f), value) + 4;
    };
    mWarningArea = mRateWarning.isEmpty()
                       ? juce::Rectangle<int>()
                       : area.removeFromRight(textWidth(Theme::font(11.5f), mRateWarning) + 16)
                             .withSizeKeepingCentre(textWidth(Theme::font(11.5f), mRateWarning) + 16, 20);
    area.removeFromRight(kGap);
    mOutputArea = area.removeFromRight(statusWidth(Strings::Ui::OutStatus, mOutputStatus));
    area.removeFromRight(kGap);
    mMidiArea = area.removeFromRight(statusWidth(Strings::Ui::MidiStatus, mMidiStatus));
    area.removeFromRight(kGap);
    mDetails.setBounds(area);
}
