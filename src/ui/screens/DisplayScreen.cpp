#include <stdio.h>
#include "ui/screens/DisplayScreen.h"

#include "ui/Theme.h"

namespace {
const char* compactBrightnessName(BrightnessLevel level) {
  switch (level) {
    case BrightnessLevel::Low:
      return "LOW";
    case BrightnessLevel::High:
      return "HIGH";
    case BrightnessLevel::Medium:
    default:
      return "MED";
  }
}
}  // namespace

DisplayScreen::DisplayScreen(TFT_eSPI& tft, SettingsManager& settings, ScreenManager& nav)
    : tft_(tft), settings_(settings), nav_(nav) {}

UiRect DisplayScreen::brightnessButton(uint8_t index) const {
  static const int16_t xs[3] = {8, 112, 216};
  return UiRect{xs[index < 3 ? index : 0], 50, 96, 52};
}

void DisplayScreen::enter() {
  Theme::setActiveAccent(settings_.settings().accent);
  Theme::clear(tft_);
  Theme::drawHeader(tft_, "DISPLAY");
  Theme::drawSectionLabel(tft_, "BACKLIGHT", 10, 40);

  uint16_t accentColor = Theme::activeAccent();
  for (uint8_t i = 0; i < 3; ++i) {
    BrightnessLevel level = static_cast<BrightnessLevel>(i);
    bool selected = settings_.settings().brightness == level;
    UiRect rect = brightnessButton(i);

    tft_.fillRoundRect(rect.x, rect.y, rect.w, rect.h, 7,
                       selected ? Theme::PanelSelected : Theme::Panel);
    tft_.drawRoundRect(rect.x, rect.y, rect.w, rect.h, 7,
                       selected ? accentColor : Theme::Stroke);

    tft_.setFreeFont(FONT_TITLE);
    tft_.setTextColor(selected ? accentColor : Theme::TextDim,
                      selected ? Theme::PanelSelected : Theme::Panel);
    tft_.setTextDatum(MC_DATUM);
    tft_.drawString(compactBrightnessName(level), rect.x + rect.w / 2, rect.y + 20);

    char percent[8];
    snprintf(percent, sizeof(percent), "%u%%", static_cast<unsigned>(brightnessPercent(level)));
    tft_.setFreeFont(FONT_LABEL);
    tft_.setTextColor(Theme::TextFaint, selected ? Theme::PanelSelected : Theme::Panel);
    tft_.setTextDatum(MC_DATUM);
    tft_.drawString(percent, rect.x + rect.w / 2, rect.y + 39);
  }

  tft_.drawFastHLine(8, 116, 304, Theme::Stroke);

  tft_.setFreeFont(FONT_LABEL);
  tft_.setTextColor(Theme::TextDim, Theme::Bg);
  tft_.setTextDatum(ML_DATUM);
  tft_.drawString("RESOLUTION", 10, 136);
  tft_.setTextColor(Theme::Text, Theme::Bg);
  tft_.setTextDatum(MR_DATUM);
  tft_.drawString("320 x 240", 310, 136);

  tft_.setTextColor(Theme::TextDim, Theme::Bg);
  tft_.setTextDatum(ML_DATUM);
  tft_.drawString("TOUCH PANEL", 10, 158);
  tft_.setTextColor(Theme::Text, Theme::Bg);
  tft_.setTextDatum(MR_DATUM);
  tft_.drawString("XPT2046 RESISTIVE", 310, 158);

  tft_.setTextColor(Theme::TextDim, Theme::Bg);
  tft_.setTextDatum(ML_DATUM);
  tft_.drawString("ORIENTATION", 10, 180);
  tft_.setTextColor(Theme::Text, Theme::Bg);
  tft_.setTextDatum(MR_DATUM);
  tft_.drawString("LANDSCAPE", 310, 180);

  tft_.setTextColor(Theme::TextFaint, Theme::Bg);
  tft_.setTextDatum(MC_DATUM);
  tft_.drawString("Brightness saves automatically.", 160, 212);
}

void DisplayScreen::handleTouch(int16_t x, int16_t y) {
  if (Theme::backButtonRect().contains(x, y)) {
    nav_.back();
    return;
  }

  for (uint8_t i = 0; i < 3; ++i) {
    if (brightnessButton(i).contains(x, y)) {
      settings_.setBrightness(static_cast<BrightnessLevel>(i));
      enter();
      return;
    }
  }
}
