/// @file MapHeader.h
/// @brief The strip above a map: its letter, axes, variance and View menu

#pragma once

#include <IMapView.h>

#include <juce_gui_basics/juce_gui_basics.h>

#include <functional>
#include <vector>

/// @brief Header of one map, with the View chip: what the map shows and on which axes
class MapHeader : public juce::Component
{
public:
    static constexpr int kHeight = 26;

    explicit MapHeader(juce::String letter);

    /// @brief An axis pair of the View menu was chosen
    std::function<void(int id)> onProjectionSelected;

    /// @brief A layer of the View menu was chosen
    std::function<void(MapLayer)> onLayerSelected;

    void setCaption(const std::string &title, const std::string &subtitle);
    void setProjections(std::vector<ProjectionItem> items);

    /// @brief The layer the menu ticks; the Show section is left out when the sound is not available
    void setLayer(MapLayer layer, bool soundAvailable);

    void paint(juce::Graphics &g) override;
    void resized() override;

private:
    void openMenu();
    void addLayerItems(juce::PopupMenu &menu) const;
    void addProjectionItems(juce::PopupMenu &menu) const;

    juce::String                mLetter;
    juce::String                mTitle;
    juce::String                mSubtitle;
    std::vector<ProjectionItem> mProjections;
    MapLayer                    mLayer          = MapLayer::piece;
    bool                        mSoundAvailable = false;
    juce::TextButton            mProjectionButton;
};
