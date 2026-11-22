#include <math.h>
#include "core/AppTypes.h"

#include <stdio.h>
#include <string.h>

namespace {

const MetricInfo kMetrics[MetricCount] = {
    {"ENGINE RPM", "ENGINE RPM", "RPM", 0.0f, 8000.0f, 0, true},
    {"ENGINE TEMP", "ENG TEMP", "\xB0" "C", 40.0f, 130.0f, 0, false},
    {"BATTERY", "BATTERY", "V", 10.0f, 16.0f, 1, false},
    {"VEHICLE SPEED", "SPEED", "km/h", 0.0f, 220.0f, 0, false},
    {"THROTTLE", "THROTTLE", "%", 0.0f, 100.0f, 0, false},
    {"FUEL LEVEL", "FUEL", "%", 0.0f, 100.0f, 0, false},
    {"INTAKE AIR", "INTAKE", "\xB0" "C", -20.0f, 80.0f, 0, false},
};

// Card labels are kept under ~7 characters so they fit a 98 px wide tile.
const char* const kCardLabels[MetricCount] = {"RPM", "TEMP", "BATT", "SPEED",
                                             "THROT", "FUEL", "INTAKE"};

const char* const kLayoutNames[] = {"CLASSIC", "SPORT", "MINIMAL", "TECHNICAL"};
const char* const kLayoutDescriptions[] = {
    "Balanced cluster",
    "Wide tach, compact tiles",
    "Clean type, no clutter",
    "Dense telemetry detail",
};
const char* const kGaugeStyleNames[] = {"ARC", "BAR", "NUMERIC"};
const char* const kIndicatorStyleNames[] = {"BAR", "DOTS"};
const char* const kAccentNames[] = {"ICE", "RED", "AMBER", "GREEN"};
const char* const kScenarioNames[] = {"ENGINE IDLE", "NORMAL DRIVING", "ACCELERATION",
                                      "HIGH RPM", "WARNING"};
const char* const kScenarioDescriptions[] = {
    "Parked, warm",
    "Steady cruise",
    "Gear pulls",
    "Redline load run",
    "Fault conditions",
};

uint8_t clampIndex(int value, uint8_t count) {
  if (value < 0) return 0;
  if (value >= static_cast<int>(count)) return static_cast<uint8_t>(count - 1);
  return static_cast<uint8_t>(value);
}

void writeInteger(int32_t value, bool thousands, char* out, size_t outSize) {
  char digits[16];
  bool negative = value < 0;
  uint32_t magnitude = negative ? static_cast<uint32_t>(-value) : static_cast<uint32_t>(value);

  int len = 0;
  do {
    digits[len++] = static_cast<char>('0' + (magnitude % 10));
    magnitude /= 10;
  } while (magnitude > 0 && len < static_cast<int>(sizeof(digits)));

  size_t pos = 0;
  if (negative && pos + 1 < outSize) out[pos++] = '-';

  for (int i = len - 1; i >= 0; --i) {
    if (pos + 1 < outSize) out[pos++] = digits[i];
    if (thousands && i > 0 && (i % 3) == 0 && pos + 1 < outSize) out[pos++] = ',';
  }
  out[pos] = '\0';
}

}  // namespace

const MetricInfo& metricInfo(MetricId id) {
  uint8_t index = static_cast<uint8_t>(id);
  if (index >= MetricCount) index = 0;
  return kMetrics[index];
}

const char* metricShortName(MetricId id) {
  return metricInfo(id).shortName;
}

const char* metricUnit(MetricId id) {
  return metricInfo(id).unit;
}

const char* metricCardLabel(MetricId id) {
  uint8_t index = static_cast<uint8_t>(id);
  if (index >= MetricCount) index = 0;
  return kCardLabels[index];
}

void formatMetricValue(MetricId id, float value, char* out, size_t outSize) {
  if (out == nullptr || outSize == 0) return;
  const MetricInfo& info = metricInfo(id);

  if (info.decimals == 0) {
    writeInteger(static_cast<int32_t>(lroundf(value)), info.thousands, out, outSize);
    return;
  }

  snprintf(out, outSize, "%.*f", static_cast<int>(info.decimals), static_cast<double>(value));
}

const char* layoutName(LayoutId id) {
  return kLayoutNames[clampIndex(static_cast<int>(id), 4)];
}

const char* layoutDescription(LayoutId id) {
  return kLayoutDescriptions[clampIndex(static_cast<int>(id), 4)];
}

const char* gaugeStyleName(GaugeStyle id) {
  return kGaugeStyleNames[clampIndex(static_cast<int>(id), 3)];
}

const char* indicatorStyleName(IndicatorStyle id) {
  return kIndicatorStyleNames[clampIndex(static_cast<int>(id), 2)];
}

const char* accentName(AccentId id) {
  return kAccentNames[clampIndex(static_cast<int>(id), 4)];
}

const char* scenarioName(ScenarioId id) {
  return kScenarioNames[clampIndex(static_cast<int>(id), 5)];
}

const char* scenarioDescription(ScenarioId id) {
  return kScenarioDescriptions[clampIndex(static_cast<int>(id), 5)];
}

const char* updateRateName(UpdateRate id) {
  static const char* const names[] = {"SLOW", "NORMAL", "FAST"};
  return names[clampIndex(static_cast<int>(id), 3)];
}

const char* brightnessName(BrightnessLevel id) {
  static const char* const names[] = {"LOW", "MEDIUM", "HIGH"};
  return names[clampIndex(static_cast<int>(id), 3)];
}

uint16_t updateRateMs(UpdateRate rate) {
  switch (rate) {
    case UpdateRate::Slow:
      return 400;
    case UpdateRate::Fast:
      return 60;
    case UpdateRate::Normal:
    default:
      return 150;
  }
}

uint8_t brightnessPercent(BrightnessLevel level) {
  switch (level) {
    case BrightnessLevel::Low:
      return 35;
    case BrightnessLevel::High:
      return 100;
    case BrightnessLevel::Medium:
    default:
      return 70;
  }
}

AppSettings defaultSettings() {
  AppSettings settings{};
  settings.layout = LayoutId::Classic;
  settings.primaryMetric = MetricId::Rpm;
  settings.secondaryMetrics[0] = MetricId::CoolantTemp;
  settings.secondaryMetrics[1] = MetricId::BatteryVoltage;
  settings.secondaryMetrics[2] = MetricId::Speed;
  settings.primaryStyle = GaugeStyle::Arc;
  settings.indicatorStyle = IndicatorStyle::Bar;
  settings.accent = AccentId::Ice;
  settings.scenario = ScenarioId::Normal;
  settings.updateRate = UpdateRate::Normal;
  settings.brightness = BrightnessLevel::High;
  settings.showTicks = true;
  settings.showIndicators = true;
  return settings;
}

MetricId metricAtSlot(const AppSettings& settings, uint8_t slot) {
  if (slot == 0) return settings.primaryMetric;
  if (slot <= SecondarySlotCount) return settings.secondaryMetrics[slot - 1];
  return MetricId::Rpm;
}

void setMetricAtSlot(AppSettings& settings, uint8_t slot, MetricId metric) {
  if (slot == 0) {
    settings.primaryMetric = metric;
  } else if (slot <= SecondarySlotCount) {
    settings.secondaryMetrics[slot - 1] = metric;
  }
}

bool assignMetricUnique(AppSettings& settings, uint8_t slot, MetricId metric) {
  if (slot > SecondarySlotCount || static_cast<uint8_t>(metric) >= MetricCount) return false;

  int existingSlot = -1;
  if (settings.primaryMetric == metric) {
    existingSlot = 0;
  } else {
    for (uint8_t i = 0; i < SecondarySlotCount; ++i) {
      if (settings.secondaryMetrics[i] == metric) {
        existingSlot = static_cast<int>(i) + 1;
        break;
      }
    }
  }

  if (existingSlot == static_cast<int>(slot)) return true;

  MetricId displaced = metricAtSlot(settings, slot);
  setMetricAtSlot(settings, slot, metric);
  if (existingSlot >= 0) {
    setMetricAtSlot(settings, static_cast<uint8_t>(existingSlot), displaced);
  }
  return true;
}

bool sanitizeSettings(AppSettings& settings) {
  const AppSettings before = settings;

  settings.layout = static_cast<LayoutId>(
      clampIndex(static_cast<int>(settings.layout), static_cast<uint8_t>(LayoutId::Count)));
  settings.primaryStyle = static_cast<GaugeStyle>(
      clampIndex(static_cast<int>(settings.primaryStyle), static_cast<uint8_t>(GaugeStyle::Count)));
  settings.indicatorStyle = static_cast<IndicatorStyle>(clampIndex(
      static_cast<int>(settings.indicatorStyle), static_cast<uint8_t>(IndicatorStyle::Count)));
  settings.accent = static_cast<AccentId>(
      clampIndex(static_cast<int>(settings.accent), static_cast<uint8_t>(AccentId::Count)));
  settings.scenario = static_cast<ScenarioId>(
      clampIndex(static_cast<int>(settings.scenario), static_cast<uint8_t>(ScenarioId::Count)));
  settings.updateRate = static_cast<UpdateRate>(
      clampIndex(static_cast<int>(settings.updateRate), static_cast<uint8_t>(UpdateRate::Count)));
  settings.brightness = static_cast<BrightnessLevel>(clampIndex(
      static_cast<int>(settings.brightness), static_cast<uint8_t>(BrightnessLevel::Count)));

  if (static_cast<uint8_t>(settings.primaryMetric) >= MetricCount) {
    settings.primaryMetric = MetricId::Rpm;
  }

  const AppSettings defaults = defaultSettings();
  bool used[MetricCount] = {};
  used[static_cast<uint8_t>(settings.primaryMetric)] = true;

  for (uint8_t i = 0; i < SecondarySlotCount; ++i) {
    MetricId candidate = settings.secondaryMetrics[i];
    bool valid = static_cast<uint8_t>(candidate) < MetricCount &&
                 !used[static_cast<uint8_t>(candidate)];
    if (!valid) {
      candidate = defaults.secondaryMetrics[i];
      if (used[static_cast<uint8_t>(candidate)]) {
        candidate = MetricId::Rpm;
        for (uint8_t m = 0; m < MetricCount; ++m) {
          if (!used[m]) {
            candidate = static_cast<MetricId>(m);
            break;
          }
        }
      }
    }
    settings.secondaryMetrics[i] = candidate;
    used[static_cast<uint8_t>(candidate)] = true;
  }

  return memcmp(&before, &settings, sizeof(AppSettings)) != 0;
}
