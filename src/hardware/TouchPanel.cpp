#include "hardware/TouchPanel.h"

#include "core/AppTypes.h"

namespace {

const SPISettings kTouchSpiSettings(2000000, MSBFIRST, SPI_MODE0);
constexpr uint32_t kSampleIntervalMs = 12;
constexpr uint16_t kMinRaw = 80;
constexpr uint16_t kMaxRaw = 4020;

int16_t mapClamped(uint16_t raw, uint16_t rawMin, uint16_t rawMax, int16_t outMax) {
  int32_t value = map(static_cast<long>(raw), static_cast<long>(rawMin),
                      static_cast<long>(rawMax), 0, static_cast<long>(outMax));
  if (value < 0) value = 0;
  if (value > outMax) value = outMax;
  return static_cast<int16_t>(value);
}

}  // namespace

bool TouchPanel::begin() {
  pinMode(TouchPins::Cs, OUTPUT);
  digitalWrite(TouchPins::Cs, HIGH);
  pinMode(TouchPins::Irq, INPUT);

  bool ok = spi_.begin(TouchPins::Clk, TouchPins::Miso, TouchPins::Mosi, TouchPins::Cs);
  if (!ok) {
    Serial.println("[touch] SPI bus init failed");
  }
  return ok;
}

uint16_t TouchPanel::readChannel(uint8_t command) {
  spi_.beginTransaction(kTouchSpiSettings);
  digitalWrite(TouchPins::Cs, LOW);
  spi_.transfer(command);
  uint16_t value = spi_.transfer16(0x0000) >> 3;
  digitalWrite(TouchPins::Cs, HIGH);
  spi_.endTransaction();
  return value & 0x0FFF;
}

bool TouchPanel::read(int16_t& x, int16_t& y) {
  uint32_t now = millis();
  if (now - lastReadMs_ < kSampleIntervalMs) {
    if (pressed_) {
      x = lastX_;
      y = lastY_;
    }
    return pressed_;
  }
  lastReadMs_ = now;

  // Require two consecutive released reads so a single noisy IRQ sample cannot
  // split a held touch into two taps.
  if (digitalRead(TouchPins::Irq) != LOW) {
    pressSamples_ = 0;
    if (releaseSamples_ < 2) ++releaseSamples_;
    if (releaseSamples_ >= 2) pressed_ = false;
    if (pressed_) {
      x = lastX_;
      y = lastY_;
    }
    return pressed_;
  }

  uint32_t sumX = 0;
  uint32_t sumY = 0;
  uint8_t samples = 0;
  for (uint8_t i = 0; i < 3; ++i) {
    uint16_t rx = readChannel(0x90);  // X, 12-bit
    uint16_t ry = readChannel(0xD0);  // Y, 12-bit
    if (rx >= kMinRaw && rx <= kMaxRaw && ry >= kMinRaw && ry <= kMaxRaw) {
      sumX += rx;
      sumY += ry;
      ++samples;
    }
  }

  if (samples == 0) {
    pressSamples_ = 0;
    if (releaseSamples_ < 2) ++releaseSamples_;
    if (releaseSamples_ >= 2) pressed_ = false;
    if (pressed_) {
      x = lastX_;
      y = lastY_;
    }
    return pressed_;
  }

  releaseSamples_ = 0;
  rawX_ = static_cast<uint16_t>(sumX / samples);
  rawY_ = static_cast<uint16_t>(sumY / samples);

  uint16_t rx = rawX_;
  uint16_t ry = rawY_;
#if TOUCH_SWAP_XY
  uint16_t tmp = rx;
  rx = ry;
  ry = tmp;
#endif

  int16_t sx = mapClamped(rx, TOUCH_RAW_X_MIN, TOUCH_RAW_X_MAX, ScreenDim::Width - 1);
  int16_t sy = mapClamped(ry, TOUCH_RAW_Y_MIN, TOUCH_RAW_Y_MAX, ScreenDim::Height - 1);
#if TOUCH_INVERT_X
  sx = ScreenDim::Width - 1 - sx;
#endif
#if TOUCH_INVERT_Y
  sy = ScreenDim::Height - 1 - sy;
#endif

  lastX_ = sx;
  lastY_ = sy;

  // Two valid samples are needed before a press is reported, which filters
  // contact bounce while still keeping latency around 12-24 ms.
  if (pressSamples_ < 2) ++pressSamples_;
  pressed_ = pressSamples_ >= 2;
  if (!pressed_) return false;

  x = sx;
  y = sy;
  return true;
}
