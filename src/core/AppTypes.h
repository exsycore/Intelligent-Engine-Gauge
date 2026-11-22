#pragma once

#include <Arduino.h>
#include <stddef.h>
#include <stdint.h>

// ---------------------------------------------------------------------------
// UI geometry helpers
// ---------------------------------------------------------------------------

struct UiRect {
  int16_t x = 0;
  int16_t y = 0;
  int16_t w = 0;
  int16_t h = 0;

  bool contains(int16_t px, int16_t py) const {
    return px >= x && px < (x + w) && py >= y && py < (y + h);
  }
};

namespace ScreenDim {
constexpr int16_t Width = 320;
constexpr int16_t Height = 240;
constexpr int16_t StatusBarHeight = 28;
constexpr int16_t ContentTop = 34;
constexpr int16_t ContentBottom = 236;
}  // namespace ScreenDim

// ---------------------------------------------------------------------------
// Telemetry metric model
// ---------------------------------------------------------------------------

enum class MetricId : uint8_t {
  Rpm = 0,
  CoolantTemp,
  BatteryVoltage,
  Speed,
  Throttle,
  FuelLevel,
  IntakeTemp,
  Count
};

constexpr uint8_t MetricCount = static_cast<uint8_t>(MetricId::Count);
constexpr uint8_t SecondarySlotCount = 3;

struct MetricInfo {
  const char* name;       // Long name used in pickers
  const char* shortName;  // Compact name used on the dashboard
  const char* unit;
  float minValue;
  float maxValue;
  uint8_t decimals;
  bool thousands;
};

const MetricInfo& metricInfo(MetricId id);
const char* metricShortName(MetricId id);
// Shortest label that still identifies the metric on a small dashboard card.
const char* metricCardLabel(MetricId id);
const char* metricUnit(MetricId id);

// Formats a value without its unit (e.g. "8,500", "13.8").
void formatMetricValue(MetricId id, float value, char* out, size_t outSize);

// ---------------------------------------------------------------------------
// Dashboard configuration
// ---------------------------------------------------------------------------

enum class LayoutId : uint8_t { Classic = 0, Sport, Minimal, Technical, Count };
enum class GaugeStyle : uint8_t { Arc = 0, Bar, Numeric, Count };
enum class IndicatorStyle : uint8_t { Bar = 0, Dots, Count };
enum class AccentId : uint8_t { Ice = 0, Red, Amber, Green, Count };
enum class ScenarioId : uint8_t { Idle = 0, Normal, Acceleration, HighRpm, Warning, Count };
enum class UpdateRate : uint8_t { Slow = 0, Normal, Fast, Count };
enum class BrightnessLevel : uint8_t { Low = 0, Medium, High, Count };

const char* layoutName(LayoutId id);
const char* layoutDescription(LayoutId id);
const char* gaugeStyleName(GaugeStyle id);
const char* indicatorStyleName(IndicatorStyle id);
const char* accentName(AccentId id);
const char* scenarioName(ScenarioId id);
const char* scenarioDescription(ScenarioId id);
const char* updateRateName(UpdateRate id);
const char* brightnessName(BrightnessLevel id);

uint16_t updateRateMs(UpdateRate rate);
uint8_t brightnessPercent(BrightnessLevel level);

struct AppSettings {
  LayoutId layout;
  MetricId primaryMetric;
  MetricId secondaryMetrics[SecondarySlotCount];
  GaugeStyle primaryStyle;
  IndicatorStyle indicatorStyle;
  AccentId accent;
  ScenarioId scenario;
  UpdateRate updateRate;
  BrightnessLevel brightness;
  bool showTicks;
  bool showIndicators;
};

AppSettings defaultSettings();

// Returns true when the settings were changed to a safe, valid state.
bool sanitizeSettings(AppSettings& settings);

// Metric shown in a configuration slot: 0 = primary, 1..3 = secondary slots.
MetricId metricAtSlot(const AppSettings& settings, uint8_t slot);
void setMetricAtSlot(AppSettings& settings, uint8_t slot, MetricId metric);

// Swaps with any other slot that already uses the metric, keeping all four
// dashboard assignments unique. Returns false for an invalid slot/metric.
bool assignMetricUnique(AppSettings& settings, uint8_t slot, MetricId metric);
