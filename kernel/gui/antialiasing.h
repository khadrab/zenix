// kernel/gui/antialiasing.h - Modern Anti-Aliasing System
#ifndef ANTIALIASING_H
#define ANTIALIASING_H

#include "../../include/types.h"
#include "color.h"

// Anti-aliasing quality levels
typedef enum {
    AA_NONE = 0,      // No anti-aliasing
    AA_FAST = 1,      // Fast (2x2 sampling)
    AA_NORMAL = 2,    // Normal (4x4 sampling)
    AA_HIGH = 3       // High quality (8x8 sampling)
} aa_quality_t;

// Font rendering with anti-aliasing
typedef struct {
    uint8_t* bitmap;     // Character bitmap
    uint8_t width;       // Character width
    uint8_t height;      // Character height
    int8_t offset_x;     // Horizontal offset
    int8_t offset_y;     // Vertical offset
    uint8_t advance;     // Advance to next character
} aa_glyph_t;

// Anti-aliased text rendering context
typedef struct {
    rgba_color_t fg_color;
    rgba_color_t bg_color;
    aa_quality_t quality;
    uint8_t size;        // Font size
    uint8_t bold;        // Bold text
    uint8_t italic;      // Italic text
} aa_text_ctx_t;

// ============================================================================
// ANTI-ALIASED DRAWING FUNCTIONS
// ============================================================================

// Draw anti-aliased pixel (with sub-pixel positioning)
void aa_draw_pixel(uint8_t* buffer, uint32_t buf_width,
                   float x, float y,
                   rgba_color_t color);

// Draw anti-aliased line (Xiaolin Wu's algorithm)
void aa_draw_line(uint8_t* buffer, uint32_t buf_width, uint32_t buf_height,
                  float x1, float y1, float x2, float y2,
                  rgba_color_t color);

// Draw anti-aliased circle
void aa_draw_circle(uint8_t* buffer, uint32_t buf_width, uint32_t buf_height,
                    float cx, float cy, float radius,
                    rgba_color_t color);

// Fill anti-aliased circle (with smooth edges)
void aa_fill_circle(uint8_t* buffer, uint32_t buf_width, uint32_t buf_height,
                    float cx, float cy, float radius,
                    rgba_color_t color);

// Draw anti-aliased rounded rectangle
void aa_draw_rounded_rect(uint8_t* buffer, uint32_t buf_width, uint32_t buf_height,
                         float x, float y, float width, float height,
                         float radius, rgba_color_t color);

// Fill anti-aliased rounded rectangle
void aa_fill_rounded_rect(uint8_t* buffer, uint32_t buf_width, uint32_t buf_height,
                         float x, float y, float width, float height,
                         float radius, rgba_color_t color);

// ============================================================================
// ANTI-ALIASED TEXT RENDERING
// ============================================================================

// Render single character with anti-aliasing
void aa_draw_char(uint8_t* buffer, uint32_t buf_width, uint32_t buf_height,
                  int32_t x, int32_t y, char c,
                  aa_text_ctx_t* ctx);

// Render string with anti-aliasing
void aa_draw_text(uint8_t* buffer, uint32_t buf_width, uint32_t buf_height,
                  int32_t x, int32_t y, const char* text,
                  aa_text_ctx_t* ctx);

// Measure text width (for layout)
uint32_t aa_text_width(const char* text, aa_text_ctx_t* ctx);

// Get text height
uint32_t aa_text_height(aa_text_ctx_t* ctx);

// ============================================================================
// SUB-PIXEL RENDERING (ClearType-style)
// ============================================================================

// Draw text with sub-pixel rendering (better clarity on LCD)
void aa_draw_text_subpixel(uint8_t* buffer, uint32_t buf_width, uint32_t buf_height,
                           int32_t x, int32_t y, const char* text,
                           aa_text_ctx_t* ctx);

// ============================================================================
// HELPER FUNCTIONS
// ============================================================================

// Calculate coverage (0.0 to 1.0) for anti-aliasing
float aa_calculate_coverage(float x, float y, float center_x, float center_y, float radius);

// Apply anti-aliasing to existing pixel
rgba_color_t aa_blend_pixel(rgba_color_t new_color, rgba_color_t old_color, float coverage);

// Initialize anti-aliasing system
void aa_init(void);

// Set global anti-aliasing quality
void aa_set_quality(aa_quality_t quality);

// Get current quality
aa_quality_t aa_get_quality(void);

#endif // ANTIALIASING_H