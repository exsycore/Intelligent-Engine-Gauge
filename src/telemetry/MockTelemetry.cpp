#include "telemetry/MockTelemetry.h"

#include <math.h>

namespace {

float approach(float current, float target, float maxDelta) {
  if (current < target) {
    current += maxDelta;
    return current > target ? target : current;
  }
  current -= maxDelta;
  return current < target ? target : current;
}

float clampf(float value, float low, float high) {
  if (value < low) return low;
  if (value > high) return high;
  return value;
}

}  // namespace

void MockTelemetryProvider::begin() {
  seedState();
  lastPollMs_ = millis();
  simulatedMs_ = 0;
  scenarioTime_ = 0.0f;
  applyScenarioTargets();

  // Seed with values that match the selected scenario so restoring a saved
  // demo mode does not start with a misleading transient.
  data_.rpm = 850.0f;
  data_.coolantC = 85.0f;
  data_.batteryV = 13.8f;
  data_.speedKph = 65.0f;
  data_.throttlePct = 16.0f;
  data_.fuelPct = 72.0f;
  data_.intakeC = 32.0f;

  switch (scenario_) {
    case ScenarioId::Idle:
      data_.rpm = 820.0f;
      data_.coolantC = 87.0f;
      data_.batteryV = 14.1f;
      data_.speedKph = 0.0f;
      data_.throttlePct = 2.5f;
      data_.intakeC = 31.0f;
      break;
    case ScenarioId::Acceleration:
      data_.rpm = 3300.0f;
      data_.coolantC = 94.0f;
      data_.batteryV = 13.8f;
      data_.speedKph = 55.0f;
      data_.throttlePct = 78.0f;
      data_.intakeC = 42.0f;
      break;
    case ScenarioId::HighRpm:
      data_.rpm = 7200.0f;
      data_.coolantC = 103.0f;
      data_.batteryV = 13.6f;
      data_.speedKph = 140.0f;
      data_.throttlePct = 86.0f;
      data_.intakeC = 45.0f;
      break;
    case ScenarioId::Warning:
      data_.rpm = 6900.0f;
      data_.coolantC = 108.0f;
      data_.batteryV = 11.5f;
      data_.speedKph = 120.0f;
      data_.throttlePct = 45.0f;
      data_.intakeC = 58.0f;
      fuel_ = 8.0f;
      data_.fuelPct = fuel_;
      break;
    case ScenarioId::Normal:
    default:
      break;
  }
  version_ = 1;
}

void MockTelemetryProvider::setScenario(ScenarioId scenario) {
  if (scenario == scenario_) return;
  if (static_cast<uint8_t>(scenario) >= static_cast<uint8_t>(ScenarioId::Count)) return;
  scenario_ = scenario;
  scenarioTime_ = 0.0f;
  applyScenarioTargets();
}

void MockTelemetryProvider::setUpdateRate(UpdateRate rate) {
  updateRate_ = rate;
  intervalMs_ = updateRateMs(rate);
}

void MockTelemetryProvider::seedState() {
  // Fuel is persistent state inside the simulator; everything else is driven
  // toward per-scenario targets by stepValues().
  if (fuel_ <= 0.0f) fuel_ = 72.0f;
}

void MockTelemetryProvider::poll(uint32_t nowMs) {
  if (lastPollMs_ == 0) {
    lastPollMs_ = nowMs;
    return;
  }
  if (nowMs - lastPollMs_ < intervalMs_) return;

  uint32_t elapsed = nowMs - lastPollMs_;
  lastPollMs_ = nowMs;
  simulatedMs_ += elapsed;

  float dt = static_cast<float>(elapsed) / 1000.0f;
  if (dt > 0.5f) dt = 0.5f;

  scenarioTime_ += dt;
  updateTargets(dt);
  stepValues(dt);
  ++version_;
}

void MockTelemetryProvider::applyScenarioTargets() {
  const float wobble = sinf(scenarioTime_ * 1.7f);

  switch (scenario_) {
    case ScenarioId::Idle:
      targetRpm_ = 820.0f + wobble * 20.0f;
      targetCoolant_ = 88.0f;
      targetBattery_ = 14.2f;
      targetSpeed_ = 0.0f;
      targetThrottle_ = 2.5f;
      targetIntake_ = 31.0f;
      break;

    case ScenarioId::Acceleration: {
      const float cycle = fmodf(scenarioTime_, 16.0f);
      if (cycle < 5.0f) {
        float pull = cycle / 5.0f;
        targetSpeed_ = 55.0f + pull * 75.0f;
        targetThrottle_ = 78.0f + pull * 12.0f;
        targetRpm_ = 3300.0f + pull * 2900.0f + wobble * 40.0f;
        targetCoolant_ = 96.0f;
        targetBattery_ = 13.7f;
        targetIntake_ = 44.0f;
      } else if (cycle < 9.0f) {
        targetSpeed_ = 122.0f;
        targetThrottle_ = 24.0f;
        targetRpm_ = 3150.0f + wobble * 40.0f;
        targetCoolant_ = 95.0f;
        targetBattery_ = 13.9f;
        targetIntake_ = 39.0f;
      } else if (cycle < 13.0f) {
        targetSpeed_ = 55.0f;
        targetThrottle_ = 5.0f;
        targetRpm_ = 1750.0f + wobble * 30.0f;
        targetCoolant_ = 93.0f;
        targetBattery_ = 14.0f;
        targetIntake_ = 35.0f;
      } else {
        targetSpeed_ = 55.0f;
        targetThrottle_ = 15.0f;
        targetRpm_ = 2100.0f + wobble * 30.0f;
        targetCoolant_ = 93.0f;
        targetBattery_ = 14.0f;
        targetIntake_ = 35.0f;
      }
      break;
    }

    case ScenarioId::HighRpm:
      targetRpm_ = 7350.0f + wobble * 45.0f;
      targetCoolant_ = 106.0f;
      targetBattery_ = 13.5f;
      targetSpeed_ = 145.0f;
      targetThrottle_ = 88.0f;
      targetIntake_ = 47.0f;
      break;

    case ScenarioId::Warning:
      targetRpm_ = 4200.0f + wobble * 30.0f;
      targetCoolant_ = 112.0f;
      targetBattery_ = 11.3f;
      targetSpeed_ = 40.0f;
      targetThrottle_ = 30.0f;
      targetIntake_ = 58.0f;
      targetFuel_ = 8.0f;
      break;

    case ScenarioId::Normal:
    default:
      targetRpm_ = 2450.0f + wobble * 30.0f;
      targetCoolant_ = 90.0f;
      targetBattery_ = 13.9f;
      targetSpeed_ = 65.0f;
      targetThrottle_ = 18.0f;
      targetIntake_ = 34.0f;
      break;
  }

  if (scenario_ != ScenarioId::Warning) targetFuel_ = 66.0f;
}

void MockTelemetryProvider::updateTargets(float dt) {
  applyScenarioTargets();

  // Small correlated jitter keeps the readout alive like a real sensor.
  const float t = static_cast<float>(simulatedMs_) / 1000.0f;
  const float jitter = sinf(t * 6.4f) * 12.0f + sinf(t * 2.1f) * 7.0f;
  targetRpm_ = clampf(targetRpm_ + jitter, 750.0f, 7900.0f);

  if (scenario_ == ScenarioId::Normal) {
    targetSpeed_ += sinf(t * 0.35f) * 0.8f;
  }

  // Driving load warms the intake charge and pulls system voltage down a bit.
  if (data_.speedKph > 30.0f) {
    targetIntake_ += 1.5f;
    targetBattery_ -= 0.05f;
  }

  (void)dt;
}

void MockTelemetryProvider::stepValues(float dt) {
  data_.rpm = approach(data_.rpm, targetRpm_, 4200.0f * dt);

  float speedRate = 9.0f * dt;
  if (scenario_ == ScenarioId::Acceleration) speedRate = 27.0f * dt;
  data_.speedKph = approach(data_.speedKph, targetSpeed_, speedRate);

  data_.throttlePct = approach(data_.throttlePct, targetThrottle_, 120.0f * dt);

  float tempRate = (targetCoolant_ > data_.coolantC + 2.0f) ? 1.8f : 0.32f;
  if (scenario_ == ScenarioId::Warning && targetCoolant_ > data_.coolantC) tempRate = 3.0f;
  data_.coolantC = approach(data_.coolantC, targetCoolant_, tempRate * dt);

  float batteryRate = (targetBattery_ < data_.batteryV) ? 0.28f : 0.06f;
  if (scenario_ == ScenarioId::Warning && targetBattery_ < data_.batteryV) batteryRate = 0.45f;
  data_.batteryV = approach(data_.batteryV, targetBattery_, batteryRate * dt);

  data_.intakeC = approach(data_.intakeC, targetIntake_, 0.5f * dt);

  float fuelRate = 0.015f;
  if (targetFuel_ > fuel_) {
    fuelRate = 4.0f;  // demo refuel when leaving the warning scenario
  } else if (scenario_ == ScenarioId::Warning) {
    fuelRate = 15.0f;
  } else if (scenario_ == ScenarioId::Acceleration) {
    fuelRate = 0.05f;
  }
  fuel_ = approach(fuel_, targetFuel_, fuelRate * dt);
  fuel_ = clampf(fuel_, 0.0f, 100.0f);
  data_.fuelPct = fuel_;

  data_.rpm = clampf(data_.rpm, 0.0f, 8000.0f);
  data_.coolantC = clampf(data_.coolantC, -20.0f, 130.0f);
  data_.batteryV = clampf(data_.batteryV, 9.0f, 15.5f);
  data_.speedKph = clampf(data_.speedKph, 0.0f, 220.0f);
  data_.intakeC = clampf(data_.intakeC, -20.0f, 80.0f);
}
