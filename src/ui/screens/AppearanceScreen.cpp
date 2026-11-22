#include "ui/screens/AppearanceScreen.h"

#include "ui/Theme.h"

AppearanceScreen::AppearanceScreen(TFT_eSPI& tft, SettingsManager& settings, ScreenManager& nav)
    : tft_(tft), settings_(settings), nav_(nav) {}

UiRect AppearanceScreen::swatchRect(uint8_t index) const {
  return UiRect{static_cast<int16_t>(8 + index * 76), 50, 70, 60};
}

void AppearanceScreen::enter() {
  Theme::setActiveAccent(settings_.settings().accent);
  Theme::clear(tft_);
  Theme::drawHeader(tft_, "APPEARANCE");

  Theme::drawSectionLabel(tft_, "ACCENT COLOR", 10, 38);

  for (uint8_t i = 0; i < static_cast<uint8_t>(AccentId::Count); ++i) {
    AccentId id = static_cast<AccentId>(i);
    bool selected = settings_.settings().accent == id;
    UiRect rect = swatchRect(i);
    uint16_t color = Theme::accent(id);

    tft_.fillRoundRect(rect.x, rect.y, rect.w, rect.h, 7,
                       selected ? Theme::PanelSelected : Theme::Panel);
    tft_.drawRoundRect(rect.x, rect.y, rect.w, rect.h, 7,
                       selected ? color : Theme::Stroke);
    tft_.fillCircle(rect.x + rect.w / 2, rect.y + 21, 12, color);
    if (selected) {
      Theme::drawCheck(tft_, rect.x + rect.w / 2, rect.y + 21, Theme::Bg);
    }

    tft_.setFreeFont(FONT_LABEL_BOLD);
    tft_.setTextColor(selected ? Theme::Text : Theme::TextDim,
                      selected ? Theme::PanelSelected : Theme::Panel);
    tft_.setTextDatum(MC_DATUM);
    tft_.drawString(accentName(id), rect.x + rect.w / 2, rect.y + 44);
  }

  Theme::drawToggleRow(tft_, UiRect{4, 122, 312, 40}, "TICK MARKS",
                       settings_.settings().showTicks);
  Theme::drawToggleRow(tft_, UiRect{4, 170, 312, 40}, "CARD INDICATORS",
                       settings_.settings().showIndicators);

  tft_.setFreeFont(FONT_LABEL);
  tft_.setTextColor(Theme::TextFaint, Theme::Bg);
  tft_.setTextDatum(MC_DATUM);
  tft_.drawString("Accent tints gauges and highlights.", 160, 222);
}

void AppearanceScreen::handleTouch(int16_t x, int16_t y) {
  if (Theme::backButtonRect().contains(x, y)) {
    nav_.back();
    return;
  }

  for (uint8_t i = 0; i < static_cast<uint8_t>(AccentId::Count); ++i) {
    if (swatchRect(i).contains(x, y)) {
      settings_.setAccent(static_cast<AccentId>(i));
      enter();
      return;
    }
  }

  if (UiRect{4, 122, 312, 40}.contains(x, y)) {
    settings_.setShowTicks(!settings_.settings().showTicks);
    enter();
  } else if (UiRect{4, 170, 312, 40}.contains(x, y)) {
    settings_.setShowIndicators(!settings_.settings().showIndicators);
    enter();
  }
}
