#ifndef LIGHTING_KIT_H
#define LIGHTING_KIT_H

#include<stdint.h>

#define LK_RED 0x200000
#define LK_GREEN 0x002000
#define LK_BLUE 0x000020
#define LK_BLACK 0x000000

// Function declarations
void lk_init(uint16_t strip_length, uint32_t starting_color);
void lk_set_pixels_to_buffer(void);
void lk_set_num_pixels(uint16_t num_pixels);
uint16_t lk_get_length(void);
void lk_memset_full_neopixel_buffer(uint32_t color);
void lk_update_pixels(void);
void lk_set_pixel_color(uint16_t pixel, uint32_t color);

#endif
