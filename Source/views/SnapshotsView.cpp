/// @file SnapshotsView.cpp
/// @brief Implementation of SnapshotsView

#include <SnapshotsView.h>

#include <CloudImage.h>
#include <StringDefinitions.h>
#include <Theme.h>

namespace
{
constexpr int kTitleWidth  = 110;
constexpr int kSlotWidth   = 64;
constexpr int kSlotGap     = 6;
constexpr int kThumbnailPx = 72; ///< 36 logical pixels at 2×
constexpr int kGlideGap    = 64; ///< Room for the "Glide" label
constexpr int kGlideWidth  = 150;
constexpr int kMaxGlideMs  = 2'000;
} // namespace

SnapshotsView::SnapshotsView()
{
    for (std::size_t i = 0; i < mSlots.size(); ++i)
    {
        const int slot = static_cast<int>(i);
        mSlots[i].setSlot(slot);
        mSlots[i].onTapped = [this, slot] {
            if (mPresenter)
            {
                mPresenter->onSlotTapped(slot);
            }
        };
        mSlots[i].onHeld = [this, slot] {
            if (mPresenter)
            {
                mPresenter->onSlotHeld(slot);
            }
        };
        addAndMakeVisible(mSlots[i]);
    }
    mGlide.setSliderStyle(juce::Slider::LinearHorizontal);
    mGlide.setRange(0.0, kMaxGlideMs, 10.0);
    mGlide.setTextBoxStyle(juce::Slider::TextBoxRight, true, 64, 20);
    mGlide.setTextValueSuffix(Strings::Ui::Milliseconds);
    mGlide.setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
    mGlide.setColour(juce::Slider::trackColourId, Theme::muted);
    mGlide.setColour(juce::Slider::backgroundColourId, Theme::line2);
    mGlide.setColour(juce::Slider::thumbColourId, Theme::text);
    mGlide.onValueChange = [this] {
        if (mPresenter)
        {
            mPresenter->onGlideChanged(static_cast<int>(mGlide.getValue()));
        }
    };
    addAndMakeVisible(mGlide);
}

void SnapshotsView::setThumbnailOutline(const std::vector<MapPoint> &frames)
{
    mOutline = frames.empty() ? juce::Image() : CloudImage::renderOutline(frames, kThumbnailPx);
}

void SnapshotsView::showSlot(int slot, bool filled, std::optional<MapPoint> at)
{
    mSlots.at(static_cast<std::size_t>(slot)).setFilled(filled, at, mOutline);
}

void SnapshotsView::showGlideMs(int milliseconds)
{
    mGlide.setValue(milliseconds, juce::dontSendNotification);
}

void SnapshotsView::paint(juce::Graphics &g)
{
    auto area = getLocalBounds().toFloat();
    g.setColour(Theme::muted);
    g.setFont(Theme::title());
    g.drawText(Strings::Ui::Snapshots, area.removeFromLeft(kTitleWidth), juce::Justification::centredLeft);
    g.setColour(Theme::dim);
    g.setFont(Theme::font(11.0f));
    g.drawText(
        Theme::utf8(Strings::Ui::SnapshotsHint),
        area.withLeft(static_cast<float>(mGlide.getRight())),
        juce::Justification::centredRight
    );
    g.setColour(Theme::muted);
    g.setFont(Theme::font(12.5f));
    const auto glide = mGlide.getBounds().toFloat();
    g.drawText(
        Strings::Ui::Glide,
        glide.withX(glide.getX() - 44.0f).withWidth(40.0f),
        juce::Justification::centredRight
    );
}

void SnapshotsView::resized()
{
    auto area = getLocalBounds().withTrimmedLeft(kTitleWidth).reduced(0, 4);
    for (auto &slot : mSlots)
    {
        slot.setBounds(area.removeFromLeft(kSlotWidth));
        area.removeFromLeft(kSlotGap);
    }
    area.removeFromLeft(kGlideGap);
    mGlide.setBounds(area.removeFromLeft(kGlideWidth).withSizeKeepingCentre(kGlideWidth, 24));
}
