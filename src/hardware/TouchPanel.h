#pragma once

#include <Arduino.h>
#include <SPI.h>

// Minimal XPT2046 driver for the ESP32-2432S028. The touch controller lives on
// its own SPI bus, so it does not interfere with the TFT bus.
//
// The default calibration matches the common CYD panel in landscape rotation.
// If touches land offset on a different unit, adjust these four constants (or
// invert an axis) and rebuild.
#ifndef TOUCH_RAW_X_MIN
#define TOUCH_RAW_X_MIN 200
#endif
#ifndef TOUCH_RAW_X_MAX
#define TOUCH_RAW_X_MAX 3700
#endif
#ifndef TOUCH_RAW_Y_MIN
#define TOUCH_RAW_Y_MIN 240
#endif
#ifndef TOUCH_RAW_Y_MAX
#define TOUCH_RAW_Y_MAX 3800
#endif
#ifndef TOUCH_SWAP_XY
#define TOUCH_SWAP_XY 0
#endif
#ifndef TOUCH_INVERT_X
#define TOUCH_INVERT_X 0
#endif
#ifndef TOUCH_INVERT_Y
#define TOUCH_INVERT_Y 0
#endif

namespace TouchPins {
constexpr uint8_t Cs = 33;
constexpr uint8_t Irq = 36;
constexpr uint8_t Clk = 25;
constexpr uint8_t Mosi = 32;
constexpr uint8_t Miso = 39;
}  // namespace TouchPins

class TouchPanel {
 public:
  bool begin();

  // Returns true while a touch is held. Coordinates are screen space (0..319,
  // 0..239) and remain valid on the press transition used by the UI.
  bool read(int16_t& x, int16_t& y);

  uint16_t rawX() const { return rawX_; }
  uint16_t rawY() const { return rawY_; }

 private:
  uint16_t readChannel(uint8_t command);

  SPIClass spi_{VSPI};
  uint32_t lastReadMs_ = 0;
  bool pressed_ = false;
  int16_t lastX_ = 0;
  int16_t lastY_ = 0;
  uint16_t rawX_ = 0;
  uint16_t rawY_ = 0;
  uint8_t pressSamples_ = 0;
  uint8_t releaseSamples_ = 0;
};
