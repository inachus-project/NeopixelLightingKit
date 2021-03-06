#include <SPI.h>
#include <SdFat.h>
#include "lighting_kit.h"
#include "command_parser.h"

// Neopixel lighting kit

// Constants
#define test_length 30
#define READY_RESPONSE    "READY\n"
#define DEBUGGING         false
#define TIME_TO_SERIAL    5000
#define SD_FAT_TYPE       3 // exFat
#define SD_FILE_NAME      "playback.txt"
#define DELAY_COMMAND     "{DELAY:"

// Global variables
command * current_command;
bool running_from_serial = true;
File myFile;
SdFs sd;
const int chipSelect = SS;
elapsedMillis write_delay = 0;

// Function declarations
void open_SD_card(void);
int read_file(void);

// Setup
void setup() {

  // Start serial port
  elapsedMillis serial_start_time = 0;
  Serial.begin(115200);
  while(!Serial){

    // Check for the timeout
    if(serial_start_time > TIME_TO_SERIAL){
      
      running_from_serial = false;
      break;
      
    }
    
  }

  // Send response if necessary
  if(running_from_serial){

    open_SD_card(false);
    Serial.write(READY_RESPONSE);
    
  }
  else{
    
    open_SD_card(true);  
    
  }
  

  current_command = cp_buffer_init();
  cp_reset_command_buffer(current_command);

}

// Main loop
void loop() {

  // Read data from serial port
  int received_data;
  if (running_from_serial){
    
    received_data = Serial.read();
    
  }
  else{
    
    received_data = read_file();
    
  }

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

        if(running_from_serial){
          Serial.write(COMMAND_INVALID);
        }
        
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

          // Check if the command should be written to the file
          if(running_from_serial && (0 != strcmp("{ENTER_FAST}\n", current_command->command_buffer))){
            write_command_to_file(current_command->command_buffer);
          }

          // Check if we should respond
          if(running_from_serial){
            Serial.write(COMMAND_RECEIVED);
          }
          
        }
        else {
          if(running_from_serial){
            Serial.write(COMMAND_INVALID);
          }
        }
        
      }

      // Reset command
      cp_reset_command_buffer(current_command);
    }
    
    
  }

}

// Writes a command to the file
void write_command_to_file(char * new_command){

  // Make time delay command
  // Create buffers
  char delay_command[COMMAND_BUF_LEN];
  memset(delay_command, 0, COMMAND_BUF_LEN);
  char number[16];
  memset(number, 0, 16);

  // Create command
  strcpy(delay_command, DELAY_COMMAND);
  strcat(delay_command, itoa(write_delay, number, 16));
  strcat(delay_command, "}\n");
  write_delay = 0;

  // Write commands
  myFile.print(delay_command);
  myFile.print(new_command);
  myFile.flush();
  
}

// Gets the next character from the SD card
// Returns -1 if not available
int read_file(void){

  // Get character
  if (!myFile.available()){

    // Reopen file
    myFile.close();
    myFile.open(SD_FILE_NAME, FILE_READ);

    // Check for empty file
    if (!myFile.available()){
      
      while(true);  
      
    }

  }

  // Get character
  return myFile.read();
  
}

// Sets up the SD card for reading
void open_SD_card(bool reading){

  // Attempt to open the SD card
  if (!sd.begin(SdioConfig(FIFO_SDIO))){
    
    while(true);
    
  }

  // Open the file
  if (reading){
    myFile = sd.open(SD_FILE_NAME, FILE_READ);
    
  }
  else{
    sd.remove(SD_FILE_NAME);
    myFile = sd.open(SD_FILE_NAME, FILE_WRITE);
    
  }

  if (!myFile){
    
    while(true);  
    
  }

  // Check for empty file
  if (!myFile.available() && reading){
    
    while(true);  
    
  }
  
}
