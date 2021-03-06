#include <Arduino.h>
#include "string.h"
#include <stdlib.h>
#include "lighting_kit.h"
#include "command_parser.h"

// Example command
// {COMMAND:PARAM_1,PARAM_2,PARAM_3,PARAM_4}\n

// Local functions
static void set_one_neopixel(uint16_t neopixel_index, uint32_t rgb_color);
static bool set_many_neopixels(uint16_t neopixel_index, uint32_t rgb_color, uint16_t num_pixels);
static void delay_millis(uint32_t millis_to_delay);
static void set_max_pixels(uint16_t max_pixels);
static void neopixel_init(uint16_t num_pixels, uint32_t rgb_color);
static void update_neopixels(void);
static void fast_mode(void);
static void get_fast_command(char * fast_buffer);

// Resets the command buffer and variables
void cp_reset_command_buffer(command * current_command){

  current_command->command_received = false;
  current_command->command_buffer_index = 0;
  memset(current_command->command_buffer, 0, COMMAND_BUF_LEN);
  memset(current_command->command_type, 0, COMMAND_TYPE_LEN);
  for(uint16_t count = 0; count < MAX_PARAMS; count++){
    memset(current_command->command_parameters[count], 0, PARAM_LEN);
  }
  
}

// Initialize buffer
command * cp_buffer_init(void){

  // Create the structure
  command * current_command = new command;

  return current_command;
  
}

// Deletes buffer
void cp_buffer_deinit(command * current_command){
  
  delete current_command;
  
}

// Get next character and check for full command
bool cp_add_to_command_buffer(command * current_command, int received_data){

  // Get next character
  current_command->command_buffer[current_command->command_buffer_index] = (char)(received_data & 0xFF);
  current_command->command_buffer_index++;

  // Check for full command
  if(received_data == '\n'){
    return true;
  }
  return false;
  
}

// Parse the current command into structure variables
bool cp_parse_command(command * current_command){

  // Setup variable
  uint8_t temp_index = 0;
  uint16_t raw_command_index = 0;

  // Check for the opening brace
  if(current_command->command_buffer[raw_command_index] != '{') return false;
  raw_command_index++;

  // Get command type
  while(current_command->command_buffer[raw_command_index] != ':' && current_command->command_buffer[raw_command_index] != '}'){

    current_command->command_type[temp_index] = current_command->command_buffer[raw_command_index];
    temp_index++;
    raw_command_index++;
    
  }

  // Check for end
  if(current_command->command_buffer[raw_command_index] == '}'){

    raw_command_index++;
    if(current_command->command_buffer[raw_command_index] == '\n') return true;
    else return false;
    
  }

  // Move to parameters
  raw_command_index++;

  // Process all parameters
  for(uint16_t count = 0; count < MAX_PARAMS; count++){

    // Process parameter
    temp_index = 0;
    while((current_command->command_buffer[raw_command_index] != ',') 
           && (current_command->command_buffer[raw_command_index] != '}')
           && (current_command->command_buffer[raw_command_index] != '\n')){

      // Copy character
      current_command->command_parameters[count][temp_index] = current_command->command_buffer[raw_command_index];

      // Increment variables
      temp_index++;
      raw_command_index++;
      
    }

    // Check for end of command
    if(current_command->command_buffer[raw_command_index] == '}'){
      break;
    }
    raw_command_index++;
    
  }

  // Check for closing brace
  if(current_command->command_buffer[raw_command_index] != '}') return false;

  // Check for newline
  raw_command_index++;
  if(current_command->command_buffer[raw_command_index] != '\n') return false;

  // Success
  return true;

  /*
  char ** temp_return;
  uint16_t pixel_position = strtol(temp_buff, temp_return, 10);
  temp_index++;

  memset(temp_buff, 0, 16);
  uint8_t temp_index_2 = 0;
  while(command_buffer[temp_index] != '\n'){

    temp_buff[temp_index_2] = command_buffer[temp_index];
    temp_index++;
    temp_index_2++;
    
  }
  
  uint32_t pixel_color = strtol(temp_buff, temp_return, 16);

  lk_set_pixel_color(pixel_position, pixel_color);
  lk_update_pixels();
  */
  
}

// Selects command function from command structure
bool cp_process_command(command * current_command){

  // Variables
  char ** temp_return;

  // Find command
  if(strcmp("SET_ONE", current_command->command_type) == 0){

    
    set_one_neopixel(strtol(current_command->command_parameters[0], temp_return, 16), 
                     strtol(current_command->command_parameters[1], temp_return, 16));
    return true;
    
  }
  else if(strcmp("SET_MANY", current_command->command_type) == 0){

    if(!set_many_neopixels(strtol(current_command->command_parameters[0], temp_return, 16),
                       strtol(current_command->command_parameters[1], temp_return, 16), 
                       strtol(current_command->command_parameters[2], temp_return, 16))){

      return false;
      
    }
    
    return true;
    
  }
  else if(strcmp("DELAY", current_command->command_type) == 0){
    
    delay_millis(strtol(current_command->command_parameters[0], temp_return, 16));
    return true;
    
  }
  else if(strcmp("SET_MAX", current_command->command_type) == 0){
    
    set_max_pixels(strtol(current_command->command_parameters[0], temp_return, 16));
    return true;
    
  }
  else if(strcmp("INIT", current_command->command_type) == 0){
    
    neopixel_init(strtol(current_command->command_parameters[0], temp_return, 16),
                  strtol(current_command->command_parameters[1], temp_return, 16));
    return true;
    
  }
  else if(strcmp("UPDATE", current_command->command_type) == 0){

    update_neopixels();
    return true;
    
  }
  else if(strcmp("ENTER_FAST", current_command->command_type) == 0){

    fast_mode();
    return true;
    
  }

  // Failure
  return false;
  
}

// Fast mode for quick changes to neopixels
static void fast_mode(void){
  
  char fast_buffer[FAST_BUFFER_LEN];
  memset(fast_buffer, 0, FAST_BUFFER_LEN);

  Serial.write(COMMAND_RECEIVED);

  while(strcmp(fast_buffer, "EXIT\n") != 0){
    
    get_fast_command(fast_buffer);

    if(strcmp(fast_buffer, "U\n") == 0){
      
      lk_update_pixels();
      // Serial.write(FAST_RESPONSE);
      
    }
    else if(fast_buffer[0] == 'T'){
      
      char temp_buffer[FAST_BUFFER_LEN];
      char ** temp_return;
      uint16_t temp_index = 0;
      uint16_t raw_command_index = 0;
      memset(temp_buffer, 0, FAST_BUFFER_LEN);

      // Get command type
      while(fast_buffer[raw_command_index] != ':' && fast_buffer[raw_command_index] != '\n'){
    
        temp_index++;
        raw_command_index++;
        
      }

      // Validate
      if(fast_buffer[raw_command_index] == '\n'){
        
        // Serial.write(FAST_RESPONSE);
        continue;
        
      }
      raw_command_index++;

      memset(temp_buffer, 0, FAST_BUFFER_LEN);
      temp_index = 0;
      
      // Get command type
      while(fast_buffer[raw_command_index] != ':' && fast_buffer[raw_command_index] != '\n'){
    
        temp_buffer[temp_index] = fast_buffer[raw_command_index];
        temp_index++;
        raw_command_index++;
        
      }

      uint32_t time_delay = strtol(temp_buffer, temp_return, 16);

      delay_millis(time_delay);
      // Serial.write(FAST_RESPONSE);
      
    }
    else if(strcmp(fast_buffer, "EXIT\n") == 0) continue;
    else{
      
      char temp_buffer[FAST_BUFFER_LEN];
      char ** temp_return;
      uint16_t temp_index = 0;
      uint16_t raw_command_index = 0;
      memset(temp_buffer, 0, FAST_BUFFER_LEN);

      // Get command type
      while(fast_buffer[raw_command_index] != ':' && fast_buffer[raw_command_index] != '\n'){
    
        temp_buffer[temp_index] = fast_buffer[raw_command_index];
        temp_index++;
        raw_command_index++;
        
      }

      // Validate
      if(fast_buffer[raw_command_index] == '\n'){
        
        // Serial.write(FAST_RESPONSE);
        continue;
        
      }
      
      uint16_t index = strtol(temp_buffer, temp_return, 16);
      raw_command_index++;

      memset(temp_buffer, 0, FAST_BUFFER_LEN);
      temp_index = 0;
      
      // Get command type
      while(fast_buffer[raw_command_index] != ':' && fast_buffer[raw_command_index] != '\n'){
    
        temp_buffer[temp_index] = fast_buffer[raw_command_index];
        temp_index++;
        raw_command_index++;
        
      }

      uint32_t rgb = strtol(temp_buffer, temp_return, 16);

      set_one_neopixel(index, rgb);
      // Serial.write(FAST_RESPONSE);
      
    }
    
    memset(fast_buffer, 0, FAST_BUFFER_LEN);
    
  }
  
}

// Gets commands in fast mode
static void get_fast_command(char * fast_buffer){
  
  int temp = 0;
  uint16_t temp_index = 0;
  while(temp != '\n'){
    
      temp = Serial.read();

      if(temp <= 0) continue;

      // Validate buffer limit
      if(temp_index >= FAST_BUFFER_LEN) break;

      // Store in buffer
      fast_buffer[temp_index] = (char)(0xFF & temp);

      // Increment
      temp_index++;
    
  }  
  
}

// Initialize neopixels
static void neopixel_init(uint16_t num_pixels, uint32_t rgb_color){

  lk_init(num_pixels, rgb_color);
  // lk_update_pixels();
  
}

// Displays current pixels from memory buffer
static void update_neopixels(void){

  lk_update_pixels();
  
}

// Sets one neopixel to a color
static void set_one_neopixel(uint16_t neopixel_index, uint32_t rgb_color){

  lk_set_pixel_color(neopixel_index, rgb_color);
  // lk_update_pixels();
  
}

// Sets multiple neopixels to a color
static bool set_many_neopixels(uint16_t neopixel_index, uint32_t rgb_color, uint16_t num_pixels){

  if((neopixel_index + num_pixels) > lk_get_length()) return false;

  // Loop and set items in buffer
  for(uint16_t count = neopixel_index; count < (neopixel_index + num_pixels); count++){
    
    lk_set_pixel_color(count, rgb_color);
    
  }

  // Display new colors
  // lk_update_pixels();
  return true;
  
}

// It's a millisecond delay, what more do you want?
static void delay_millis(uint32_t millis_to_delay){

  delay(millis_to_delay);
  
}

static void set_max_pixels(uint16_t max_pixels){
  
  lk_set_num_pixels(max_pixels);
  
}
