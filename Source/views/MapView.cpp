/// @file MapView.cpp
/// @brief Implementation of MapView

#include <MapView.h>

#include <MapMarks.h>
#include <StringDefinitions.h>
#include <Theme.h>

namespace
{
constexpr int kCloudHz = 30;

juce::Colour nearnessColour(MapNearness::Kind kind)
{
    switch (kind)
    {
        case MapNearness::Kind::on:
            return Theme::muted;
        case MapNearness::Kind::near:
            return Theme::you;
        case MapNearness::Kind::off:
            break;
    }
    return Theme::warn;
}
} // namespace

MapView::MapView(juce::String letter, bool showLegend)
    : mShowLegend(showLegend)
    , mHeader(std::move(letter))
{
    mHeader.onProjectionSelected = [this](int id) {
        if (mPresenter)
        {
            mPresenter->onProjectionSelected(id);
        }
    };
    mHeader.onLayerSelected = [this](MapLayer layer) {
        if (mPresenter)
        {
            mPresenter->onLayerSelected(layer);
        }
    };
    addAndMakeVisible(mHeader);
    for (auto *tag : {&mXTag, &mYTag})
    {
        const int index = tag == &mXTag ? 0 : 1;
        tag->onClicked  = [this, index] {
            if (mPresenter)
            {
                mPresenter->onCcTagClicked(index);
            }
        };
        addChildComponent(tag);
    }
    startTimerHz(kCloudHz);
}

void MapView::showCcTags(const std::vector<CcTagInfo> &tags)
{
    const bool visible = tags.size() == 2;
    mXTag.setVisible(visible);
    mYTag.setVisible(visible);
    if (visible)
    {
        mXTag.setInfo(tags[0]);
        mYTag.setInfo(tags[1]);
    }
}

void MapView::resized()
{
    mHeader.setBounds(getLocalBounds().removeFromTop(MapHeader::kHeight));
    const auto area = mapArea().toNearestInt();
    mXTag.setBounds(area.getRight() - CcTag::kWidth - 10, area.getBottom() - 46, CcTag::kWidth, CcTag::kHeight);
    mYTag.setBounds(area.getX() + 24, area.getCentreY() - (CcTag::kHeight / 2), CcTag::kWidth, CcTag::kHeight);
}

void MapView::showProjectionMenu(const std::vector<ProjectionItem> &items)
{
    mHeader.setProjections(items);
}

void MapView::showGlideTarget(bool active, MapPoint at)
{
    mGliding     = active;
    mGlideTarget = at;
    repaint();
}

void MapView::setCloudLayout(const std::vector<MapPoint> &frames)
{
    mBackground.setCloudLayout(frames);
    mCloudChanged = true;
}

void MapView::showCloudAlphas(const std::vector<float> &dots, const std::vector<float> &segments)
{
    mBackground.setCloudAlphas(dots, segments);
    mCloudChanged = true;
}

void MapView::showLayer(MapLayer layer, bool soundAvailable)
{
    mBackground.setLayer(layer);
    mHeader.setLayer(layer, soundAvailable);
    repaint();
}

void MapView::showSoundGrid(std::size_t size, const std::vector<float> &brightness, const std::vector<float> &tonal)
{
    mBackground.setSound(size, brightness, tonal);
    repaint();
}

void MapView::showRing(MapPoint at, bool offMap)
{
    mRing       = at;
    mRingOffMap = offMap;
    repaint();
}

void MapView::showPins(const std::vector<MapPin> &pins)
{
    mPins = pins;
    repaint();
}

void MapView::showNearness(const MapNearness &nearness)
{
    mNearness = nearness;
    repaint();
}

void MapView::showPosition(const std::string &text)
{
    mPosition = Theme::utf8(text);
    repaint();
}

void MapView::showCaption(const MapCaption &caption)
{
    mCaption = caption;
    mHeader.setCaption(caption.title, caption.subtitle);
    repaint();
}

void MapView::showPlainPad(bool plain)
{
    mBackground.setPlain(plain);
    repaint();
}

void MapView::timerCallback()
{
    // The cloud can change at mouse rate; redraw it at most kCloudHz times a second.
    if (std::exchange(mCloudChanged, false))
    {
        repaint();
    }
}

juce::Rectangle<float> MapView::mapArea() const
{
    return getLocalBounds().withTrimmedTop(MapHeader::kHeight).toFloat().reduced(0.5f);
}

juce::Point<float> MapView::toPixels(MapPoint p) const
{
    const auto area = mapArea();
    return {area.getX() + (p.x * area.getWidth()), area.getBottom() - (p.y * area.getHeight())};
}

void MapView::paint(juce::Graphics &g)
{
    const auto scale = g.getInternalContext().getPhysicalPixelScaleFactor();
    mBackground.paint(g, mapArea(), toPixels(mCaption.origin), scale);
    paintMarks(g);
    paintOverlays(g);
    g.setColour(Theme::line2);
    g.drawRoundedRectangle(mapArea(), MapBackground::kCorner, 1.0f);
}

void MapView::paintMarks(juce::Graphics &g) const
{
    const auto area = mapArea();
    for (const auto &pin : mPins)
    {
        MapMarks::pin(g, toPixels(pin.at), pin.slot, area);
    }
    if (mGliding)
    {
        MapMarks::glideLine(g, toPixels(mRing), toPixels(mGlideTarget));
    }
    MapMarks::ring(g, toPixels(mRing), mRingOffMap, area);
}

void MapView::paintOverlays(juce::Graphics &g) const
{
    const auto area = mapArea().reduced(10.0f, 8.0f);
    g.setFont(Theme::font(11.5f));
    if (!mBackground.isPlain())
    {
        MapMarks::nearness(
            g,
            area.withHeight(16.0f),
            Theme::utf8(mNearness.text),
            mNearness.gauge,
            nearnessColour(mNearness.kind)
        );
        if (mShowLegend)
        {
            MapMarks::legend(g, area.withTrimmedLeft(area.getWidth() * 0.5f), mBackground.layer());
        }
    }
    g.setColour(Theme::text);
    g.setFont(Theme::mono(11.5f));
    g.drawText(mPosition, area.withTrimmedTop(area.getHeight() - 16.0f), juce::Justification::centredLeft);
    g.setColour(Theme::dim);
    g.setFont(Theme::font(11.0f));
    g.drawText(
        Theme::utf8(mCaption.xLabel),
        area.withTrimmedTop(area.getHeight() - 16.0f),
        juce::Justification::centredRight
    );
    MapMarks::verticalLabel(g, Theme::utf8(mCaption.yLabel), mapArea());
}

void MapView::mouseDown(const juce::MouseEvent &event)
{
    dragTo(event.position);
}

void MapView::mouseDrag(const juce::MouseEvent &event)
{
    dragTo(event.position);
}

void MapView::dragTo(juce::Point<float> position)
{
    const auto area = mapArea();
    if (mPresenter == nullptr || (!area.contains(position) && !isMouseButtonDown()))
    {
        return;
    }
    const float x = juce::jlimit(0.0f, 1.0f, (position.x - area.getX()) / area.getWidth());
    const float y = juce::jlimit(0.0f, 1.0f, (area.getBottom() - position.y) / area.getHeight());
    mPresenter->onMapDragged(x, y);
}
