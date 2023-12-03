#ifndef LIB_SYSTEM_MANAGER_COMMANDS_WIFI_COMMANDS_H_
#define LIB_SYSTEM_MANAGER_COMMANDS_WIFI_COMMANDS_H_


#include "esp_err.h"
#include "tb_event.h"


esp_err_t send_to_wifi_queue(WiFiEvent *evt);
esp_err_t process_wifi_credentials(WiFiEventType e_type, const char *ssid, const char *pass);
void register_wifi();


#endif  // LIB_SYSTEM_MANAGER_COMMANDS_WIFI_COMMANDS_H_
