#include "ui/screens/MetricPickerScreen.h"

#include <stdio.h>

#include "ui/Theme.h"

namespace {

int assignSlotFor(const AppSettings& settings, MetricId metric) {
  if (settings.primaryMetric == metric) return 0;
  for (uint8_t i = 0; i < SecondarySlotCount; ++i) {
    if (settings.secondaryMetrics[i] == metric) return static_cast<int>(i) + 1;
  }
  return -1;
}

}  // namespace

MetricPickerScreen::MetricPickerScreen(TFT_eSPI& tft, SettingsManager& settings,
                                       ScreenManager& nav)
    : tft_(tft), settings_(settings), nav_(nav) {}

UiRect MetricPickerScreen::rowRect(uint8_t index) const {
  return UiRect{4, static_cast<int16_t>(36 + index * 28), 312, 26};
}

void MetricPickerScreen::enter() {
  Theme::setActiveAccent(settings_.settings().accent);
  Theme::clear(tft_);

  char title[24];
  if (targetSlot_ == 0) {
    snprintf(title, sizeof(title), "PRIMARY METRIC");
  } else {
    snprintf(title, sizeof(title), "SLOT %u METRIC", static_cast<unsigned>(targetSlot_));
  }
  Theme::drawHeader(tft_, title);

  uint16_t accentColor = Theme::activeAccent();
  const AppSettings& settings = settings_.settings();

  for (uint8_t i = 0; i < MetricCount; ++i) {
    MetricId metric = static_cast<MetricId>(i);
    int assignedSlot = assignSlotFor(settings, metric);
    bool selected = assignedSlot == static_cast<int>(targetSlot_);
    UiRect row = rowRect(i);

    uint16_t fill = selected ? Theme::PanelSelected : Theme::Panel;
    tft_.fillRoundRect(row.x, row.y, row.w, row.h, 5, fill);
    tft_.drawRoundRect(row.x, row.y, row.w, row.h, 5,
                       selected ? accentColor : Theme::Stroke);

    if (selected) {
      tft_.fillCircle(row.x + 10, row.y + row.h / 2, 3, accentColor);
    }

    tft_.setFreeFont(FONT_LABEL_BOLD);
    tft_.setTextColor(selected ? Theme::Text : Theme::TextDim, fill);
    tft_.setTextDatum(ML_DATUM);
    tft_.drawString(metricInfo(metric).name, row.x + 20, row.y + row.h / 2);

    if (selected) {
      tft_.setFreeFont(FONT_LABEL);
      tft_.setTextColor(accentColor, fill);
      tft_.setTextDatum(MR_DATUM);
      tft_.drawString("CURRENT", row.x + row.w - 10, row.y + row.h / 2);
    } else if (assignedSlot == 0) {
      tft_.setFreeFont(FONT_LABEL);
      tft_.setTextColor(Theme::TextFaint, fill);
      tft_.setTextDatum(MR_DATUM);
      tft_.drawString("PRIMARY", row.x + row.w - 10, row.y + row.h / 2);
    } else if (assignedSlot > 0) {
      char tag[16];
      snprintf(tag, sizeof(tag), "SLOT %d", assignedSlot);
      tft_.setFreeFont(FONT_LABEL);
      tft_.setTextColor(Theme::TextFaint, fill);
      tft_.setTextDatum(MR_DATUM);
      tft_.drawString(tag, row.x + row.w - 10, row.y + row.h / 2);
    } else {
      tft_.setFreeFont(FONT_LABEL);
      tft_.setTextColor(Theme::TextFaint, fill);
      tft_.setTextDatum(MR_DATUM);
      tft_.drawString(metricUnit(metric), row.x + row.w - 10, row.y + row.h / 2);
    }
  }
}

void MetricPickerScreen::handleTouch(int16_t x, int16_t y) {
  if (Theme::backButtonRect().contains(x, y)) {
    nav_.back();
    return;
  }

  for (uint8_t i = 0; i < MetricCount; ++i) {
    if (!rowRect(i).contains(x, y)) continue;
    if (settings_.assignMetric(targetSlot_, static_cast<MetricId>(i))) {
      nav_.back();
    }
    return;
  }
}
