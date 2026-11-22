#include "ui/screens/GaugeConfigScreen.h"

#include <stdio.h>

#include "ui/Theme.h"

namespace {
UiRect slotRow(uint8_t slot) {
  return UiRect{6, static_cast<int16_t>(36 + (slot - 1) * 42), 308, 38};
}
UiRect styleButton(uint8_t index) {
  static const int16_t xs[3] = {6, 112, 218};
  return UiRect{xs[index < 3 ? index : 0], 118, 96, 38};
}
}  // namespace

GaugeConfigScreen::GaugeConfigScreen(TFT_eSPI& tft, SettingsManager& settings, ScreenManager& nav,
                                     MetricPickerScreen& picker, bool primaryMode)
    : tft_(tft),
      settings_(settings),
      nav_(nav),
      picker_(picker),
      primaryMode_(primaryMode) {}

void GaugeConfigScreen::drawFieldRow(const UiRect& rect, const char* caption,
                                     const char* value) {
  tft_.fillRoundRect(rect.x, rect.y, rect.w, rect.h, 6, Theme::Panel);
  tft_.drawRoundRect(rect.x, rect.y, rect.w, rect.h, 6, Theme::Stroke);

  tft_.setFreeFont(FONT_LABEL);
  tft_.setTextColor(Theme::TextFaint, Theme::Panel);
  tft_.setTextDatum(ML_DATUM);
  tft_.drawString(caption, rect.x + 12, rect.y + 10);

  tft_.setFreeFont(FONT_LABEL_BOLD);
  tft_.setTextColor(Theme::Text, Theme::Panel);
  tft_.setTextDatum(ML_DATUM);
  tft_.drawString(value, rect.x + 12, rect.y + 26);

  Theme::drawChevron(tft_, rect.x + rect.w - 14, rect.y + rect.h / 2, Theme::TextDim);
}

void GaugeConfigScreen::drawSegmentButton(const UiRect& rect, const char* label, bool selected) {
  uint16_t accentColor = Theme::activeAccent();
  tft_.fillRoundRect(rect.x, rect.y, rect.w, rect.h, 6,
                     selected ? Theme::PanelSelected : Theme::Panel);
  tft_.drawRoundRect(rect.x, rect.y, rect.w, rect.h, 6,
                     selected ? accentColor : Theme::Stroke);
  tft_.setFreeFont(FONT_LABEL_BOLD);
  tft_.setTextColor(selected ? accentColor : Theme::TextDim,
                    selected ? Theme::PanelSelected : Theme::Panel);
  tft_.setTextDatum(MC_DATUM);
  tft_.drawString(label, rect.x + rect.w / 2, rect.y + rect.h / 2);
}

void GaugeConfigScreen::enter() {
  Theme::setActiveAccent(settings_.settings().accent);
  Theme::clear(tft_);
  Theme::drawHeader(tft_, primaryMode_ ? "PRIMARY GAUGE" : "SECONDARY");

  const AppSettings& s = settings_.settings();
  if (primaryMode_) {
    UiRect metricRow{6, 36, 308, 54};
    drawFieldRow(metricRow, "METRIC", metricInfo(s.primaryMetric).name);

    Theme::drawSectionLabel(tft_, "GAUGE STYLE", 10, 106);
    for (uint8_t i = 0; i < 3; ++i) {
      GaugeStyle style = static_cast<GaugeStyle>(i);
      drawSegmentButton(styleButton(i), gaugeStyleName(style), s.primaryStyle == style);
    }

    tft_.setFreeFont(FONT_LABEL);
    tft_.setTextColor(Theme::TextDim, Theme::Bg);
    tft_.setTextDatum(ML_DATUM);
    tft_.drawString("Arc: circular sweep", 10, 176);
    tft_.setTextColor(Theme::TextFaint, Theme::Bg);
    tft_.drawString("Bar: strip - Numeric: value only", 10, 194);
    tft_.drawString("Minimal layout is numeric only", 10, 212);
  } else {
    for (uint8_t slot = 1; slot <= SecondarySlotCount; ++slot) {
      UiRect row = slotRow(slot);
      char caption[12];
      snprintf(caption, sizeof(caption), "SLOT %u", static_cast<unsigned>(slot));

      tft_.fillRoundRect(row.x, row.y, row.w, row.h, 6, Theme::Panel);
      tft_.drawRoundRect(row.x, row.y, row.w, row.h, 6, Theme::Stroke);

      tft_.setFreeFont(FONT_LABEL);
      tft_.setTextColor(Theme::TextFaint, Theme::Panel);
      tft_.setTextDatum(ML_DATUM);
      tft_.drawString(caption, row.x + 12, row.y + 10);

      tft_.setFreeFont(FONT_LABEL_BOLD);
      tft_.setTextColor(Theme::Text, Theme::Panel);
      tft_.setTextDatum(ML_DATUM);
      tft_.drawString(metricInfo(s.secondaryMetrics[slot - 1]).name, row.x + 12, row.y + 26);

      Theme::drawChevron(tft_, row.x + row.w - 14, row.y + row.h / 2, Theme::TextDim);
    }

    Theme::drawSectionLabel(tft_, "INDICATOR STYLE", 10, 170);
    drawSegmentButton(UiRect{6, 180, 150, 36}, "BAR", s.indicatorStyle == IndicatorStyle::Bar);
    drawSegmentButton(UiRect{164, 180, 150, 36}, "DOTS",
                      s.indicatorStyle == IndicatorStyle::Dots);

    tft_.setFreeFont(FONT_LABEL);
    tft_.setTextColor(Theme::TextFaint, Theme::Bg);
    tft_.setTextDatum(ML_DATUM);
    tft_.drawString("Metrics already in use will swap slots.", 10, 228);
  }
}

void GaugeConfigScreen::handleTouch(int16_t x, int16_t y) {
  if (Theme::backButtonRect().contains(x, y)) {
    nav_.back();
    return;
  }

  const AppSettings& s = settings_.settings();
  if (primaryMode_) {
    UiRect metricRow{6, 36, 308, 54};
    if (metricRow.contains(x, y)) {
      picker_.setTargetSlot(0);
      nav_.show(ScreenId::MetricPicker);
      return;
    }
    for (uint8_t i = 0; i < 3; ++i) {
      if (styleButton(i).contains(x, y)) {
        settings_.setPrimaryStyle(static_cast<GaugeStyle>(i));
        enter();
        return;
      }
    }
    return;
  }

  for (uint8_t slot = 1; slot <= SecondarySlotCount; ++slot) {
    if (slotRow(slot).contains(x, y)) {
      picker_.setTargetSlot(slot);
      nav_.show(ScreenId::MetricPicker);
      return;
    }
  }

  if (UiRect{6, 180, 150, 36}.contains(x, y)) {
    settings_.setIndicatorStyle(IndicatorStyle::Bar);
    enter();
  } else if (UiRect{164, 180, 150, 36}.contains(x, y)) {
    settings_.setIndicatorStyle(IndicatorStyle::Dots);
    enter();
  }
}
