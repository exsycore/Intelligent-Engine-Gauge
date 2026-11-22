#pragma once

#include <stdint.h>

#include "core/AppTypes.h"

// Data model shared by every telemetry source (mock, sensors, ECU, serial...).
struct TelemetryData {
  float rpm = 0.0f;
  float coolantC = 0.0f;
  float batteryV = 0.0f;
  float speedKph = 0.0f;
  float throttlePct = 0.0f;
  float fuelPct = 0.0f;
  float intakeC = 0.0f;
};

float telemetryValue(const TelemetryData& data, MetricId metric);

// Provider contract. The UI only ever reads this interface, never the mock
// implementation, so real telemetry can replace the simulator later.
class ITelemetryProvider {
 public:
  virtual ~ITelemetryProvider() = default;

  virtual void begin() = 0;
  virtual void poll(uint32_t nowMs) = 0;

  virtual const TelemetryData& data() const = 0;
  virtual uint32_t version() const = 0;

  virtual bool isSimulated() const = 0;
  virtual const char* sourceLabel() const = 0;
};

bool metricInWarning(MetricId metric, float value);
bool telemetryHasWarning(const TelemetryData& data);

// Short message for the dashboard warning strip (nullptr when all is normal).
const char* telemetryWarningLabel(const TelemetryData& data);
