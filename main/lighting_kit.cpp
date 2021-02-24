#include <OctoWS2811.h>
// #include <XLR8NeoPixel.h>
#include "lighting_kit.h"

#define NEOPIXEL_PIN 2
#define MAX_NUM_NEOPIXELS 30

// Strip length specified by user
uint16_t current_length = MAX_NUM_NEOPIXELS;
DMAMEM int octo_buffer[MAX_NUM_NEOPIXELS * 6];

// Neopixel object
OctoWS2811 strip(current_length, octo_buffer, NULL, WS2811_GRB | WS2811_800kHz);

// buffer for neopixel colors
uint32_t strip_buffer[MAX_NUM_NEOPIXELS] = {};

void lk_init(uint16_t strip_length, uint32_t starting_color){

  // Start neopixels
  strip.begin();
  strip.show();

  // Validate length
  if(strip_length > MAX_NUM_NEOPIXELS) return;

  // Set length
  current_length = strip_length;

  // Initialize colors
  lk_memset_full_neopixel_buffer(starting_color);
  
}

void lk_set_num_pixels(uint16_t num_pixels){
  
  current_length = num_pixels;
  
}

uint16_t lk_get_length(void){
  
  return current_length;  
  
}

// Sets pixels to buffer
void lk_set_pixels_to_buffer(void){

  // Initilize pixel color
  for(uint16_t count = 0; count < current_length; count++){

    strip.setPixel(count, strip_buffer[count]);
      
  }
    
}

// Memsets 32 bit buffer to specific value
void lk_memset_full_neopixel_buffer(uint32_t color){

  for(uint16_t count = 0; count < current_length; count++){

    strip_buffer[count] = color;
      
  }
  
}

// Changes a single pixel color in the buffer
void lk_set_pixel_color(uint16_t pixel, uint32_t color){

  // Validate index
  if(pixel >= current_length) return;

  // Set the color
  strip_buffer[pixel] = color;
  
}

// Updates the pixels in the XLR8 library and displays them
void lk_update_pixels(void){

  lk_set_pixels_to_buffer();
  while(strip.busy());
  strip.show();
  
}
