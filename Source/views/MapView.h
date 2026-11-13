/// @file MapView.h
/// @brief One map: header, sliced cloud, pins, ring and readouts

#pragma once

#include <CcTag.h>
#include <IMapPresenter.h>
#include <IMapView.h>
#include <MapBackground.h>
#include <MapHeader.h>

#include <juce_gui_basics/juce_gui_basics.h>

/// @brief A 2-D map of the piece the user drags on
class MapView
    : public juce::Component
    , public IMapView
    , private juce::Timer
{
public:
    /// @param letter "A" or "B"
    /// @param showLegend Whether this map carries the legend
    MapView(juce::String letter, bool showLegend);

    void setPresenter(IMapPresenter *presenter)
    {
        mPresenter = presenter;
    }

    void setCloudLayout(const std::vector<MapPoint> &frames) override;
    void showCloudAlphas(const std::vector<float> &dots, const std::vector<float> &segments) override;
    void showRing(MapPoint at, bool offMap) override;
    void showPins(const std::vector<MapPin> &pins) override;
    void showNearness(const MapNearness &nearness) override;
    void showPosition(const std::string &text) override;
    void showCaption(const MapCaption &caption) override;
    void showPlainPad(bool plain) override;
    void showProjectionMenu(const std::vector<ProjectionItem> &items) override;
    void showGlideTarget(bool active, MapPoint at) override;
    void showCcTags(const std::vector<CcTagInfo> &tags) override;
    void showLayer(MapLayer layer, bool soundAvailable) override;
    void
    showSoundGrid(std::size_t size, const std::vector<float> &brightness, const std::vector<float> &tonal) override;

    void paint(juce::Graphics &g) override;
    void resized() override;
    void mouseDown(const juce::MouseEvent &event) override;
    void mouseDrag(const juce::MouseEvent &event) override;

private:
    void                   timerCallback() override;
    juce::Rectangle<float> mapArea() const;
    juce::Point<float>     toPixels(MapPoint p) const;
    void                   paintMarks(juce::Graphics &g) const;
    void                   paintOverlays(juce::Graphics &g) const;
    void                   dragTo(juce::Point<float> position);

    bool                mShowLegend;
    IMapPresenter      *mPresenter = nullptr;
    MapBackground       mBackground;
    bool                mCloudChanged = false;
    MapPoint            mRing;
    bool                mRingOffMap = false;
    std::vector<MapPin> mPins;
    MapNearness         mNearness;
    juce::String        mPosition;
    MapCaption          mCaption;
    MapHeader           mHeader;
    CcTag               mXTag; ///< MIDI map mode: the x axis controller
    CcTag               mYTag; ///< MIDI map mode: the y axis controller
    bool                mGliding = false;
    MapPoint            mGlideTarget;
};
