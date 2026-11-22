#pragma once

#include <TFT_eSPI.h>

#include "settings/SettingsManager.h"
#include "ui/Screen.h"
#include "ui/ScreenManager.h"
#include "ui/screens/MetricPickerScreen.h"

class GaugeConfigScreen : public Screen {
 public:
  GaugeConfigScreen(TFT_eSPI& tft, SettingsManager& settings, ScreenManager& nav,
                    MetricPickerScreen& picker, bool primaryMode);

  void enter() override;
  void handleTouch(int16_t x, int16_t y) override;

 private:
  void drawFieldRow(const UiRect& rect, const char* caption, const char* value);
  void drawSegmentButton(const UiRect& rect, const char* label, bool selected);

  TFT_eSPI& tft_;
  SettingsManager& settings_;
  ScreenManager& nav_;
  MetricPickerScreen& picker_;
  bool primaryMode_;
};
