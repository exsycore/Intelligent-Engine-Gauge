#pragma once

#include <TFT_eSPI.h>

#include "hardware/TouchPanel.h"
#include "settings/SettingsManager.h"
#include "telemetry/MockTelemetry.h"
#include "ui/ScreenManager.h"
#include "ui/screens/AppearanceScreen.h"
#include "ui/DashboardScreen.h"
#include "ui/screens/DisplayScreen.h"
#include "ui/screens/GaugeConfigScreen.h"
#include "ui/screens/LayoutScreen.h"
#include "ui/screens/MetricPickerScreen.h"
#include "ui/screens/MockScreen.h"
#include "ui/screens/ResetScreen.h"
#include "ui/screens/SettingsMenuScreen.h"

class UiApp {
 public:
  UiApp();
  void begin();
  void loop();

 private:
  void syncServices();
  void applyBacklight();

  TFT_eSPI tft_;
  TouchPanel touch_;
  MockTelemetryProvider mock_;
  SettingsManager settings_;
  ScreenManager nav_;

  DashboardScreen dashboard_;
  SettingsMenuScreen settingsMenu_;
  LayoutScreen layoutScreen_;
  MetricPickerScreen metricPicker_;
  GaugeConfigScreen primaryGaugeScreen_;
  GaugeConfigScreen secondaryGaugeScreen_;
  AppearanceScreen appearanceScreen_;
  DisplayScreen displayScreen_;
  MockScreen mockScreen_;
  ResetScreen resetScreen_;

  bool touchWasDown_ = false;
  bool backlightReady_ = false;
  uint8_t appliedBacklight_ = 255;
};
