// Neopixel lighting something

#define test_length 30
#define COMMAND_RECEIVED  "COMMAND_RECEIVED\n"
#define COMMAND_INVALID   "COMMAND_INVALID\n"
#define READY_RESPONSE    "READY\n"
#define DEBUGGING         false

#include "lighting_kit.h"
#include "command_parser.h"

command * current_command;

// Setup
void setup() {

  // Start serial port
  Serial.begin(115200);
  Serial.write(READY_RESPONSE);

  current_command = cp_buffer_init();
  cp_reset_command_buffer(current_command);

}

// Main loop
void loop() {

  // Read data from serial port
  int received_data = Serial.read();

  // Check if data is received
  if(received_data >= 0){

    // Send back received character
#if DEBUGGING
    Serial.write(received_data);
#endif

    // Process character
    if(cp_add_to_command_buffer(current_command, received_data)){

      // Parse command
      if(!cp_parse_command(current_command)){

        Serial.write(COMMAND_INVALID);
          
      }
      else{
#if DEBUGGING
        Serial.write(current_command->command_type);
        Serial.write("\n");
        Serial.write(current_command->command_parameters[0]);
        Serial.write("\n");
        Serial.write(current_command->command_parameters[1]);
        Serial.write("\n");
        Serial.write(current_command->command_parameters[2]);
        Serial.write("\n");
        Serial.write(current_command->command_parameters[3]);
        Serial.write("\n");
#endif

        if(cp_process_command(current_command)){

          Serial.write(COMMAND_RECEIVED);
          
        }
        else Serial.write(COMMAND_INVALID);
        
      }

      // Reset command
      cp_reset_command_buffer(current_command);
    }
    
    
  }
  /*
  for(uint16_t count = current_length - 1; count > 0; count--){

    strip_buffer[count] = strip_buffer[count - 1];
    
  }

  // Changing color test
  if(strip_buffer[0] == RED){
    strip_buffer[0] = GREEN;
  }
  else if(strip_buffer[0] == GREEN){
    strip_buffer[0] = BLUE;
  }
  else{
    strip_buffer[0] = RED;
  }
  */

  // Update
  

  // Delay for human eyes
  // delay(500);

}
