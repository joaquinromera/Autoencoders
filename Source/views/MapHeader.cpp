/// @file MapHeader.cpp
/// @brief Implementation of MapHeader

#include <MapHeader.h>

#include <StringDefinitions.h>
#include <Theme.h>

namespace
{
constexpr int kChipWidth = 96;

// Menu ids: axis pairs are 1..n (their id + 1), layers sit far above them
constexpr int kPieceItem = 10'001;
constexpr int kSoundItem = 10'002;
} // namespace

MapHeader::MapHeader(juce::String letter)
    : mLetter(std::move(letter))
{
    mProjectionButton.setButtonText(Theme::utf8(Strings::Ui::View));
    mProjectionButton.setColour(juce::TextButton::buttonColourId, Theme::background);
    mProjectionButton.setColour(juce::TextButton::textColourOffId, Theme::muted);
    mProjectionButton.onClick = [this] {
        openMenu();
    };
    addAndMakeVisible(mProjectionButton);
}

void MapHeader::setCaption(const std::string &title, const std::string &subtitle)
{
    mTitle    = Theme::utf8(title);
    mSubtitle = Theme::utf8(subtitle);
    repaint();
}

void MapHeader::setProjections(std::vector<ProjectionItem> items)
{
    mProjections = std::move(items);
}

void MapHeader::setLayer(MapLayer layer, bool soundAvailable)
{
    mLayer          = layer;
    mSoundAvailable = soundAvailable;
}

void MapHeader::paint(juce::Graphics &g)
{
    auto       header = getLocalBounds().toFloat();
    const auto pad    = header.removeFromLeft(17.0f).withSizeKeepingCentre(17.0f, 17.0f);
    g.setColour(Theme::line2);
    g.drawRoundedRectangle(pad, 4.0f, 1.0f);
    g.setColour(Theme::muted);
    g.setFont(Theme::mono(10.5f));
    g.drawText(mLetter, pad, juce::Justification::centred);

    header.removeFromLeft(8.0f);
    g.setColour(Theme::text);
    g.setFont(Theme::font(13.0f, true));
    g.drawText(mTitle, header, juce::Justification::centredLeft);
    header.removeFromLeft(
        static_cast<float>(juce::GlyphArrangement::getStringWidthInt(Theme::font(13.0f, true), mTitle)) + 8.0f
    );
    g.setColour(Theme::muted);
    g.setFont(Theme::font(12.5f));
    g.drawText(mSubtitle, header, juce::Justification::centredLeft);
}

void MapHeader::resized()
{
    mProjectionButton.setBounds(getLocalBounds().removeFromRight(kChipWidth).reduced(0, 3));
}

void MapHeader::openMenu()
{
    juce::PopupMenu menu;
    addLayerItems(menu);
    addProjectionItems(menu);
    menu.showMenuAsync(
        juce::PopupMenu::Options().withTargetComponent(mProjectionButton),
        [safe = juce::Component::SafePointer<MapHeader>(this)](int result) {
            if (safe == nullptr || result <= 0)
            {
                return;
            }
            if (result >= kPieceItem && safe->onLayerSelected)
            {
                safe->onLayerSelected(result == kSoundItem ? MapLayer::sound : MapLayer::piece);
            }
            else if (result < kPieceItem && safe->onProjectionSelected)
            {
                safe->onProjectionSelected(result - 1);
            }
        }
    );
}

void MapHeader::addLayerItems(juce::PopupMenu &menu) const
{
    if (!mSoundAvailable)
    {
        return;
    }
    menu.addSectionHeader(Strings::Ui::ShowSection);
    menu.addItem(kPieceItem, Theme::utf8(Strings::Ui::ShowPiece), true, mLayer == MapLayer::piece);
    menu.addItem(kSoundItem, Theme::utf8(Strings::Ui::ShowSound), true, mLayer == MapLayer::sound);
}

void MapHeader::addProjectionItems(juce::PopupMenu &menu) const
{
    juce::PopupMenu raw;
    bool            principalHeader = false;
    for (const auto &item : mProjections)
    {
        if (!item.principal)
        {
            raw.addItem(item.id + 1, Theme::utf8(item.label), true, item.selected);
            continue;
        }
        if (!std::exchange(principalHeader, true))
        {
            menu.addSectionHeader(Theme::utf8(Strings::Ui::PrincipalPairs));
        }
        menu.addItem(item.id + 1, Theme::utf8(item.label), true, item.selected);
    }
    menu.addSubMenu(Theme::utf8(Strings::Ui::RawPairs), raw);
}
