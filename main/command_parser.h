#ifndef COMMAND_PARSER_H
#define COMMAND_PARSER_H

#define COMMAND_BUF_LEN   64
#define MAX_PARAMS        4
#define PARAM_LEN         16
#define COMMAND_TYPE_LEN  PARAM_LEN
#define TEMP_BUF_SIZE[PARAM_LEN + COMMAND_TYPE_LEN]
#define FAST_BUFFER_LEN   16

#define FAST_RESPONSE     "K\n"
#define COMMAND_RECEIVED  "COMMAND_RECEIVED\n"
#define COMMAND_INVALID   "COMMAND_INVALID\n"

typedef struct {
  
  char command_buffer[COMMAND_BUF_LEN];
  char command_type[COMMAND_TYPE_LEN];
  char command_parameters[MAX_PARAMS][PARAM_LEN];
  uint16_t command_buffer_index;
  bool command_received;

} command;

void cp_reset_command_buffer(command * current_command);
bool cp_add_to_command_buffer(command * current_command, int received_data);
bool cp_parse_command(command * current_command);
bool cp_process_command(command * current_command);
command * cp_buffer_init(void);
void cp_buffer_deinit(command * current_command);

#endif
