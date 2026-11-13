/// @file OutputView.cpp
/// @brief Implementation of OutputView

#include <OutputView.h>

#include <StringDefinitions.h>
#include <Theme.h>

namespace
{
namespace Ui = Strings::Ui;

constexpr int kHeaderHeight = 26;
constexpr int kFieldWidth   = 70;

juce::String fieldText(float db, bool atDefault)
{
    auto text = juce::String(db, 1).replace("-", juce::String::fromUTF8("−"));
    return atDefault ? text + juce::String::fromUTF8(" ●") : text;
}

float parseField(const juce::String &text)
{
    return text.replace(juce::String::fromUTF8("−"), "-").getFloatValue();
}
} // namespace

OutputView::OutputView()
{
    setUpField(mFloorField, [](IOutputPresenter &p, float v) {
        p.onFloorEdited(v);
    });
    setUpField(mRangeField, [](IOutputPresenter &p, float v) {
        p.onRangeEdited(v);
    });
    mPlot.onBandPressed = [this](float db, float reachDb) {
        if (mPresenter)
        {
            mPresenter->onBandPressed(db, reachDb);
        }
    };
    mPlot.onBandDragged = [this](float db) {
        if (mPresenter)
        {
            mPresenter->onBandDragged(db);
        }
    };
    addAndMakeVisible(mPlot);
    for (auto *tag : {&mFloorTag, &mRangeTag})
    {
        const int index = tag == &mFloorTag ? 0 : 1;
        tag->onClicked  = [this, index] {
            if (mPresenter)
            {
                mPresenter->onCcTagClicked(index);
            }
        };
        addChildComponent(tag);
    }
}

void OutputView::showCcTags(const std::vector<CcTagInfo> &tags)
{
    const bool visible = tags.size() == 2;
    mFloorTag.setVisible(visible);
    mRangeTag.setVisible(visible);
    if (visible)
    {
        mFloorTag.setInfo(tags[0]);
        mRangeTag.setInfo(tags[1]);
    }
}

void OutputView::setUpField(juce::Label &field, std::function<void(IOutputPresenter &, float)> onEdited)
{
    field.setEditable(true);
    field.setFont(Theme::mono(12.0f));
    field.setJustificationType(juce::Justification::centred);
    field.setColour(juce::Label::textColourId, Theme::text);
    field.setColour(juce::Label::backgroundColourId, Theme::plot);
    field.setColour(juce::Label::outlineColourId, Theme::line);
    field.onTextChange = [this, &field, onEdited] {
        if (mPresenter)
        {
            onEdited(*mPresenter, parseField(field.getText()));
        }
    };
    addAndMakeVisible(field);
}

void OutputView::showFrequencies(const std::vector<double> &hz)
{
    mPlot.setFrequencies(hz);
}

void OutputView::showSpectrum(const std::vector<float> &db)
{
    mPlot.setSpectrum(db);
}

void OutputView::showBand(const OutputBand &band)
{
    mFloorField.setText(fieldText(band.floorDb, band.floorAtDefault), juce::dontSendNotification);
    mRangeField.setText(fieldText(band.rangeDb, band.rangeAtDefault), juce::dontSendNotification);
    mPlot.setBand(band);
}

void OutputView::showMeter(float levelDb, float peakDb)
{
    mPlot.setMeter(levelDb, peakDb);
}

void OutputView::paint(juce::Graphics &g)
{
    auto header = getLocalBounds().removeFromTop(kHeaderHeight);
    g.setColour(Theme::muted);
    g.setFont(Theme::title());
    g.drawText(Ui::Output, header.removeFromLeft(64), juce::Justification::centredLeft);
    g.setFont(Theme::font(12.5f));
    g.drawText(Theme::utf8(Ui::OutputHint), header.removeFromLeft(620), juce::Justification::centredLeft);

    header.removeFromRight(4);
    g.drawText(Theme::utf8(Ui::ModelDefaults), header.removeFromRight(110), juce::Justification::centredRight);
    const auto drawFieldLabels = [&](const juce::Label &field, const char *label) {
        const auto box = field.getBounds();
        g.setColour(Theme::muted);
        g.drawText(label, box.withX(box.getX() - 54).withWidth(48), juce::Justification::centredRight);
        g.setColour(Theme::dim);
        g.drawText(Ui::Decibels, box.withX(box.getRight() + 4).withWidth(24), juce::Justification::centredLeft);
    };
    drawFieldLabels(mFloorField, Ui::Floor);
    drawFieldLabels(mRangeField, Ui::Range);
}

void OutputView::resized()
{
    auto area   = getLocalBounds();
    auto header = area.removeFromTop(kHeaderHeight);
    header.removeFromRight(124);
    mRangeField.setBounds(
        header.removeFromRight(kFieldWidth).withSizeKeepingCentre(kFieldWidth, 20).translated(-30, 0)
    );
    header.removeFromRight(150); // room for the Range tag left of its label
    mFloorField.setBounds(
        header.removeFromRight(kFieldWidth).withSizeKeepingCentre(kFieldWidth, 20).translated(-30, 0)
    );
    mPlot.setBounds(area.withTrimmedTop(6));
    for (auto [tag, field] : {
             std::pair {&mFloorTag, &mFloorField},
             std::pair {&mRangeTag, &mRangeField}
    })
    {
        tag->setBounds(field->getX() - 58 - CcTag::kWidth, field->getY() + 2, CcTag::kWidth, CcTag::kHeight);
    }
}
