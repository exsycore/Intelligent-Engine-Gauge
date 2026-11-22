#include <stdio.h>
#include "ui/screens/MockScreen.h"

#include "ui/Theme.h"

namespace {
const char* compactRateName(UpdateRate rate) {
  switch (rate) {
    case UpdateRate::Slow:
      return "SLOW";
    case UpdateRate::Fast:
      return "FAST";
    case UpdateRate::Normal:
    default:
      return "NORM";
  }
}
}  // namespace

MockScreen::MockScreen(TFT_eSPI& tft, SettingsManager& settings, ScreenManager& nav)
    : tft_(tft), settings_(settings), nav_(nav) {}

UiRect MockScreen::scenarioRect(uint8_t index) const {
  return UiRect{4, static_cast<int16_t>(34 + index * 30), 312, 28};
}

UiRect MockScreen::rateButton(uint8_t index) const {
  static const int16_t xs[3] = {52, 138, 224};
  return UiRect{xs[index < 3 ? index : 0], 191, 80, 26};
}

void MockScreen::enter() {
  Theme::setActiveAccent(settings_.settings().accent);
  Theme::clear(tft_);
  Theme::drawHeader(tft_, "TELEMETRY", "DEMO");

  uint16_t accentColor = Theme::activeAccent();
  for (uint8_t i = 0; i < static_cast<uint8_t>(ScenarioId::Count); ++i) {
    ScenarioId scenario = static_cast<ScenarioId>(i);
    bool selected = settings_.settings().scenario == scenario;
    UiRect row = scenarioRect(i);
    uint16_t fill = selected ? Theme::PanelSelected : Theme::Panel;

    tft_.fillRoundRect(row.x, row.y, row.w, row.h, 5, fill);
    tft_.drawRoundRect(row.x, row.y, row.w, row.h, 5, selected ? accentColor : Theme::Stroke);
    if (selected) {
      tft_.fillRoundRect(row.x + 1, row.y + 7, 3, row.h - 14, 2, accentColor);
    }

    const char* name = scenarioName(scenario);
    int16_t nameWidth = static_cast<int16_t>(Theme::textWidth(tft_, name, FONT_LABEL_BOLD));
    int16_t descMax = static_cast<int16_t>(row.w - 34 - nameWidth);
    Theme::drawTextElided(tft_, name, row.x + 12, row.y + row.h / 2, FONT_LABEL_BOLD,
                          selected ? Theme::Text : Theme::TextDim, fill, ML_DATUM,
                          static_cast<int16_t>(row.w - 24));
    Theme::drawTextElided(tft_, scenarioDescription(scenario), row.x + row.w - 12,
                          row.y + row.h / 2, FONT_LABEL,
                          selected ? Theme::TextDim : Theme::TextFaint, fill, MR_DATUM, descMax);
  }

  tft_.setFreeFont(FONT_LABEL);
  tft_.setTextColor(Theme::TextDim, Theme::Bg);
  tft_.setTextDatum(ML_DATUM);
  tft_.drawString("RATE", 10, 204);

  for (uint8_t i = 0; i < 3; ++i) {
    UpdateRate rate = static_cast<UpdateRate>(i);
    bool selected = settings_.settings().updateRate == rate;
    UiRect rect = rateButton(i);
    tft_.fillRoundRect(rect.x, rect.y, rect.w, rect.h, 5,
                       selected ? accentColor : Theme::Panel);
    tft_.drawRoundRect(rect.x, rect.y, rect.w, rect.h, 5,
                       selected ? accentColor : Theme::Stroke);
    tft_.setFreeFont(FONT_LABEL_BOLD);
    tft_.setTextColor(selected ? Theme::Bg : Theme::TextDim,
                      selected ? accentColor : Theme::Panel);
    tft_.setTextDatum(MC_DATUM);
    tft_.drawString(compactRateName(rate), rect.x + rect.w / 2, rect.y + rect.h / 2);
  }

  tft_.setFreeFont(FONT_LABEL);
  tft_.setTextColor(Theme::TextFaint, Theme::Bg);
  tft_.setTextDatum(MC_DATUM);
  tft_.drawString("Simulated data - no sensors", 160, 231);
}

void MockScreen::handleTouch(int16_t x, int16_t y) {
  if (Theme::backButtonRect().contains(x, y)) {
    nav_.back();
    return;
  }

  for (uint8_t i = 0; i < static_cast<uint8_t>(ScenarioId::Count); ++i) {
    if (scenarioRect(i).contains(x, y)) {
      settings_.setScenario(static_cast<ScenarioId>(i));
      enter();
      return;
    }
  }

  for (uint8_t i = 0; i < 3; ++i) {
    if (rateButton(i).contains(x, y)) {
      settings_.setUpdateRate(static_cast<UpdateRate>(i));
      enter();
      return;
    }
  }
}
