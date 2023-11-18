#ifndef LIB_SYSTEM_MANAGER_SYSTEM_MANAGER_H_
#define LIB_SYSTEM_MANAGER_SYSTEM_MANAGER_H_


#include "inttypes.h"
#include "driver/uart.h"
#include "esp_console.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"


#define UART_BUF_SIZE       256

class SystemManager;

SystemManager *get_system_manager();


class SystemManager {
 private:
    QueueHandle_t event_queue_handle;
    QueueHandle_t ui_queue_handle;
    QueueHandle_t sd_queue_handle;

    uart_port_t uart_num;
    const char *prompt_str;
    esp_console_config_t esp_console_config;

    void init_queues();
    void register_commands();
    void process_command(char *cmd);

 public:    
    SystemManager();
    SystemManager(uart_port_t uart_num, const char *prompt_str);
    SystemManager(uart_port_t uart_num, const char *prompt_str, esp_console_config_t config);
    ~SystemManager();

    QueueHandle_t *get_event_queue();
    QueueHandle_t *get_ui_queue();
    QueueHandle_t *get_sd_queue();

    void poll_event();
    void init_console();
    void deinit_console();
};


#endif  // LIB_SYSTEM_MANAGER_SYSTEM_MANAGER_H_
