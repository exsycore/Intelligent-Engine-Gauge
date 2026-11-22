#pragma once

#include "core/AppTypes.h"
#include "storage/SettingsStorage.h"

// Owns the validated application settings and persists changes. UI code only
// talks to this class; it never touches NVS directly.
class SettingsManager {
 public:
  bool begin();

  const AppSettings& settings() const { return settings_; }
  uint32_t revision() const { return revision_; }
  bool persistenceOk() const { return persistenceOk_; }

  void setLayout(LayoutId layout);
  void setPrimaryStyle(GaugeStyle style);
  void setIndicatorStyle(IndicatorStyle style);
  void setAccent(AccentId accent);
  void setScenario(ScenarioId scenario);
  void setUpdateRate(UpdateRate rate);
  void setBrightness(BrightnessLevel level);
  void setShowTicks(bool enabled);
  void setShowIndicators(bool enabled);
  bool assignMetric(uint8_t slot, MetricId metric);

  void resetDefaults();

 private:
  void commit();

  SettingsStorage storage_;
  AppSettings settings_ = defaultSettings();
  uint32_t revision_ = 0;
  bool persistenceOk_ = false;
};
