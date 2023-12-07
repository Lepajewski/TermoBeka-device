#ifndef LIB_SYSTEM_MANAGER_COMMANDS_UI_COMMANDS_H_
#define LIB_SYSTEM_MANAGER_COMMANDS_UI_COMMANDS_H_

#include "tb_event.h"

esp_err_t send_to_ui_queue(UIEvent *evt);
void register_user_interface();


#endif  // LIB_SYSTEM_MANAGER_COMMANDS_UI_COMMANDS_H_
