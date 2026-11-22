#include "settings/SettingsManager.h"

#include <Arduino.h>

bool SettingsManager::begin() {
  bool storageReady = storage_.begin();
  bool loaded = storageReady && storage_.load(settings_);

  if (!loaded) {
    settings_ = defaultSettings();
  }

  bool changed = sanitizeSettings(settings_);
  revision_ = loaded ? 1 : 2;

  if (storageReady && (!loaded || changed)) {
    persistenceOk_ = storage_.save(settings_);
  } else {
    persistenceOk_ = storageReady;
  }

  Serial.printf("[settings] source=%s persisted=%s layout=%s\n", loaded ? "nvs" : "defaults",
                persistenceOk_ ? "yes" : "no", layoutName(settings_.layout));
  return storageReady;
}

void SettingsManager::commit() {
  sanitizeSettings(settings_);
  persistenceOk_ = storage_.save(settings_);
  ++revision_;
}

void SettingsManager::setLayout(LayoutId layout) {
  if (static_cast<uint8_t>(layout) >= static_cast<uint8_t>(LayoutId::Count)) return;
  if (settings_.layout == layout) return;
  settings_.layout = layout;
  commit();
}

void SettingsManager::setPrimaryStyle(GaugeStyle style) {
  if (static_cast<uint8_t>(style) >= static_cast<uint8_t>(GaugeStyle::Count)) return;
  if (settings_.primaryStyle == style) return;
  settings_.primaryStyle = style;
  commit();
}

void SettingsManager::setIndicatorStyle(IndicatorStyle style) {
  if (static_cast<uint8_t>(style) >= static_cast<uint8_t>(IndicatorStyle::Count)) return;
  if (settings_.indicatorStyle == style) return;
  settings_.indicatorStyle = style;
  commit();
}

void SettingsManager::setAccent(AccentId accent) {
  if (static_cast<uint8_t>(accent) >= static_cast<uint8_t>(AccentId::Count)) return;
  if (settings_.accent == accent) return;
  settings_.accent = accent;
  commit();
}

void SettingsManager::setScenario(ScenarioId scenario) {
  if (static_cast<uint8_t>(scenario) >= static_cast<uint8_t>(ScenarioId::Count)) return;
  if (settings_.scenario == scenario) return;
  settings_.scenario = scenario;
  commit();
}

void SettingsManager::setUpdateRate(UpdateRate rate) {
  if (static_cast<uint8_t>(rate) >= static_cast<uint8_t>(UpdateRate::Count)) return;
  if (settings_.updateRate == rate) return;
  settings_.updateRate = rate;
  commit();
}

void SettingsManager::setBrightness(BrightnessLevel level) {
  if (static_cast<uint8_t>(level) >= static_cast<uint8_t>(BrightnessLevel::Count)) return;
  if (settings_.brightness == level) return;
  settings_.brightness = level;
  commit();
}

void SettingsManager::setShowTicks(bool enabled) {
  if (settings_.showTicks == enabled) return;
  settings_.showTicks = enabled;
  commit();
}

void SettingsManager::setShowIndicators(bool enabled) {
  if (settings_.showIndicators == enabled) return;
  settings_.showIndicators = enabled;
  commit();
}

bool SettingsManager::assignMetric(uint8_t slot, MetricId metric) {
  if (!assignMetricUnique(settings_, slot, metric)) return false;
  commit();
  return true;
}

void SettingsManager::resetDefaults() {
  settings_ = defaultSettings();
  commit();
}
