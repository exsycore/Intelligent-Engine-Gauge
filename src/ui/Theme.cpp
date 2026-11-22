#include "ui/Theme.h"

#include <math.h>
#include <stdio.h>
#include <string.h>

namespace {

uint8_t channelR(uint16_t color) { return static_cast<uint8_t>(((color >> 11) & 0x1F) << 3); }
uint8_t channelG(uint16_t color) { return static_cast<uint8_t>(((color >> 5) & 0x3F) << 2); }
uint8_t channelB(uint16_t color) { return static_cast<uint8_t>((color & 0x1F) << 3); }

const GFXfont* const kValueFonts[] = {&FreeSansBold24pt7b, &FreeSansBold18pt7b,
                                      &FreeSansBold12pt7b};
const GFXfont* const kCompactValueFonts[] = {&FreeSansBold18pt7b, &FreeSansBold12pt7b};

}  // namespace

namespace Theme {

namespace {
AccentId gActiveAccent = AccentId::Ice;
}

void setActiveAccent(AccentId id) { gActiveAccent = id; }
uint16_t activeAccent() { return accent(gActiveAccent); }

uint16_t accent(AccentId id) {
  switch (id) {
    case AccentId::Red:
      return AccentRed;
    case AccentId::Amber:
      return AccentAmber;
    case AccentId::Green:
      return AccentGreen;
    case AccentId::Ice:
    default:
      return AccentIce;
  }
}

uint16_t dim(uint16_t color, uint8_t amount) {
  uint16_t r = static_cast<uint16_t>(channelR(color) * (100 - amount) / 100);
  uint16_t g = static_cast<uint16_t>(channelG(color) * (100 - amount) / 100);
  uint16_t b = static_cast<uint16_t>(channelB(color) * (100 - amount) / 100);
  return rgb565(static_cast<uint8_t>(r), static_cast<uint8_t>(g), static_cast<uint8_t>(b));
}

void init(TFT_eSPI& tft) {
  tft.setRotation(1);
  tft.fillScreen(Bg);
  tft.setTextColor(Text, Bg);
  tft.setTextDatum(TL_DATUM);
}

void clear(TFT_eSPI& tft) { tft.fillScreen(Bg); }

int textWidth(TFT_eSPI& tft, const char* text, const GFXfont* font) {
  tft.setFreeFont(font);
  return tft.textWidth(text);
}

void drawText(TFT_eSPI& tft, const char* text, int16_t x, int16_t y, const GFXfont* font,
              uint16_t color, uint8_t datum) {
  tft.setFreeFont(font);
  tft.setTextColor(color, Bg);
  tft.setTextDatum(datum);
  tft.drawString(text, x, y);
}

const GFXfont* const* valueFonts() { return kValueFonts; }
const GFXfont* const* compactValueFonts() { return kCompactValueFonts; }

const GFXfont* fitFont(TFT_eSPI& tft, const char* text, int16_t maxWidth,
                       const GFXfont* const* fonts, uint8_t fontCount) {
  if (fonts == nullptr || fontCount == 0) return FONT_LABEL;
  for (uint8_t i = 0; i < fontCount; ++i) {
    if (textWidth(tft, text, fonts[i]) <= maxWidth) return fonts[i];
  }
  return fonts[fontCount - 1];
}

void elideText(TFT_eSPI& tft, const char* text, const GFXfont* font, int16_t maxWidth,
               char* out, size_t outSize) {
  if (out == nullptr || outSize == 0) return;
  char source[40];
  snprintf(source, sizeof(source), "%s", text != nullptr ? text : "");

  if (maxWidth <= 0) {
    out[0] = '\0';
    return;
  }
  if (textWidth(tft, source, font) <= maxWidth) {
    snprintf(out, outSize, "%s", source);
    return;
  }

  char trial[48];
  size_t length = strlen(source);
  while (length > 0) {
    --length;
    memcpy(trial, source, length);
    trial[length] = '.';
    trial[length + 1] = '.';
    trial[length + 2] = '\0';
    if (textWidth(tft, trial, font) <= maxWidth) {
      snprintf(out, outSize, "%s", trial);
      return;
    }
  }
  out[0] = '\0';
}

void drawTextElided(TFT_eSPI& tft, const char* text, int16_t x, int16_t y, const GFXfont* font,
                    uint16_t color, uint16_t background, uint8_t datum, int16_t maxWidth) {
  char buffer[48];
  elideText(tft, text, font, maxWidth, buffer, sizeof(buffer));
  tft.setFreeFont(font);
  tft.setTextColor(color, background);
  tft.setTextDatum(datum);
  tft.drawString(buffer, x, y);
}

void drawValueText(TFT_eSPI& tft, const UiRect& area, const char* text,
                   const GFXfont* const* fonts, uint8_t fontCount, uint16_t color,
                   uint16_t background) {
  tft.fillRect(area.x, area.y, area.w, area.h, background);
  const GFXfont* font = fitFont(tft, text, static_cast<int16_t>(area.w - 6), fonts, fontCount);
  tft.setFreeFont(font);
  tft.setTextColor(color, background);
  tft.setTextDatum(MC_DATUM);
  tft.drawString(text, area.x + area.w / 2, area.y + area.h / 2);
}

UiRect settingsButtonRect() { return UiRect{282, 0, 38, ScreenDim::StatusBarHeight}; }

void drawStatusBar(TFT_eSPI& tft, const char* source, bool simulated, const char* warning) {
  tft.fillRect(0, 0, ScreenDim::Width, ScreenDim::StatusBarHeight, Bg);
  tft.drawFastHLine(0, ScreenDim::StatusBarHeight - 1, ScreenDim::Width, Stroke);

  if (simulated) {
    tft.fillRoundRect(6, 5, 48, 17, 4, rgb565(42, 33, 16));
    tft.drawRoundRect(6, 5, 48, 17, 4, Demo);
    tft.setFreeFont(FONT_LABEL_BOLD);
    tft.setTextColor(Demo, rgb565(42, 33, 16));
    tft.setTextDatum(MC_DATUM);
    tft.drawString("DEMO", 30, 13);
  } else {
    tft.fillRoundRect(6, 5, 48, 17, 4, Panel);
    tft.drawRoundRect(6, 5, 48, 17, 4, Stroke);
    tft.setFreeFont(FONT_LABEL_BOLD);
    tft.setTextColor(Ok, Panel);
    tft.setTextDatum(MC_DATUM);
    tft.drawString("LIVE", 30, 13);
  }  tft.fillCircle(62, 13, 3, simulated ? Demo : Ok);
  drawTextElided(tft, source != nullptr ? source : "SIM", 69, 13, FONT_LABEL, TextDim, Bg,
                 ML_DATUM, 55);

  if (warning != nullptr) {
    drawWarningTriangle(tft, 128, 13, Warn);
    drawTextElided(tft, warning, 140, 13, FONT_LABEL_BOLD, Warn, Bg, ML_DATUM, 138);
  }

  UiRect gear = settingsButtonRect();
  tft.fillRoundRect(gear.x + 3, 4, 32, 20, 5, Panel);
  tft.drawRoundRect(gear.x + 3, 4, 32, 20, 5, Stroke);
  int16_t cx = gear.x + 19;
  tft.drawFastHLine(cx - 9, 9, 18, TextDim);
  tft.drawFastHLine(cx - 9, 14, 18, TextDim);
  tft.drawFastHLine(cx - 9, 19, 18, TextDim);
  tft.fillCircle(cx - 4, 9, 2, Text);
  tft.fillCircle(cx + 5, 14, 2, Text);
  tft.fillCircle(cx, 19, 2, Text);
}

UiRect backButtonRect() { return UiRect{3, 2, 44, 26}; }

void drawHeader(TFT_eSPI& tft, const char* title, const char* tag) {
  tft.fillRect(0, 0, ScreenDim::Width, 30, Bg);
  tft.drawFastHLine(0, 29, ScreenDim::Width, Stroke);

  UiRect back = backButtonRect();
  tft.fillRoundRect(back.x, back.y, back.w, back.h, 6, PanelRaised);
  tft.drawRoundRect(back.x, back.y, back.w, back.h, 6, Stroke);
  int16_t bx = back.x + 20;
  int16_t by = back.y + back.h / 2;
  tft.drawLine(bx + 3, by - 4, bx - 1, by, Text);
  tft.drawLine(bx - 1, by, bx + 3, by + 4, Text);

  int16_t titleMax = (tag != nullptr) ? 190 : 224;
  drawTextElided(tft, title, ScreenDim::Width / 2, 15, FONT_TITLE, Text, Bg, MC_DATUM, titleMax);

  if (tag != nullptr) {
    drawTextElided(tft, tag, ScreenDim::Width - 8, 15, FONT_LABEL_BOLD, Demo, Bg, MR_DATUM, 48);
  }
}

void drawChevron(TFT_eSPI& tft, int16_t x, int16_t y, uint16_t color) {
  tft.drawLine(x - 2, y - 4, x + 2, y, color);
  tft.drawLine(x + 2, y, x - 2, y + 4, color);
}

void drawCheck(TFT_eSPI& tft, int16_t x, int16_t y, uint16_t color) {
  tft.drawLine(x - 5, y, x - 1, y + 4, color);
  tft.drawLine(x - 1, y + 4, x + 6, y - 5, color);
}

void drawWarningTriangle(TFT_eSPI& tft, int16_t x, int16_t y, uint16_t color) {
  tft.drawTriangle(x, y - 7, x - 7, y + 5, x + 7, y + 5, color);
  tft.drawFastVLine(x, y - 3, 5, color);
  tft.drawPixel(x, y + 3, color);
}

void drawRow(TFT_eSPI& tft, const UiRect& rect, const char* label, const char* value,
             bool chevron, bool danger, bool selected) {
  uint16_t fill = selected ? PanelSelected : Panel;
  uint16_t border = selected ? activeAccent() : Stroke;
  tft.fillRoundRect(rect.x, rect.y, rect.w, rect.h, 6, fill);
  tft.drawRoundRect(rect.x, rect.y, rect.w, rect.h, 6, border);

  int16_t midY = rect.y + rect.h / 2;
  bool centeredOnly = danger && value == nullptr && !chevron;

  if (selected) {
    tft.fillRoundRect(rect.x + 1, rect.y + 7, 3, rect.h - 14, 2, activeAccent());
  }

  if (centeredOnly) {
    tft.setFreeFont(FONT_LABEL_BOLD);
    tft.setTextColor(Danger, fill);
    tft.setTextDatum(MC_DATUM);
    tft.drawString(label, rect.x + rect.w / 2, midY);
    return;
  }

  tft.setFreeFont(FONT_LABEL_BOLD);
  tft.setTextColor(danger ? Danger : Text, fill);
  int16_t labelX = rect.x + 12;
  int16_t valueRight = rect.x + rect.w - (chevron ? 26 : 12);
  int16_t valueLeft = labelX;
  if (value != nullptr) {
    valueLeft = static_cast<int16_t>(valueRight - textWidth(tft, value, FONT_LABEL));
  }
  drawTextElided(tft, label, labelX, midY, FONT_LABEL_BOLD, danger ? Danger : Text, fill,
                 ML_DATUM, static_cast<int16_t>(valueLeft - labelX - 8));

  if (value != nullptr) {
    drawTextElided(tft, value, valueRight, midY, FONT_LABEL, danger ? Danger : TextDim, fill,
                   MR_DATUM, static_cast<int16_t>(valueRight - labelX - 8));
  }

  if (chevron) {
    drawChevron(tft, rect.x + rect.w - 14, midY, TextDim);
  }
}

void drawToggleRow(TFT_eSPI& tft, const UiRect& rect, const char* label, bool enabled) {
  uint16_t fill = enabled ? PanelSelected : Panel;
  uint16_t border = enabled ? activeAccent() : Stroke;
  tft.fillRoundRect(rect.x, rect.y, rect.w, rect.h, 6, fill);
  tft.drawRoundRect(rect.x, rect.y, rect.w, rect.h, 6, border);

  int16_t midY = rect.y + rect.h / 2;
  tft.setFreeFont(FONT_LABEL_BOLD);
  tft.setTextColor(Text, fill);
  tft.setTextDatum(ML_DATUM);
  tft.drawString(label, rect.x + 12, midY);

  int16_t sw = 42;
  int16_t sh = 20;
  int16_t sx = rect.x + rect.w - sw - 12;
  int16_t sy = midY - sh / 2;
  tft.fillRoundRect(sx, sy, sw, sh, sh / 2, enabled ? activeAccent() : Track);
  int16_t knobX = enabled ? sx + sw - 10 : sx + 10;
  tft.fillCircle(knobX, sy + sh / 2, 7, enabled ? Bg : TextDim);

  tft.setFreeFont(FONT_LABEL);
  tft.setTextColor(enabled ? Text : TextFaint, fill);
  tft.setTextDatum(MR_DATUM);
  tft.drawString(enabled ? "ON" : "OFF", sx - 10, midY);
}

void drawSectionLabel(TFT_eSPI& tft, const char* label, int16_t x, int16_t y) {
  tft.setFreeFont(FONT_LABEL);
  tft.setTextColor(TextFaint, Bg);
  tft.setTextDatum(ML_DATUM);
  tft.drawString(label, x, y);
}

void drawLayoutGlyph(TFT_eSPI& tft, const UiRect& rect, LayoutId layout, uint16_t color) {
  tft.fillRoundRect(rect.x, rect.y, rect.w, rect.h, 5, Bg);
  tft.drawRoundRect(rect.x, rect.y, rect.w, rect.h, 5, Stroke);

  int16_t cx = rect.x + rect.w / 2;
  int16_t cy = rect.y + rect.h / 2;

  if (layout == LayoutId::Classic) {
    tft.drawCircle(cx - 2, cy, 8, color);
    tft.drawLine(cx - 2, cy, cx + 3, cy - 5, color);
    for (int i = 0; i < 3; ++i) {
      tft.fillRect(cx + 8, cy - 6 + i * 5, 6, 3, Stroke);
    }
  } else if (layout == LayoutId::Sport) {
    tft.drawArc(cx, cy + 4, 12, 9, 90, 270, color, Bg, false);
    tft.drawFastHLine(cx - 8, cy + 7, 16, color);
  } else if (layout == LayoutId::Minimal) {
    tft.drawFastHLine(cx - 12, cy - 5, 24, color);
    tft.drawFastHLine(cx - 8, cy, 16, TextDim);
    tft.drawFastHLine(cx - 8, cy + 5, 16, Stroke);
  } else {
    tft.drawCircle(cx, cy, 9, color);
    tft.drawCircle(cx, cy, 5, Stroke);
    tft.drawFastVLine(cx, cy - 12, 4, Stroke);
    tft.drawFastVLine(cx, cy + 9, 4, Stroke);
  }
}

}  // namespace Theme
