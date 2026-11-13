/// @file LatentPanelView.cpp
/// @brief Implementation of LatentPanelView

#include <LatentPanelView.h>

#include <StringDefinitions.h>
#include <Theme.h>

namespace
{
constexpr int kHeaderHeight = 26;
constexpr int kSliderWidth  = 62;
} // namespace

void LatentPanelView::buildSliders(const std::vector<LatentSliderSpec> &sliders)
{
    mSliders.clear();
    for (std::size_t i = 0; i < sliders.size(); ++i)
    {
        auto slider = std::make_unique<LatentSliderView>(Theme::utf8(sliders[i].label), sliders[i].min, sliders[i].max);
        slider->onDragged = [this, i](float value) {
            if (mPresenter)
            {
                mPresenter->onSliderDragged(static_cast<int>(i), value);
            }
        };
        slider->onCcTagClicked = [this, i] {
            if (mPresenter)
            {
                mPresenter->onCcTagClicked(static_cast<int>(i));
            }
        };
        addAndMakeVisible(*slider);
        mSliders.push_back(std::move(slider));
    }
    resized();
}

LatentSliderView *LatentPanelView::sliderAt(int index) const
{
    return index >= 0 && static_cast<std::size_t>(index) < mSliders.size()
               ? mSliders[static_cast<std::size_t>(index)].get()
               : nullptr;
}

void LatentPanelView::showValue(int index, float value, bool flagged)
{
    if (auto *slider = sliderAt(index))
    {
        slider->setValue(value, flagged);
    }
}

void LatentPanelView::showHistogram(int index, const std::vector<float> &heights, float zeroShare, bool empty)
{
    if (auto *slider = sliderAt(index))
    {
        slider->setHistogram(heights, zeroShare, empty);
    }
}

void LatentPanelView::showSweep(int index, const std::vector<float> &brightness, const std::vector<float> &tonal)
{
    if (auto *slider = sliderAt(index))
    {
        slider->setSweep(brightness, tonal);
    }
}

void LatentPanelView::showWithoutStrips()
{
    for (auto &slider : mSliders)
    {
        slider->setStripsVisible(false);
    }
}

void LatentPanelView::showCcTags(const std::vector<CcTagInfo> &tags)
{
    for (std::size_t i = 0; i < mSliders.size(); ++i)
    {
        mSliders[i]->setCcTag(i < tags.size() ? std::optional<CcTagInfo>(tags[i]) : std::nullopt);
    }
}

void LatentPanelView::paint(juce::Graphics &g)
{
    namespace Ui = Strings::Ui;
    auto header  = getLocalBounds().removeFromTop(kHeaderHeight).toFloat();
    g.setColour(Theme::muted);
    g.setFont(Theme::title());
    g.drawText(Ui::Latent, header, juce::Justification::centredLeft);

    g.setFont(Theme::font(11.0f));

    // The sound key runs from noise to tone, as the sound strips do.
    struct Key
    {
        juce::Colour from, to;
        const char  *label;
    };

    const Key keys[] {
        { Theme::histogram, Theme::histogram,   Ui::KeyPiece},
        {Theme::sweepNoise,     Theme::tonal,   Ui::KeySound},
        {       Theme::you,       Theme::you,     Ui::KeyYou},
        {      Theme::warn,      Theme::warn, Ui::KeyOutside}
    };
    for (auto it = std::rbegin(keys); it != std::rend(keys); ++it)
    {
        const auto label = Theme::utf8(it->label);
        const auto width = static_cast<float>(juce::GlyphArrangement::getStringWidthInt(Theme::font(11.0f), label));
        g.setColour(Theme::muted);
        g.drawText(label, header.removeFromRight(width), juce::Justification::centredRight);
        const auto chip = header.removeFromRight(14.0f).withSizeKeepingCentre(7.0f, 10.0f);
        g.setGradientFill({it->from, chip.getX(), 0.0f, it->to, chip.getRight(), 0.0f, false});
        g.fillRoundedRectangle(chip, 1.5f);
        header.removeFromRight(8.0f);
    }
}

void LatentPanelView::resized()
{
    auto area = getLocalBounds().withTrimmedTop(kHeaderHeight);
    for (auto &slider : mSliders)
    {
        slider->setBounds(area.removeFromLeft(kSliderWidth));
    }
}
