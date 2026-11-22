#include "ui/screens/ResetScreen.h"

#include "ui/Theme.h"

ResetScreen::ResetScreen(TFT_eSPI& tft, SettingsManager& settings, ScreenManager& nav)
    : tft_(tft), settings_(settings), nav_(nav) {}

void ResetScreen::enter() {
  Theme::setActiveAccent(settings_.settings().accent);
  Theme::clear(tft_);
  Theme::drawHeader(tft_, "RESET SETTINGS");

  Theme::drawWarningTriangle(tft_, 160, 64, Theme::Warn);

  tft_.setFreeFont(FONT_TITLE);
  tft_.setTextColor(Theme::Text, Theme::Bg);
  tft_.setTextDatum(MC_DATUM);
  tft_.drawString("Restore factory defaults?", 160, 94);

  tft_.setFreeFont(FONT_LABEL);
  tft_.setTextColor(Theme::TextDim, Theme::Bg);
  tft_.setTextDatum(MC_DATUM);
  tft_.drawString("Dashboard, gauges, appearance and", 160, 120);
  tft_.drawString("simulator settings will be reset.", 160, 136);

  tft_.fillRoundRect(12, 158, 140, 44, 7, Theme::Panel);
  tft_.drawRoundRect(12, 158, 140, 44, 7, Theme::Stroke);
  tft_.setFreeFont(FONT_LABEL_BOLD);
  tft_.setTextColor(Theme::Text, Theme::Panel);
  tft_.setTextDatum(MC_DATUM);
  tft_.drawString("CANCEL", 82, 180);

  uint16_t dangerFill = Theme::rgb565(48, 20, 24);
  tft_.fillRoundRect(168, 158, 140, 44, 7, dangerFill);
  tft_.drawRoundRect(168, 158, 140, 44, 7, Theme::Danger);
  tft_.setTextColor(Theme::Danger, dangerFill);
  tft_.drawString("RESET", 238, 180);

  tft_.setFreeFont(FONT_LABEL);
  tft_.setTextColor(Theme::TextFaint, Theme::Bg);
  tft_.setTextDatum(MC_DATUM);
  tft_.drawString("Saved settings will be overwritten.", 160, 224);
}

void ResetScreen::handleTouch(int16_t x, int16_t y) {
  if (Theme::backButtonRect().contains(x, y)) {
    nav_.back();
    return;
  }

  if (UiRect{12, 158, 140, 44}.contains(x, y)) {
    nav_.back();
    return;
  }

  if (UiRect{168, 158, 140, 44}.contains(x, y)) {
    settings_.resetDefaults();
    nav_.back();
  }
}
