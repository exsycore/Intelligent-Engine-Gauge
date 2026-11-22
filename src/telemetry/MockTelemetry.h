#pragma once

#include "telemetry/Telemetry.h"

// Deterministic, smoothly changing simulator used while no sensors or ECU are
// connected. It implements the same provider interface a real telemetry source
// will use later.
class MockTelemetryProvider : public ITelemetryProvider {
 public:
  void begin() override;
  void poll(uint32_t nowMs) override;

  const TelemetryData& data() const override { return data_; }
  uint32_t version() const override { return version_; }
  bool isSimulated() const override { return true; }
  const char* sourceLabel() const override { return "SIM"; }

  void setScenario(ScenarioId scenario);
  ScenarioId scenario() const { return scenario_; }

  void setUpdateRate(UpdateRate rate);
  UpdateRate updateRate() const { return updateRate_; }

 private:
  void seedState();
  void updateTargets(float dt);
  void stepValues(float dt);
  void applyScenarioTargets();

  TelemetryData data_;
  uint32_t version_ = 0;
  uint32_t lastPollMs_ = 0;
  uint32_t simulatedMs_ = 0;

  ScenarioId scenario_ = ScenarioId::Normal;
  UpdateRate updateRate_ = UpdateRate::Normal;
  uint16_t intervalMs_ = 150;

  float scenarioTime_ = 0.0f;
  float fuel_ = 72.0f;

  // Targets for the current simulation slice.
  float targetRpm_ = 850.0f;
  float targetCoolant_ = 85.0f;
  float targetBattery_ = 13.8f;
  float targetSpeed_ = 0.0f;
  float targetThrottle_ = 3.0f;
  float targetFuel_ = 72.0f;
  float targetIntake_ = 30.0f;
};
