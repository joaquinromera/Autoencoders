#pragma once

#include <ILatentPanelView.h>
#include <IMapView.h>
#include <IOutputView.h>
#include <ISnapshotsView.h>
#include <ITopBarView.h>

#include <gmock/gmock.h>

class MockTopBarView : public ITopBarView
{
public:
    MOCK_METHOD(void, showModel, (const std::string &name, const std::string &details), (override));
    MOCK_METHOD(void, showNoModel, (), (override));
    MOCK_METHOD(void, showMidiMapMode, (bool on), (override));
    MOCK_METHOD(void, showMidiStatus, (const std::string &text), (override));
    MOCK_METHOD(void, showOutputStatus, (const std::string &text), (override));
    MOCK_METHOD(void, showRateWarning, (const std::string &text), (override));
    MOCK_METHOD(void, showMuted, (bool muted), (override));
};

class MockOutputView : public IOutputView
{
public:
    MOCK_METHOD(void, showFrequencies, (const std::vector<double> &hz), (override));
    MOCK_METHOD(void, showSpectrum, (const std::vector<float> &db), (override));
    MOCK_METHOD(void, showBand, (const OutputBand &band), (override));
    MOCK_METHOD(void, showMeter, (float levelDb, float peakDb), (override));
    MOCK_METHOD(void, showCcTags, (const std::vector<CcTagInfo> &tags), (override));
};

class MockMapView : public IMapView
{
public:
    MOCK_METHOD(void, setCloudLayout, (const std::vector<MapPoint> &frames), (override));
    MOCK_METHOD(
        void, showCloudAlphas, (const std::vector<float> &dots, const std::vector<float> &segments), (override)
    );
    MOCK_METHOD(void, showRing, (MapPoint at, bool offMap), (override));
    MOCK_METHOD(void, showPins, (const std::vector<MapPin> &pins), (override));
    MOCK_METHOD(void, showNearness, (const MapNearness &nearness), (override));
    MOCK_METHOD(void, showPosition, (const std::string &text), (override));
    MOCK_METHOD(void, showCaption, (const MapCaption &caption), (override));
    MOCK_METHOD(void, showPlainPad, (bool plain), (override));
    MOCK_METHOD(void, showProjectionMenu, (const std::vector<ProjectionItem> &items), (override));
    MOCK_METHOD(void, showGlideTarget, (bool active, MapPoint at), (override));
    MOCK_METHOD(void, showCcTags, (const std::vector<CcTagInfo> &tags), (override));
    MOCK_METHOD(void, showLayer, (MapLayer layer, bool soundAvailable), (override));
    MOCK_METHOD(
        void, showSoundGrid, (std::size_t size, const std::vector<float> &brightness, const std::vector<float> &tonal),
        (override)
    );
};

class MockLatentPanelView : public ILatentPanelView
{
public:
    MOCK_METHOD(void, buildSliders, (const std::vector<LatentSliderSpec> &sliders), (override));
    MOCK_METHOD(void, showValue, (int index, float value, bool flagged), (override));
    MOCK_METHOD(
        void, showHistogram, (int index, const std::vector<float> &heights, float zeroShare, bool empty), (override)
    );
    MOCK_METHOD(
        void, showSweep, (int index, const std::vector<float> &brightness, const std::vector<float> &tonal), (override)
    );
    MOCK_METHOD(void, showWithoutStrips, (), (override));
    MOCK_METHOD(void, showCcTags, (const std::vector<CcTagInfo> &tags), (override));
};

class MockSnapshotsView : public ISnapshotsView
{
public:
    MOCK_METHOD(void, setThumbnailOutline, (const std::vector<MapPoint> &frames), (override));
    MOCK_METHOD(void, showSlot, (int slot, bool filled, std::optional<MapPoint> at), (override));
    MOCK_METHOD(void, showGlideMs, (int milliseconds), (override));
};
