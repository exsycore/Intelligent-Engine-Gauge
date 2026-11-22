#include "ui/screens/SettingsMenuScreen.h"

#include "ui/Theme.h"

SettingsMenuScreen::SettingsMenuScreen(TFT_eSPI& tft, SettingsManager& settings, ScreenManager& nav)
    : tft_(tft), settings_(settings), nav_(nav) {}

UiRect SettingsMenuScreen::rowRect(uint8_t index) const {
  return UiRect{4, static_cast<int16_t>(34 + index * 29), 312, 27};
}

void SettingsMenuScreen::enter() {
  Theme::setActiveAccent(settings_.settings().accent);
  Theme::clear(tft_);
  Theme::drawHeader(tft_, "SETTINGS");

  const AppSettings& s = settings_.settings();
  Theme::drawRow(tft_, rowRect(0), "Dashboard Layout", layoutName(s.layout), true);
  Theme::drawRow(tft_, rowRect(1), "Primary Gauge", metricShortName(s.primaryMetric), true);
  Theme::drawRow(tft_, rowRect(2), "Secondary Gauges", "3 SLOTS", true);
  Theme::drawRow(tft_, rowRect(3), "Appearance", accentName(s.accent), true);
  Theme::drawRow(tft_, rowRect(4), "Display Settings", brightnessName(s.brightness), true);
  Theme::drawRow(tft_, rowRect(5), "Simulator", scenarioName(s.scenario), true);
  Theme::drawRow(tft_, rowRect(6), "Reset Settings", nullptr, false, true);
}

void SettingsMenuScreen::handleTouch(int16_t x, int16_t y) {
  if (Theme::backButtonRect().contains(x, y)) {
    nav_.back();
    return;
  }

  for (uint8_t i = 0; i < 7; ++i) {
    if (!rowRect(i).contains(x, y)) continue;
    switch (i) {
      case 0:
        nav_.show(ScreenId::LayoutSelect);
        break;
      case 1:
        nav_.show(ScreenId::PrimaryGauge);
        break;
      case 2:
        nav_.show(ScreenId::SecondaryGauges);
        break;
      case 3:
        nav_.show(ScreenId::Appearance);
        break;
      case 4:
        nav_.show(ScreenId::Display);
        break;
      case 5:
        nav_.show(ScreenId::MockTelemetry);
        break;
      case 6:
        nav_.show(ScreenId::ResetConfirm);
        break;
      default:
        break;
    }
    return;
  }
}
