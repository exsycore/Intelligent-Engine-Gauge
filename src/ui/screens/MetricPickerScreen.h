#pragma once

#include <TFT_eSPI.h>

#include "settings/SettingsManager.h"
#include "ui/Screen.h"
#include "ui/ScreenManager.h"

class MetricPickerScreen : public Screen {
 public:
  MetricPickerScreen(TFT_eSPI& tft, SettingsManager& settings, ScreenManager& nav);

  void setTargetSlot(uint8_t slot) { targetSlot_ = slot; }
  void enter() override;
  void handleTouch(int16_t x, int16_t y) override;

 private:
  UiRect rowRect(uint8_t index) const;

  TFT_eSPI& tft_;
  SettingsManager& settings_;
  ScreenManager& nav_;
  uint8_t targetSlot_ = 0;
};
