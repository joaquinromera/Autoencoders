/// @file CcTag.cpp
/// @brief Implementation of CcTag

#include <CcTag.h>

#include <StringDefinitions.h>

namespace
{
const juce::Colour kBound {0xff'9a'a5'ff};
const juce::Colour kLearning {0xff'f6'a6'3a};
const juce::Colour kNone {0xff'5a'60'69};
const juce::Colour kBackground {0xff'0b'0c'0e};

juce::String labelOf(const CcTagInfo &info)
{
    switch (info.state)
    {
        case CcTagInfo::State::bound:
            return Strings::Ui::CcPrefix + juce::String(info.controller);
        case CcTagInfo::State::learning:
            return juce::String::fromUTF8(Strings::Ui::CcLearning);
        case CcTagInfo::State::none:
            break;
    }
    return juce::String::fromUTF8(Strings::Ui::CcNone);
}
} // namespace

void CcTag::setInfo(CcTagInfo info)
{
    mInfo = info;
    repaint();
}

void CcTag::paint(juce::Graphics &g)
{
    const auto box    = getLocalBounds().toFloat().reduced(0.5f);
    const bool learn  = mInfo.state == CcTagInfo::State::learning;
    const auto colour = learn ? kLearning : (mInfo.state == CcTagInfo::State::bound ? kBound : kNone);
    g.setColour(learn ? colour : kBackground.withAlpha(0.85f));
    g.fillRoundedRectangle(box, 3.0f);
    g.setColour(colour);
    g.drawRoundedRectangle(box, 3.0f, 1.0f);
    g.setColour(learn ? kBackground : colour);
    g.setFont(juce::Font(juce::FontOptions(juce::Font::getDefaultMonospacedFontName(), 10.0f, juce::Font::plain)));
    g.drawText(labelOf(mInfo), box, juce::Justification::centred);
}

void CcTag::mouseUp(const juce::MouseEvent &event)
{
    if (onClicked && getLocalBounds().contains(event.getPosition()))
    {
        onClicked();
    }
}
