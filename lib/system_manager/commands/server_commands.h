#ifndef LIB_SYSTEM_MANAGER_SERVER_COMMANDS_H_
#define LIB_SYSTEM_MANAGER_SERVER_COMMANDS_H_


esp_err_t send_to_server_queue(ServerEvent *evt);
esp_err_t process_server_credentials(ServerEventType e_type, const char *uri);
void register_server();


#endif  // LIB_SYSTEM_MANAGER_SERVER_COMMANDS_H_