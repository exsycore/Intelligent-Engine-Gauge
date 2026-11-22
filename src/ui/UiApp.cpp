#include "ui/UiApp.h"

#include <Arduino.h>

#include "ui/Theme.h"

namespace {
constexpr uint8_t kBacklightPin = 21;
}

UiApp::UiApp()
    : tft_(),
      touch_(),
      mock_(),
      settings_(),
      nav_(),
      dashboard_(tft_, settings_, mock_, nav_),
      settingsMenu_(tft_, settings_, nav_),
      layoutScreen_(tft_, settings_, nav_),
      metricPicker_(tft_, settings_, nav_),
      primaryGaugeScreen_(tft_, settings_, nav_, metricPicker_, true),
      secondaryGaugeScreen_(tft_, settings_, nav_, metricPicker_, false),
      appearanceScreen_(tft_, settings_, nav_),
      displayScreen_(tft_, settings_, nav_),
      mockScreen_(tft_, settings_, nav_),
      resetScreen_(tft_, settings_, nav_) {}

void UiApp::begin() {
  Serial.begin(115200);
  delay(120);

  tft_.init();
  Theme::init(tft_);

  settings_.begin();
  Theme::setActiveAccent(settings_.settings().accent);
  applyBacklight();

  mock_.setScenario(settings_.settings().scenario);
  mock_.setUpdateRate(settings_.settings().updateRate);
  mock_.begin();
  touch_.begin();

  nav_.add(ScreenId::Dashboard, &dashboard_);
  nav_.add(ScreenId::SettingsMenu, &settingsMenu_);
  nav_.add(ScreenId::LayoutSelect, &layoutScreen_);
  nav_.add(ScreenId::PrimaryGauge, &primaryGaugeScreen_);
  nav_.add(ScreenId::SecondaryGauges, &secondaryGaugeScreen_);
  nav_.add(ScreenId::MetricPicker, &metricPicker_);
  nav_.add(ScreenId::Appearance, &appearanceScreen_);
  nav_.add(ScreenId::Display, &displayScreen_);
  nav_.add(ScreenId::MockTelemetry, &mockScreen_);
  nav_.add(ScreenId::ResetConfirm, &resetScreen_);

  nav_.start(ScreenId::Dashboard);

  Serial.printf("[ui] ready: %dx%d landscape, demo=%s\n", ScreenDim::Width, ScreenDim::Height,
                mock_.isSimulated() ? "yes" : "no");
}

void UiApp::applyBacklight() {
  uint8_t percent = brightnessPercent(settings_.settings().brightness);
  if (!backlightReady_) {
    backlightReady_ = ledcAttach(kBacklightPin, 5000, 8);
    if (!backlightReady_) Serial.println("[ui] backlight PWM unavailable");
  }
  if (backlightReady_) {
    uint32_t duty = static_cast<uint32_t>(percent) * 255 / 100;
    ledcWrite(kBacklightPin, duty);
  }
  appliedBacklight_ = percent;
}

void UiApp::syncServices() {
  const AppSettings& settings = settings_.settings();
  Theme::setActiveAccent(settings.accent);

  if (mock_.scenario() != settings.scenario) mock_.setScenario(settings.scenario);
  if (mock_.updateRate() != settings.updateRate) mock_.setUpdateRate(settings.updateRate);
  if (appliedBacklight_ != brightnessPercent(settings.brightness)) applyBacklight();
}

void UiApp::loop() {
  mock_.poll(millis());
  syncServices();

  int16_t x = 0;
  int16_t y = 0;
  bool down = touch_.read(x, y);
  if (down && !touchWasDown_) {
    nav_.current().handleTouch(x, y);
  }
  touchWasDown_ = down;

  nav_.current().update();
  delay(1);
}
