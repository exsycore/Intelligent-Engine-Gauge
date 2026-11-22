#pragma once

#include <stdint.h>

// Every UI page implements this small contract. `enter()` draws a complete
// frame; the dashboard uses `update()` for dirty-region telemetry refreshes.
class Screen {
 public:
  virtual ~Screen() = default;

  virtual void enter() = 0;
  virtual void update() {}
  virtual void handleTouch(int16_t x, int16_t y) = 0;
};
