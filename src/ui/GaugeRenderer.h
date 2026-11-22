#pragma once

#include <TFT_eSPI.h>

#include "core/AppTypes.h"

struct ArcSpec {
  int16_t cx = 0;
  int16_t cy = 0;
  int16_t r = 0;
  int16_t ir = 0;
  uint16_t startAngle = 45;
  uint16_t sweep = 270;
  int16_t tickOuter = 7;
};

namespace GaugeRenderer {

void drawArcTrack(TFT_eSPI& tft, const ArcSpec& spec, uint16_t color, uint16_t background);
void drawArcTicks(TFT_eSPI& tft, const ArcSpec& spec, uint8_t majorCount, uint8_t minorPerMajor,
                  uint16_t majorColor, uint16_t minorColor);
void drawArcProgress(TFT_eSPI& tft, const ArcSpec& spec, float progress, uint16_t color,
                     uint16_t background);
void updateArcProgress(TFT_eSPI& tft, const ArcSpec& spec, float oldProgress, float newProgress,
                       uint16_t color, uint16_t background);

void drawBar(TFT_eSPI& tft, const UiRect& rect, float progress, uint16_t fill, uint16_t track,
             bool segmented);
void updateBar(TFT_eSPI& tft, const UiRect& rect, float progress, uint16_t fill, uint16_t track,
               bool segmented);

void drawSecondaryCard(TFT_eSPI& tft, const UiRect& rect, MetricId metric, const char* value,
                       float progress, bool warning, IndicatorStyle style, bool showIndicator,
                       uint16_t accentColor);
void updateSecondaryCard(TFT_eSPI& tft, const UiRect& rect, MetricId metric, const char* value,
                         float progress, bool warning, IndicatorStyle style, bool showIndicator,
                         uint16_t accentColor);

}  // namespace GaugeRenderer
