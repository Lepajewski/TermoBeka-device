#ifndef LIB_SYSTEM_MANAGER_SYSTEM_MANAGER_H_
#define LIB_SYSTEM_MANAGER_SYSTEM_MANAGER_H_


#include "inttypes.h"
#include "esp_console.h"
#include "driver/uart.h"
#include "freertos/queue.h"


#define UART_BUF_SIZE       256
#define UART_QUEUE_SIZE     20


class SystemManager {
 private:
    QueueHandle_t event_queue_handle;
    size_t uart_buffered_size;
    uint8_t uart_rx_data[UART_BUF_SIZE];
    uart_port_t uart_num;
    const char *prompt_str;
    esp_console_config_t esp_console_config;


    void init_event_queue();
    void register_commands();
    void process_command(char *cmd);

 public:
    static const esp_console_cmd_t system_commands[];
    
    SystemManager();
    SystemManager(uart_port_t uart_num, const char *prompt_str);
    SystemManager(uart_port_t uart_num, const char *prompt_str, esp_console_config_t config);
    ~SystemManager();

    QueueHandle_t *get_event_queue();

    void poll_event();
    void configure_uart();
    void init_console();
    void deinit_console();

    
};


#endif  // LIB_SYSTEM_MANAGER_SYSTEM_MANAGER_H_
