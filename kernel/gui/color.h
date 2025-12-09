// kernel/gui/color.h - Professional RGBA Color System
#ifndef COLOR_H
#define COLOR_H

#include "../../include/types.h"

// RGBA Color structure
typedef struct {
    uint8_t r;  // Red (0-255)
    uint8_t g;  // Green (0-255)
    uint8_t b;  // Blue (0-255)
    uint8_t a;  // Alpha (0-255, 255 = opaque)
} rgba_color_t;

// VGA 256-color palette mapping
// We use a pre-calculated palette for best color matching
typedef struct {
    uint8_t vga_index;
    rgba_color_t rgba;
} palette_entry_t;

// Material Design Color Palette (Modern & Professional)
#define COLOR_MD_RED           ((rgba_color_t){244, 67, 54, 255})    // #F44336
#define COLOR_MD_PINK          ((rgba_color_t){233, 30, 99, 255})    // #E91E63
#define COLOR_MD_PURPLE        ((rgba_color_t){156, 39, 176, 255})   // #9C27B0
#define COLOR_MD_DEEP_PURPLE   ((rgba_color_t){103, 58, 183, 255})   // #673AB7
#define COLOR_MD_INDIGO        ((rgba_color_t){63, 81, 181, 255})    // #3F51B5
#define COLOR_MD_BLUE          ((rgba_color_t){33, 150, 243, 255})   // #2196F3
#define COLOR_MD_LIGHT_BLUE    ((rgba_color_t){3, 169, 244, 255})    // #03A9F4
#define COLOR_MD_CYAN          ((rgba_color_t){0, 188, 212, 255})    // #00BCD4
#define COLOR_MD_TEAL          ((rgba_color_t){0, 150, 136, 255})    // #009688
#define COLOR_MD_GREEN         ((rgba_color_t){76, 175, 80, 255})    // #4CAF50
#define COLOR_MD_LIGHT_GREEN   ((rgba_color_t){139, 195, 74, 255})   // #8BC34A
#define COLOR_MD_LIME          ((rgba_color_t){205, 220, 57, 255})   // #CDDC39
#define COLOR_MD_YELLOW        ((rgba_color_t){255, 235, 59, 255})   // #FFEB3B
#define COLOR_MD_AMBER         ((rgba_color_t){255, 193, 7, 255})    // #FFC107
#define COLOR_MD_ORANGE        ((rgba_color_t){255, 152, 0, 255})    // #FF9800
#define COLOR_MD_DEEP_ORANGE   ((rgba_color_t){255, 87, 34, 255})    // #FF5722

// Grays (Material Design)
#define COLOR_MD_GRAY_50       ((rgba_color_t){250, 250, 250, 255})  // #FAFAFA
#define COLOR_MD_GRAY_100      ((rgba_color_t){245, 245, 245, 255})  // #F5F5F5
#define COLOR_MD_GRAY_200      ((rgba_color_t){238, 238, 238, 255})  // #EEEEEE
#define COLOR_MD_GRAY_300      ((rgba_color_t){224, 224, 224, 255})  // #E0E0E0
#define COLOR_MD_GRAY_400      ((rgba_color_t){189, 189, 189, 255})  // #BDBDBD
#define COLOR_MD_GRAY_500      ((rgba_color_t){158, 158, 158, 255})  // #9E9E9E
#define COLOR_MD_GRAY_600      ((rgba_color_t){117, 117, 117, 255})  // #757575
#define COLOR_MD_GRAY_700      ((rgba_color_t){97, 97, 97, 255})     // #616161
#define COLOR_MD_GRAY_800      ((rgba_color_t){66, 66, 66, 255})     // #424242
#define COLOR_MD_GRAY_900      ((rgba_color_t){33, 33, 33, 255})     // #212121

// Special colors
#define COLOR_WHITE            ((rgba_color_t){255, 255, 255, 255})
#define COLOR_BLACK            ((rgba_color_t){0, 0, 0, 255})
#define COLOR_TRANSPARENT      ((rgba_color_t){0, 0, 0, 0})

// Ubuntu-specific colors
#define COLOR_UBUNTU_ORANGE    ((rgba_color_t){233, 84, 32, 255})    // #E95420
#define COLOR_UBUNTU_PURPLE    ((rgba_color_t){119, 41, 83, 255})    // #772953
#define COLOR_UBUNTU_AUBERGINE ((rgba_color_t){46, 52, 54, 255})     // #2E3436

// Convert RGBA to nearest VGA 256 color index
uint8_t rgba_to_vga(rgba_color_t color);

// Blend two colors with alpha
rgba_color_t rgba_blend(rgba_color_t fg, rgba_color_t bg);

// Create RGBA color
static inline rgba_color_t rgba(uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
    rgba_color_t c = {r, g, b, a};
    return c;
}

// Darken color (for shadows)
rgba_color_t rgba_darken(rgba_color_t color, float factor);

// Lighten color (for highlights)
rgba_color_t rgba_lighten(rgba_color_t color, float factor);

// Get intermediate color for anti-aliasing
rgba_color_t rgba_mix(rgba_color_t c1, rgba_color_t c2, float ratio);

#endif // COLOR_H