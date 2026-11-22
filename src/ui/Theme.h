#pragma once

#include <TFT_eSPI.h>

#include "core/AppTypes.h"

// Shared design system: palette, type scale, spacing and common widgets.
namespace Theme {

constexpr uint16_t rgb565(uint8_t r, uint8_t g, uint8_t b) {
  return static_cast<uint16_t>(((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3));
}

constexpr uint16_t Bg = rgb565(5, 8, 12);
constexpr uint16_t Panel = rgb565(18, 22, 28);
constexpr uint16_t PanelRaised = rgb565(24, 30, 37);
constexpr uint16_t PanelSelected = rgb565(27, 38, 48);
constexpr uint16_t Stroke = rgb565(43, 52, 64);
constexpr uint16_t Text = rgb565(240, 245, 250);
constexpr uint16_t TextDim = rgb565(138, 150, 164);
constexpr uint16_t TextFaint = rgb565(82, 94, 108);
constexpr uint16_t Track = rgb565(38, 46, 56);
constexpr uint16_t Danger = rgb565(255, 82, 82);
constexpr uint16_t Warn = rgb565(255, 176, 32);
constexpr uint16_t Demo = rgb565(255, 194, 71);
constexpr uint16_t Ok = rgb565(82, 224, 138);
constexpr uint16_t AccentIce = rgb565(64, 214, 255);
constexpr uint16_t AccentRed = rgb565(255, 77, 77);
constexpr uint16_t AccentAmber = rgb565(255, 176, 32);
constexpr uint16_t AccentGreen = rgb565(82, 224, 138);

// Type scale. Numbers are drawn through valueFonts() so they can shrink to fit.
#define FONT_LABEL (&FreeSans9pt7b)
#define FONT_LABEL_BOLD (&FreeSansBold9pt7b)
#define FONT_TITLE (&FreeSansBold12pt7b)

uint16_t accent(AccentId id);
void setActiveAccent(AccentId id);
uint16_t activeAccent();
uint16_t dim(uint16_t color, uint8_t amount);
void init(TFT_eSPI& tft);
void clear(TFT_eSPI& tft);

int textWidth(TFT_eSPI& tft, const char* text, const GFXfont* font);
void drawText(TFT_eSPI& tft, const char* text, int16_t x, int16_t y, const GFXfont* font,
              uint16_t color, uint8_t datum);

// Number type scale, largest first. Fitting picks the biggest entry that fits.
constexpr uint8_t ValueFontCount = 3;
constexpr uint8_t CompactValueFontCount = 2;
const GFXfont* const* valueFonts();
const GFXfont* const* compactValueFonts();

const GFXfont* fitFont(TFT_eSPI& tft, const char* text, int16_t maxWidth,
                       const GFXfont* const* fonts, uint8_t fontCount);
void elideText(TFT_eSPI& tft, const char* text, const GFXfont* font, int16_t maxWidth,
               char* out, size_t outSize);
void drawTextElided(TFT_eSPI& tft, const char* text, int16_t x, int16_t y, const GFXfont* font,
                    uint16_t color, uint16_t background, uint8_t datum, int16_t maxWidth);

// Erases `area` (nothing else) and draws `text` centred in it with the largest value
// font that fits. The erasing rectangle is the caller's contract: keep it clear of
// gauge arcs and neighbouring content.
void drawValueText(TFT_eSPI& tft, const UiRect& area, const char* text,
                   const GFXfont* const* fonts, uint8_t fontCount, uint16_t color,
                   uint16_t background);

void drawStatusBar(TFT_eSPI& tft, const char* source, bool simulated, const char* warning);
UiRect settingsButtonRect();
void drawHeader(TFT_eSPI& tft, const char* title, const char* tag = nullptr);
UiRect backButtonRect();

void drawRow(TFT_eSPI& tft, const UiRect& rect, const char* label, const char* value,
             bool chevron, bool danger = false, bool selected = false);
void drawToggleRow(TFT_eSPI& tft, const UiRect& rect, const char* label, bool enabled);
void drawSectionLabel(TFT_eSPI& tft, const char* label, int16_t x, int16_t y);
void drawChevron(TFT_eSPI& tft, int16_t x, int16_t y, uint16_t color);
void drawCheck(TFT_eSPI& tft, int16_t x, int16_t y, uint16_t color);
void drawWarningTriangle(TFT_eSPI& tft, int16_t x, int16_t y, uint16_t color);
void drawLayoutGlyph(TFT_eSPI& tft, const UiRect& rect, LayoutId layout, uint16_t color);

}  // namespace Theme
