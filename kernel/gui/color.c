// kernel/gui/color.c - RGBA Color System Implementation (Fixed)
#include "color.h"

uint8_t rgba_to_vga(rgba_color_t color) {
    // Handle transparency
    if (color.a < 128) {
        return 0xFF;  // Special value for "no change"
    }
    
    // For the standard colors, match exactly
    if (color.r == 0 && color.g == 0 && color.b == 0) return 0;  // Black
    if (color.r >= 250 && color.g >= 250 && color.b >= 250) return 0x3F;  // White
    
    // For orange (Material Design)
    if (color.r > 200 && color.g < 120 && color.b < 60) return 0x0C;  // Bright red
    
    // For purple
    if (color.r > 100 && color.r < 150 && color.g < 60 && color.b > 60) return 0x05;  // Magenta
    
    // Grayscale detection
    int max_diff = color.r > color.g ? color.r - color.g : color.g - color.r;
    if (max_diff < 30) {
        int diff2 = color.r > color.b ? color.r - color.b : color.b - color.r;
        if (diff2 < 30) {
            // It's grayscale
            uint8_t intensity = (color.r + color.g + color.b) / 3;
            if (intensity < 32) return 0x00;      // Black
            if (intensity < 96) return 0x08;      // Dark gray
            if (intensity < 160) return 0x07;     // Gray
            if (intensity < 224) return 0x38;     // Light gray
            return 0x3F;                           // White
        }
    }
    
    // Color mapping
    uint8_t r = (color.r * 5) / 255;
    uint8_t g = (color.g * 5) / 255;
    uint8_t b = (color.b * 5) / 255;
    
    if (r > 3 && g < 2 && b < 2) return 0x0C;  // Red
    if (g > 3 && r < 2 && b < 2) return 0x02;  // Green
    if (b > 3 && r < 2 && g < 2) return 0x09;  // Blue
    if (r > 3 && g > 3 && b < 2) return 0x0E;  // Yellow
    if (r > 3 && b > 3 && g < 2) return 0x0D;  // Magenta
    if (g > 3 && b > 3 && r < 2) return 0x0B;  // Cyan
    
    // Default to closest match
    if (color.r + color.g + color.b < 128) return 0x08;  // Dark
    if (color.r + color.g + color.b < 384) return 0x07;  // Medium
    return 0x38;  // Light
}

rgba_color_t rgba_blend(rgba_color_t fg, rgba_color_t bg) {
    if (fg.a == 255) return fg;
    if (fg.a == 0) return bg;
    
    // Alpha blending formula
    float alpha = fg.a / 255.0f;
    float inv_alpha = 1.0f - alpha;
    
    rgba_color_t result;
    result.r = (uint8_t)(fg.r * alpha + bg.r * inv_alpha);
    result.g = (uint8_t)(fg.g * alpha + bg.g * inv_alpha);
    result.b = (uint8_t)(fg.b * alpha + bg.b * inv_alpha);
    result.a = 255;
    
    return result;
}

rgba_color_t rgba_darken(rgba_color_t color, float factor) {
    if (factor < 0.0f) factor = 0.0f;
    if (factor > 1.0f) factor = 1.0f;
    
    rgba_color_t result;
    result.r = (uint8_t)(color.r * factor);
    result.g = (uint8_t)(color.g * factor);
    result.b = (uint8_t)(color.b * factor);
    result.a = color.a;
    
    return result;
}

rgba_color_t rgba_lighten(rgba_color_t color, float factor) {
    if (factor < 1.0f) factor = 1.0f;
    
    // Use int to prevent overflow
    int r = color.r + (int)((255 - color.r) * (factor - 1.0f));
    int g = color.g + (int)((255 - color.g) * (factor - 1.0f));
    int b = color.b + (int)((255 - color.b) * (factor - 1.0f));
    
    rgba_color_t result;
    result.r = (r > 255) ? 255 : (uint8_t)r;
    result.g = (g > 255) ? 255 : (uint8_t)g;
    result.b = (b > 255) ? 255 : (uint8_t)b;
    result.a = color.a;
    
    return result;
}

rgba_color_t rgba_mix(rgba_color_t c1, rgba_color_t c2, float ratio) {
    if (ratio < 0.0f) ratio = 0.0f;
    if (ratio > 1.0f) ratio = 1.0f;
    
    float inv_ratio = 1.0f - ratio;
    
    rgba_color_t result;
    result.r = (uint8_t)(c1.r * inv_ratio + c2.r * ratio);
    result.g = (uint8_t)(c1.g * inv_ratio + c2.g * ratio);
    result.b = (uint8_t)(c1.b * inv_ratio + c2.b * ratio);
    result.a = (uint8_t)(c1.a * inv_ratio + c2.a * ratio);
    
    return result;
}