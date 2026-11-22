#include "ui/GaugeRenderer.h"

#include <math.h>

#include "ui/Theme.h"

namespace {

float clamp01(float value) {
  if (value < 0.0f) return 0.0f;
  if (value > 1.0f) return 1.0f;
  return value;
}

void drawArcRange(TFT_eSPI& tft, const ArcSpec& spec, float from, float to, uint16_t color,
                  uint16_t background) {
  from = clamp01(from);
  to = clamp01(to);
  if (to <= from) return;

  int a0 = static_cast<int>(lroundf(spec.startAngle + spec.sweep * from));
  int a1 = static_cast<int>(lroundf(spec.startAngle + spec.sweep * to));
  if (a0 >= 360) a0 = 359;
  if (a1 > 359) a1 = 359;
  if (a1 <= a0) return;

  tft.drawArc(spec.cx, spec.cy, spec.r, spec.ir, static_cast<uint32_t>(a0),
              static_cast<uint32_t>(a1), color, background, false);
}

int16_t padFor(const UiRect& rect) { return rect.w < 110 ? 6 : 10; }

// Baseline shared by the value and its unit so every card reads as one line,
// whatever font size the value ends up using.
constexpr int16_t kCardValueBaseline = 45;

UiRect indicatorRect(const UiRect& rect) {
  int16_t pad = padFor(rect);
  return UiRect{static_cast<int16_t>(rect.x + pad), static_cast<int16_t>(rect.y + rect.h - 9),
                static_cast<int16_t>(rect.w - pad * 2), 5};
}

// Widths are measured before drawing so a value and its unit never share the
// same pixels: the unit moves up to the label row when the card is too narrow
// for the widest value at the smallest size.
struct CardTextLayout {
  bool unitInHeader = false;
  int16_t valueMaxWidth = 0;
  int16_t labelMaxWidth = 0;
};

CardTextLayout cardTextLayout(TFT_eSPI& tft, const UiRect& rect, MetricId metric,
                             const char* value) {
  int16_t pad = padFor(rect);
  int16_t content = static_cast<int16_t>(rect.w - pad * 2);
  int16_t unit = static_cast<int16_t>(Theme::textWidth(tft, metricUnit(metric), FONT_LABEL));
  int16_t available = static_cast<int16_t>(content - unit - 6);
  const GFXfont* const* fonts = Theme::compactValueFonts();
  int16_t narrowest = static_cast<int16_t>(
      Theme::textWidth(tft, value, fonts[Theme::CompactValueFontCount - 1]));

  CardTextLayout layout;
  layout.unitInHeader = available < 8 || narrowest > available;
  layout.valueMaxWidth = layout.unitInHeader ? content : available;
  layout.labelMaxWidth = layout.unitInHeader ? static_cast<int16_t>(content - unit - 4) : content;
  return layout;
}

// Prefers the descriptive name and only falls back to the compact one when the
// card is too narrow, so tiles keep full labels wherever they fit.
const char* cardLabel(TFT_eSPI& tft, MetricId metric, int16_t maxWidth) {
  if (Theme::textWidth(tft, metricShortName(metric), FONT_LABEL_BOLD) <= maxWidth) {
    return metricShortName(metric);
  }
  return metricCardLabel(metric);
}

void drawCardText(TFT_eSPI& tft, const UiRect& rect, MetricId metric, const char* value,
                  bool warning, bool full) {
  int16_t pad = padFor(rect);
  CardTextLayout layout = cardTextLayout(tft, rect, metric, value);

  if (full) {
    Theme::drawTextElided(tft, cardLabel(tft, metric, layout.labelMaxWidth), rect.x + pad,
                          rect.y + 8, FONT_LABEL_BOLD,
                          warning ? Theme::Warn : Theme::TextDim, Theme::Panel, ML_DATUM,
                          layout.labelMaxWidth);
  }

  // When the unit had to move up, keep the value at the compact size so it stays
  // clear of both the header row above it and the indicator below it.
  const GFXfont* const* fonts = Theme::compactValueFonts();
  uint8_t fontCount = Theme::CompactValueFontCount;
  if (layout.unitInHeader) {
    fonts += fontCount - 1;
    fontCount = 1;
  }
  const GFXfont* font = Theme::fitFont(tft, value, layout.valueMaxWidth, fonts, fontCount);
  tft.setFreeFont(font);
  tft.setTextColor(warning ? Theme::Danger : Theme::Text, Theme::Panel);
  tft.setTextDatum(BL_DATUM);
  tft.drawString(value, rect.x + pad, rect.y + kCardValueBaseline);

  if (layout.unitInHeader) {
    if (full) {
      tft.setFreeFont(FONT_LABEL);
      tft.setTextColor(Theme::TextDim, Theme::Panel);
      tft.setTextDatum(TR_DATUM);
      tft.drawString(metricUnit(metric), rect.x + rect.w - pad, rect.y + 8);
    }
    return;
  }

  tft.setFreeFont(FONT_LABEL);
  tft.setTextColor(Theme::TextDim, Theme::Panel);
  tft.setTextDatum(BR_DATUM);
  tft.drawString(metricUnit(metric), rect.x + rect.w - pad, rect.y + kCardValueBaseline);
}

void drawIndicator(TFT_eSPI& tft, const UiRect& indicator, float progress, bool warning,
                   uint16_t accentColor, IndicatorStyle style) {
  tft.fillRect(indicator.x, indicator.y - 1, indicator.w, indicator.h + 2, Theme::Panel);
  progress = clamp01(progress);

  if (style == IndicatorStyle::Dots) {
    constexpr uint8_t dotCount = 5;
    int16_t step = indicator.w / dotCount;
    uint8_t filled = static_cast<uint8_t>(ceilf(progress * dotCount));
    for (uint8_t i = 0; i < dotCount; ++i) {
      int16_t x = indicator.x + step * i + step / 2;
      int16_t y = indicator.y + indicator.h / 2;
      tft.fillCircle(x, y, 2, i < filled ? (warning ? Theme::Danger : accentColor)
                                         : Theme::Track);
    }
    return;
  }

  tft.fillRoundRect(indicator.x, indicator.y, indicator.w, indicator.h,
                    indicator.h / 2, Theme::Track);
  int16_t fillW = static_cast<int16_t>(lroundf(indicator.w * progress));
  if (fillW > 1) {
    tft.fillRoundRect(indicator.x, indicator.y, fillW, indicator.h, indicator.h / 2,
                      warning ? Theme::Danger : accentColor);
  }
}

}  // namespace

namespace GaugeRenderer {

void drawArcTrack(TFT_eSPI& tft, const ArcSpec& spec, uint16_t color, uint16_t background) {
  drawArcRange(tft, spec, 0.0f, 1.0f, color, background);
}

void drawArcTicks(TFT_eSPI& tft, const ArcSpec& spec, uint8_t majorCount, uint8_t minorPerMajor,
                  uint16_t majorColor, uint16_t minorColor) {
  if (majorCount < 2) return;
  uint8_t total = static_cast<uint8_t>((majorCount - 1) * (minorPerMajor + 1) + 1);
  for (uint8_t i = 0; i < total; ++i) {
    bool major = (i % (minorPerMajor + 1)) == 0;
    float fraction = static_cast<float>(i) / static_cast<float>(total - 1);
    float angle = static_cast<float>(spec.startAngle) + spec.sweep * fraction;
    float rad = angle * 0.01745329252f;
    float sx = -sinf(rad);
    float sy = cosf(rad);

    int16_t inner = static_cast<int16_t>(spec.r + 2);
    int16_t outer = static_cast<int16_t>(spec.r + (major ? spec.tickOuter : spec.tickOuter - 3));
    int16_t x0 = static_cast<int16_t>(spec.cx + sx * inner);
    int16_t y0 = static_cast<int16_t>(spec.cy + sy * inner);
    int16_t x1 = static_cast<int16_t>(spec.cx + sx * outer);
    int16_t y1 = static_cast<int16_t>(spec.cy + sy * outer);
    tft.drawLine(x0, y0, x1, y1, major ? majorColor : minorColor);
    if (major) {
      tft.drawLine(x0 + 1, y0, x1 + 1, y1, majorColor);
    }
  }
}

void drawArcProgress(TFT_eSPI& tft, const ArcSpec& spec, float progress, uint16_t color,
                     uint16_t background) {
  drawArcRange(tft, spec, 0.0f, clamp01(progress), color, background);
}

void updateArcProgress(TFT_eSPI& tft, const ArcSpec& spec, float oldProgress, float newProgress,
                       uint16_t color, uint16_t background) {
  oldProgress = clamp01(oldProgress);
  newProgress = clamp01(newProgress);
  if (fabsf(newProgress - oldProgress) < 0.001f) return;

  if (newProgress > oldProgress) {
    drawArcRange(tft, spec, oldProgress, newProgress, color, background);
  } else {
    drawArcRange(tft, spec, newProgress, oldProgress, Theme::Track, background);
  }
}

void drawBar(TFT_eSPI& tft, const UiRect& rect, float progress, uint16_t fill, uint16_t track,
             bool segmented) {
  progress = clamp01(progress);
  tft.fillRoundRect(rect.x, rect.y, rect.w, rect.h, rect.h / 2, track);

  int16_t fillW = static_cast<int16_t>(lroundf(rect.w * progress));
  if (fillW <= 1) return;

  if (segmented) {
    constexpr uint8_t segments = 8;
    int16_t gap = 2;
    int16_t segW = (rect.w - gap * (segments - 1)) / segments;
    uint8_t lit = static_cast<uint8_t>(ceilf(progress * segments));
    for (uint8_t i = 0; i < lit; ++i) {
      tft.fillRect(rect.x + i * (segW + gap), rect.y, segW, rect.h, fill);
    }
  } else {
    tft.fillRoundRect(rect.x, rect.y, fillW, rect.h, rect.h / 2, fill);
  }
}

void updateBar(TFT_eSPI& tft, const UiRect& rect, float progress, uint16_t fill, uint16_t track,
               bool segmented) {
  drawBar(tft, rect, progress, fill, track, segmented);
}

void drawSecondaryCard(TFT_eSPI& tft, const UiRect& rect, MetricId metric, const char* value,
                       float progress, bool warning, IndicatorStyle style, bool showIndicator,
                       uint16_t accentColor) {
  tft.fillRoundRect(rect.x, rect.y, rect.w, rect.h, 8, Theme::Panel);
  tft.drawRoundRect(rect.x, rect.y, rect.w, rect.h, 8,
                    warning ? Theme::dim(Theme::Danger, 45) : Theme::Stroke);

  drawCardText(tft, rect, metric, value, warning, true);

  if (showIndicator) {
    drawIndicator(tft, indicatorRect(rect), progress, warning, accentColor, style);
  }
}

void updateSecondaryCard(TFT_eSPI& tft, const UiRect& rect, MetricId metric, const char* value,
                         float progress, bool warning, IndicatorStyle style, bool showIndicator,
                         uint16_t accentColor) {
  int16_t pad = padFor(rect);
  // Clear the whole value row so a shorter value cannot leave old glyphs behind.
  tft.fillRect(rect.x + pad - 2, rect.y + 17, rect.w - pad * 2 + 4, 36, Theme::Panel);

  drawCardText(tft, rect, metric, value, warning, false);

  if (showIndicator) {
    drawIndicator(tft, indicatorRect(rect), progress, warning, accentColor, style);
  } else {
    UiRect indicator = indicatorRect(rect);
    tft.fillRect(indicator.x, indicator.y - 1, indicator.w, indicator.h + 2, Theme::Panel);
  }
}

}  // namespace GaugeRenderer
