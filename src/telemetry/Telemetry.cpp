#include "telemetry/Telemetry.h"

float telemetryValue(const TelemetryData& data, MetricId metric) {
  switch (metric) {
    case MetricId::Rpm:
      return data.rpm;
    case MetricId::CoolantTemp:
      return data.coolantC;
    case MetricId::BatteryVoltage:
      return data.batteryV;
    case MetricId::Speed:
      return data.speedKph;
    case MetricId::Throttle:
      return data.throttlePct;
    case MetricId::FuelLevel:
      return data.fuelPct;
    case MetricId::IntakeTemp:
      return data.intakeC;
    default:
      return 0.0f;
  }
}

bool metricInWarning(MetricId metric, float value) {
  switch (metric) {
    case MetricId::Rpm:
      return value >= 6800.0f;
    case MetricId::CoolantTemp:
      return value >= 105.0f;
    case MetricId::BatteryVoltage:
      return value <= 11.8f;
    case MetricId::Speed:
      return value >= 180.0f;
    case MetricId::FuelLevel:
      return value <= 10.0f;
    case MetricId::IntakeTemp:
      return value >= 70.0f;
    case MetricId::Throttle:
    default:
      return false;
  }
}

bool telemetryHasWarning(const TelemetryData& data) {
  return metricInWarning(MetricId::CoolantTemp, data.coolantC) ||
         metricInWarning(MetricId::BatteryVoltage, data.batteryV) ||
         metricInWarning(MetricId::Rpm, data.rpm) ||
         metricInWarning(MetricId::FuelLevel, data.fuelPct) ||
         metricInWarning(MetricId::IntakeTemp, data.intakeC) ||
         metricInWarning(MetricId::Speed, data.speedKph);
}

const char* telemetryWarningLabel(const TelemetryData& data) {
  if (metricInWarning(MetricId::CoolantTemp, data.coolantC)) return "TEMP HIGH";
  if (metricInWarning(MetricId::BatteryVoltage, data.batteryV)) return "LOW VOLTAGE";
  if (metricInWarning(MetricId::Rpm, data.rpm)) return "REDLINE";
  if (metricInWarning(MetricId::FuelLevel, data.fuelPct)) return "LOW FUEL";
  if (metricInWarning(MetricId::IntakeTemp, data.intakeC)) return "INTAKE HOT";
  if (metricInWarning(MetricId::Speed, data.speedKph)) return "SPEED HIGH";
  return nullptr;
}
