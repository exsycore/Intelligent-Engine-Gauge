#pragma once

#include "ui/Screen.h"

enum class ScreenId : uint8_t {
  Dashboard = 0,
  SettingsMenu,
  LayoutSelect,
  PrimaryGauge,
  SecondaryGauges,
  MetricPicker,
  Appearance,
  Display,
  MockTelemetry,
  ResetConfirm,
  Count
};

class ScreenManager {
 public:
  static constexpr uint8_t kMaxStack = 8;

  void add(ScreenId id, Screen* screen);
  void start(ScreenId id);
  void show(ScreenId id);
  void back();
  void redraw();

  Screen& current();
  ScreenId currentId() const { return currentId_; }

 private:
  Screen* screens_[static_cast<uint8_t>(ScreenId::Count)] = {};
  ScreenId stack_[kMaxStack] = {};
  uint8_t stackDepth_ = 0;
  ScreenId currentId_ = ScreenId::Dashboard;
  Screen* current_ = nullptr;
};
