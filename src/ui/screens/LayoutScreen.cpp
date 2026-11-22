#include "ui/screens/LayoutScreen.h"

#include "ui/Theme.h"

LayoutScreen::LayoutScreen(TFT_eSPI& tft, SettingsManager& settings, ScreenManager& nav)
    : tft_(tft), settings_(settings), nav_(nav) {}

UiRect LayoutScreen::rowRect(uint8_t index) const {
  return UiRect{6, static_cast<int16_t>(38 + index * 48), 308, 44};
}

void LayoutScreen::enter() {
  Theme::setActiveAccent(settings_.settings().accent);
  Theme::clear(tft_);
  Theme::drawHeader(tft_, "LAYOUT");

  uint16_t accentColor = Theme::activeAccent();
  for (uint8_t i = 0; i < static_cast<uint8_t>(LayoutId::Count); ++i) {
    LayoutId layout = static_cast<LayoutId>(i);
    bool selected = settings_.settings().layout == layout;
    UiRect row = rowRect(i);

    tft_.fillRoundRect(row.x, row.y, row.w, row.h, 7,
                       selected ? Theme::PanelSelected : Theme::Panel);
    tft_.drawRoundRect(row.x, row.y, row.w, row.h, 7, selected ? accentColor : Theme::Stroke);

    UiRect glyph{static_cast<int16_t>(row.x + 9), static_cast<int16_t>(row.y + 8), 40, 28};
    Theme::drawLayoutGlyph(tft_, glyph, layout, selected ? accentColor : Theme::TextDim);

    tft_.setFreeFont(FONT_TITLE);
    tft_.setTextColor(selected ? Theme::Text : Theme::TextDim,
                      selected ? Theme::PanelSelected : Theme::Panel);
    tft_.setTextDatum(ML_DATUM);
    tft_.drawString(layoutName(layout), row.x + 58, row.y + 15);

    tft_.setFreeFont(FONT_LABEL);
    tft_.setTextColor(selected ? Theme::TextDim : Theme::TextFaint,
                      selected ? Theme::PanelSelected : Theme::Panel);
    tft_.setTextDatum(ML_DATUM);
    tft_.drawString(layoutDescription(layout), row.x + 58, row.y + 31);

    if (selected) {
      tft_.fillCircle(row.x + row.w - 20, row.y + row.h / 2, 8, accentColor);
      Theme::drawCheck(tft_, row.x + row.w - 20, row.y + row.h / 2, Theme::Bg);
    } else {
      tft_.drawCircle(row.x + row.w - 20, row.y + row.h / 2, 7, Theme::Stroke);
    }
  }
}

void LayoutScreen::handleTouch(int16_t x, int16_t y) {
  if (Theme::backButtonRect().contains(x, y)) {
    nav_.back();
    return;
  }

  for (uint8_t i = 0; i < static_cast<uint8_t>(LayoutId::Count); ++i) {
    if (!rowRect(i).contains(x, y)) continue;
    settings_.setLayout(static_cast<LayoutId>(i));
    enter();
    return;
  }
}
