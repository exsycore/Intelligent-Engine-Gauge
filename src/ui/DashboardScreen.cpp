#include "ui/DashboardScreen.h"

#include <math.h>
#include <stdio.h>
#include <string.h>

#include "ui/Theme.h"

namespace {

int16_t columnCenter(uint8_t index) {
  static const int16_t centers[SecondarySlotCount] = {55, 160, 265};
  return centers[index < SecondarySlotCount ? index : 0];
}

}  // namespace

DashboardScreen::DashboardScreen(TFT_eSPI& tft, SettingsManager& settings,
                                 ITelemetryProvider& telemetry, ScreenManager& nav)
    : tft_(tft), settings_(settings), telemetry_(telemetry), nav_(nav) {}

DashboardScreen::PrimarySpec DashboardScreen::buildPrimarySpec(LayoutId layout) const {
  PrimarySpec spec;

  // Every geometry below keeps text inside its container and keeps the value
  // area (the only opaque rectangle the gauge erases) clear of the arc ring.
  switch (layout) {
    case LayoutId::Sport:
      // Wide semicircle: metric and unit share the header row above the arc.
      spec.card = UiRect{6, 28, 308, 140};
      spec.arc = ArcSpec{160, 164, 112, 102, 90, 180, 6};
      spec.valueArea = UiRect{98, 89, 124, 56};
      spec.labelX = 16;
      spec.labelY = 40;
      spec.labelDatum = ML_DATUM;
      spec.labelMaxWidth = 170;
      spec.unitX = 304;
      spec.unitY = 40;
      spec.unitDatum = MR_DATUM;
      spec.bar = UiRect{64, 150, 192, 7};
      break;

    case LayoutId::Minimal:
      spec.card = UiRect{0, 30, 320, 96};
      spec.cardVisible = false;
      spec.valueArea = UiRect{20, 52, 280, 60};
      spec.labelX = 10;
      spec.labelY = 42;
      spec.labelDatum = ML_DATUM;
      spec.labelMaxWidth = 240;
      spec.unitX = 310;
      spec.unitY = 42;
      spec.unitDatum = MR_DATUM;
      spec.bar = UiRect{10, 120, 300, 3};
      break;

    case LayoutId::Technical:
      spec.card = UiRect{6, 30, 178, 206};
      spec.arc = ArcSpec{95, 140, 68, 60, 45, 270, 8};
      spec.valueArea = UiRect{47, 120, 96, 40};
      spec.labelX = 95;
      spec.labelY = 42;
      spec.labelMaxWidth = 150;
      spec.unitX = 95;
      spec.unitY = 178;
      spec.bar = UiRect{24, 210, 142, 8};
      spec.segmentedBar = true;
      spec.showScaleLabels = true;
      break;

    case LayoutId::Classic:
    default:
      spec.card = UiRect{6, 30, 178, 206};
      spec.arc = ArcSpec{95, 140, 68, 60, 45, 270, 8};
      spec.valueArea = UiRect{47, 120, 96, 40};
      spec.labelX = 95;
      spec.labelY = 48;
      spec.labelMaxWidth = 150;
      spec.unitX = 95;
      spec.unitY = 178;
      spec.bar = UiRect{24, 206, 142, 8};
      break;
  }

  if (layout == LayoutId::Minimal) {
    spec.useArc = false;
    spec.useBar = true;
  } else {
    GaugeStyle style = settings_.settings().primaryStyle;
    spec.useArc = style == GaugeStyle::Arc;
    spec.useBar = style == GaugeStyle::Bar;
  }
  return spec;
}

UiRect DashboardScreen::secondaryCard(int index) const {
  if (layout_ == LayoutId::Sport) {
    static const int16_t xs[SecondarySlotCount] = {6, 111, 216};
    int i = index < SecondarySlotCount ? index : 0;
    return UiRect{xs[i], 172, 98, 64};
  }
  static const int16_t ys[SecondarySlotCount] = {30, 100, 170};
  int i = index < SecondarySlotCount ? index : 0;
  return UiRect{190, ys[i], 124, 64};
}

float DashboardScreen::progressFor(MetricId metric, float value) const {
  const MetricInfo& info = metricInfo(metric);
  float span = info.maxValue - info.minValue;
  if (span <= 0.0f) return 0.0f;
  float progress = (value - info.minValue) / span;
  if (progress < 0.0f) return 0.0f;
  if (progress > 1.0f) return 1.0f;
  return progress;
}

uint16_t DashboardScreen::primaryColor() const {
  return primaryWarning_ ? Theme::Danger : Theme::activeAccent();
}

uint16_t DashboardScreen::valueBackground() const {
  return primary_.cardVisible ? Theme::Panel : Theme::Bg;
}

void DashboardScreen::enter() {
  Theme::setActiveAccent(settings_.settings().accent);
  layout_ = settings_.settings().layout;
  primary_ = buildPrimarySpec(layout_);
  primaryWarning_ = false;
  primaryProgress_ = 0.0f;
  primaryValue_[0] = '\0';
  for (uint8_t i = 0; i < SecondarySlotCount; ++i) slots_[i] = SlotState{};
  lastWarningText_ = nullptr;
  lastVersion_ = 0;
  drawFull();
}

void DashboardScreen::drawStatus() {
  const char* warning = telemetryWarningLabel(telemetry_.data());
  Theme::drawStatusBar(tft_, telemetry_.sourceLabel(), telemetry_.isSimulated(), warning);
  lastWarningText_ = warning;
}

void DashboardScreen::drawScaleLabels() {
  if (!primary_.useArc) return;

  const TelemetryData& data = telemetry_.data();
  MetricId metric = settings_.settings().primaryMetric;
  const MetricInfo& info = metricInfo(metric);
  (void)data;

  tft_.setFreeFont(FONT_LABEL);
  tft_.setTextColor(Theme::TextFaint, valueBackground());

  int16_t radius = static_cast<int16_t>(primary_.arc.r + primary_.arc.tickOuter + 6);
  for (uint8_t i = 0; i < 5; ++i) {
    float fraction = static_cast<float>(i) / 4.0f;
    float angle = static_cast<float>(primary_.arc.startAngle) + primary_.arc.sweep * fraction;
    float rad = angle * 0.01745329252f;
    int16_t x = static_cast<int16_t>(primary_.arc.cx - sinf(rad) * radius);
    int16_t y = static_cast<int16_t>(primary_.arc.cy + cosf(rad) * radius);

    char label[10];
    float value = info.minValue + (info.maxValue - info.minValue) * fraction;
    if (info.maxValue >= 1000.0f) {
      snprintf(label, sizeof(label), "%d", static_cast<int>(lroundf(value / 1000.0f)));
    } else {
      formatMetricValue(metric, value, label, sizeof(label));
    }

    // Anchor labels at the edge nearest the gauge centre so wide values such
    // as "108" or "11.5" cannot spill outside the primary card.
    uint8_t datum = MC_DATUM;
    if (x < primary_.arc.cx - 4) datum = ML_DATUM;
    else if (x > primary_.arc.cx + 4) datum = MR_DATUM;
    tft_.setTextDatum(datum);
    tft_.drawString(label, x, y);
  }
}

void DashboardScreen::drawPrimaryValueText() {
  uint16_t color = primaryWarning_ ? Theme::Danger : Theme::Text;
  // Only the value area is erased and it sits inside the arc's inner radius, so
  // the gauge ring is never painted over by an opaque rectangle.
  Theme::drawValueText(tft_, primary_.valueArea, primaryValue_, Theme::valueFonts(),
                       Theme::ValueFontCount, color, valueBackground());
}

void DashboardScreen::drawPrimaryStatic() {
  const AppSettings& settings = settings_.settings();
  MetricId metric = settings.primaryMetric;
  float value = telemetryValue(telemetry_.data(), metric);
  primaryWarning_ = metricInWarning(metric, value);
  primaryProgress_ = progressFor(metric, value);
  formatMetricValue(metric, value, primaryValue_, sizeof(primaryValue_));
  uint16_t accentColor = Theme::activeAccent();
  uint16_t background = valueBackground();

  if (primary_.cardVisible) {
    tft_.fillRoundRect(primary_.card.x, primary_.card.y, primary_.card.w, primary_.card.h, 10,
                       Theme::Panel);
    tft_.drawRoundRect(primary_.card.x, primary_.card.y, primary_.card.w, primary_.card.h, 10,
                       primaryWarning_ ? Theme::dim(Theme::Danger, 45) : Theme::Stroke);
  }

  if (primary_.useArc) {
    GaugeRenderer::drawArcTrack(tft_, primary_.arc, Theme::Track, background);
    if (settings.showTicks) {
      uint8_t minor = layout_ == LayoutId::Technical ? 4 : 2;
      GaugeRenderer::drawArcTicks(tft_, primary_.arc, 9, minor,
                                  Theme::dim(accentColor, 55), Theme::TextFaint);
    }
    if (primary_.showScaleLabels) drawScaleLabels();
    GaugeRenderer::drawArcProgress(tft_, primary_.arc, primaryProgress_, primaryColor(),
                                   background);
  }

  if (primary_.useBar) {
    GaugeRenderer::drawBar(tft_, primary_.bar, primaryProgress_, primaryColor(), Theme::Track,
                           primary_.segmentedBar);
  }

  Theme::drawTextElided(tft_, metricShortName(metric), primary_.labelX, primary_.labelY,
                        FONT_LABEL_BOLD, primaryWarning_ ? Theme::Warn : Theme::TextDim,
                        background, primary_.labelDatum, primary_.labelMaxWidth);

  tft_.setTextColor(Theme::TextDim, background);
  tft_.setTextDatum(primary_.unitDatum);
  tft_.drawString(metricUnit(metric), primary_.unitX, primary_.unitY);

  drawPrimaryValueText();
}

void DashboardScreen::drawMinimalSecondary(int index, bool full) {
  const AppSettings& settings = settings_.settings();
  MetricId metric = settings.secondaryMetrics[index];
  float value = telemetryValue(telemetry_.data(), metric);
  bool warning = metricInWarning(metric, value);
  float progress = progressFor(metric, value);
  int16_t cx = columnCenter(index);

  char buffer[16];
  formatMetricValue(metric, value, buffer, sizeof(buffer));

  if (full) {
    tft_.fillRect(cx - 49, 132, 98, 14, Theme::Bg);
    const char* label = Theme::textWidth(tft_, metricShortName(metric), FONT_LABEL_BOLD) <= 94
                            ? metricShortName(metric)
                            : metricCardLabel(metric);
    Theme::drawTextElided(tft_, label, cx, 140, FONT_LABEL_BOLD,
                          warning ? Theme::Warn : Theme::TextDim, Theme::Bg, MC_DATUM, 94);

    tft_.fillRect(cx - 49, 196, 98, 14, Theme::Bg);
    tft_.setFreeFont(FONT_LABEL);
    tft_.setTextColor(Theme::TextDim, Theme::Bg);
    tft_.setTextDatum(MC_DATUM);
    tft_.drawString(metricUnit(metric), cx, 202);
  }

  UiRect valueArea{static_cast<int16_t>(cx - 49), 152, 98, 40};
  Theme::drawValueText(tft_, valueArea, buffer, Theme::compactValueFonts(),
                       Theme::CompactValueFontCount, warning ? Theme::Danger : Theme::Text,
                       Theme::Bg);

  UiRect indicator{static_cast<int16_t>(cx - 35), 216, 70, 4};
  tft_.fillRect(indicator.x, indicator.y - 2, indicator.w, indicator.h + 4, Theme::Bg);
  if (settings.showIndicators) {
    if (settings.indicatorStyle == IndicatorStyle::Dots) {
      int16_t step = indicator.w / 5;
      uint8_t lit = static_cast<uint8_t>(ceilf(progress * 5.0f));
      for (uint8_t i = 0; i < 5; ++i) {
        int16_t x = indicator.x + step * i + step / 2;
        tft_.fillCircle(x, indicator.y + 2, 2,
                        i < lit ? (warning ? Theme::Danger : Theme::activeAccent())
                                : Theme::Track);
      }
    } else {
      tft_.fillRoundRect(indicator.x, indicator.y, indicator.w, indicator.h, 2, Theme::Track);
      int16_t fillW = static_cast<int16_t>(lroundf(indicator.w * progress));
      if (fillW > 1) {
        tft_.fillRoundRect(indicator.x, indicator.y, fillW, indicator.h, 2,
                           warning ? Theme::Danger : Theme::activeAccent());
      }
    }
  }

  slots_[index].warning = warning;
  slots_[index].progress = progress;
  snprintf(slots_[index].value, sizeof(slots_[index].value), "%s", buffer);
  slots_[index].valid = true;
}

void DashboardScreen::drawFull() {
  Theme::clear(tft_);
  drawStatus();
  drawPrimaryStatic();

  if (layout_ == LayoutId::Minimal) {
    tft_.drawFastVLine(108, 132, 102, Theme::Stroke);
    tft_.drawFastVLine(212, 132, 102, Theme::Stroke);
    for (uint8_t i = 0; i < SecondarySlotCount; ++i) {
      drawMinimalSecondary(i, true);
    }
  } else {
    for (uint8_t i = 0; i < SecondarySlotCount; ++i) {
      MetricId metric = settings_.settings().secondaryMetrics[i];
      float value = telemetryValue(telemetry_.data(), metric);
      float progress = progressFor(metric, value);
      bool warning = metricInWarning(metric, value);
      char buffer[16];
      formatMetricValue(metric, value, buffer, sizeof(buffer));

      GaugeRenderer::drawSecondaryCard(tft_, secondaryCard(i), metric, buffer, progress, warning,
                                       settings_.settings().indicatorStyle,
                                       settings_.settings().showIndicators,
                                       Theme::activeAccent());
      snprintf(slots_[i].value, sizeof(slots_[i].value), "%s", buffer);
      slots_[i].warning = warning;
      slots_[i].progress = progress;
      slots_[i].valid = true;
    }
  }

  lastVersion_ = telemetry_.version();
}

void DashboardScreen::update() {
  uint32_t version = telemetry_.version();
  if (version == lastVersion_) return;
  lastVersion_ = version;

  const TelemetryData& data = telemetry_.data();
  const AppSettings& settings = settings_.settings();

  const char* warningText = telemetryWarningLabel(data);
  bool warningChanged = (warningText == nullptr) != (lastWarningText_ == nullptr);
  if (!warningChanged && warningText != nullptr && lastWarningText_ != nullptr) {
    warningChanged = strcmp(warningText, lastWarningText_) != 0;
  }
  if (warningChanged) drawStatus();

  // Primary gauge: dirty-region arc/value updates unless the warning state
  // changes, which requires a color redraw of the whole gauge.
  MetricId metric = settings.primaryMetric;
  float value = telemetryValue(data, metric);
  bool warning = metricInWarning(metric, value);
  float progress = progressFor(metric, value);
  char buffer[16];
  formatMetricValue(metric, value, buffer, sizeof(buffer));

  if (warning != primaryWarning_) {
    drawPrimaryStatic();
  } else {
    if (primary_.useArc) {
      GaugeRenderer::updateArcProgress(tft_, primary_.arc, primaryProgress_, progress,
                                       primaryColor(), valueBackground());
    }
    if (primary_.useBar) {
      GaugeRenderer::updateBar(tft_, primary_.bar, progress, primaryColor(), Theme::Track,
                               primary_.segmentedBar);
    }
    if (strcmp(buffer, primaryValue_) != 0) {
      snprintf(primaryValue_, sizeof(primaryValue_), "%s", buffer);
      drawPrimaryValueText();
    }
    primaryProgress_ = progress;
  }

  // Secondary gauges.
  for (uint8_t i = 0; i < SecondarySlotCount; ++i) {
    MetricId slotMetric = settings.secondaryMetrics[i];
    float slotValue = telemetryValue(data, slotMetric);
    bool slotWarning = metricInWarning(slotMetric, slotValue);
    float slotProgress = progressFor(slotMetric, slotValue);
    char slotBuffer[16];
    formatMetricValue(slotMetric, slotValue, slotBuffer, sizeof(slotBuffer));

    bool valueChanged = strcmp(slotBuffer, slots_[i].value) != 0;
    bool progressChanged = fabsf(slotProgress - slots_[i].progress) > 0.004f;

    if (layout_ == LayoutId::Minimal) {
      if (!slots_[i].valid || slotWarning != slots_[i].warning) {
        drawMinimalSecondary(i, true);
      } else if (valueChanged || progressChanged) {
        drawMinimalSecondary(i, false);
      }
      continue;
    }

    if (!slots_[i].valid || slotWarning != slots_[i].warning) {
      GaugeRenderer::drawSecondaryCard(tft_, secondaryCard(i), slotMetric, slotBuffer,
                                       slotProgress, slotWarning, settings.indicatorStyle,
                                       settings.showIndicators, Theme::activeAccent());
    } else if (valueChanged || progressChanged) {
      GaugeRenderer::updateSecondaryCard(tft_, secondaryCard(i), slotMetric, slotBuffer,
                                         slotProgress, slotWarning, settings.indicatorStyle,
                                         settings.showIndicators, Theme::activeAccent());
    }

    snprintf(slots_[i].value, sizeof(slots_[i].value), "%s", slotBuffer);
    slots_[i].warning = slotWarning;
    slots_[i].progress = slotProgress;
    slots_[i].valid = true;
  }
}

void DashboardScreen::handleTouch(int16_t x, int16_t y) {
  if (Theme::settingsButtonRect().contains(x, y)) {
    nav_.show(ScreenId::SettingsMenu);
  }
}
