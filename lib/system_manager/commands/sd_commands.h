#ifndef LIB_SYSTEM_MANAGER_COMMANDS_SD_COMMANDS_H_
#define LIB_SYSTEM_MANAGER_COMMANDS_SD_COMMANDS_H_


#include "esp_err.h"
#include "tb_event.h"


esp_err_t send_to_sd_queue(SDEvent *evt);
esp_err_t process_path_cmd(SDEventType e_type, const char *path);
void register_sd_card();


#endif  // LIB_SYSTEM_MANAGER_COMMANDS_SD_COMMANDS_H_
