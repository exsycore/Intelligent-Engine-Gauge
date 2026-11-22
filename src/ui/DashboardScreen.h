#pragma once

#include <TFT_eSPI.h>

#include "settings/SettingsManager.h"
#include "telemetry/Telemetry.h"
#include "ui/GaugeRenderer.h"
#include "ui/Screen.h"
#include "ui/ScreenManager.h"

class DashboardScreen : public Screen {
 public:
  DashboardScreen(TFT_eSPI& tft, SettingsManager& settings, ITelemetryProvider& telemetry,
                  ScreenManager& nav);

  void enter() override;
  void update() override;
  void handleTouch(int16_t x, int16_t y) override;

 private:
  struct PrimarySpec {
    UiRect card;
    bool cardVisible = true;
    ArcSpec arc;
    UiRect bar;
    UiRect valueArea;
    int16_t labelX = 0;
    int16_t labelY = 0;
    int16_t labelMaxWidth = 160;
    uint8_t labelDatum = MC_DATUM;
    int16_t unitX = 0;
    int16_t unitY = 0;
    uint8_t unitDatum = MC_DATUM;
    bool showScaleLabels = false;
    bool useArc = true;
    bool useBar = false;
    bool segmentedBar = false;
  };

  struct SlotState {
    char value[16] = {};
    float progress = 0.0f;
    bool warning = false;
    bool valid = false;
  };

  PrimarySpec buildPrimarySpec(LayoutId layout) const;
  UiRect secondaryCard(int index) const;
  float progressFor(MetricId metric, float value) const;
  uint16_t primaryColor() const;
  uint16_t valueBackground() const;

  void drawFull();
  void drawPrimaryStatic();
  void drawPrimaryValueText();
  void drawScaleLabels();
  void drawMinimalSecondary(int index, bool full);
  void drawStatus();

  TFT_eSPI& tft_;
  SettingsManager& settings_;
  ITelemetryProvider& telemetry_;
  ScreenManager& nav_;

  LayoutId layout_ = LayoutId::Classic;
  PrimarySpec primary_;
  SlotState slots_[SecondarySlotCount];
  char primaryValue_[16] = {};
  bool primaryWarning_ = false;
  float primaryProgress_ = 0.0f;
  uint32_t lastVersion_ = 0;
  const char* lastWarningText_ = nullptr;
};
