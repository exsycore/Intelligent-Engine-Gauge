#pragma once

#include <TFT_eSPI.h>

#include "settings/SettingsManager.h"
#include "ui/Screen.h"
#include "ui/ScreenManager.h"

class LayoutScreen : public Screen {
 public:
  LayoutScreen(TFT_eSPI& tft, SettingsManager& settings, ScreenManager& nav);

  void enter() override;
  void handleTouch(int16_t x, int16_t y) override;

 private:
  UiRect rowRect(uint8_t index) const;

  TFT_eSPI& tft_;
  SettingsManager& settings_;
  ScreenManager& nav_;
};
