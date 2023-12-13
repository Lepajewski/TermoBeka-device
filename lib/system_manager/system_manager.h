#ifndef LIB_SYSTEM_MANAGER_SYSTEM_MANAGER_H_
#define LIB_SYSTEM_MANAGER_SYSTEM_MANAGER_H_


#include "inttypes.h"
#include "driver/uart.h"
#include "esp_console.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/ringbuf.h"

#include "tb_event.h"
#include "nvs_manager.h"


#define UART_BUF_SIZE       256


class SystemManager;

SystemManager *get_system_manager();


class SystemManager {
 private:
    QueueHandle_t event_queue_handle;
    QueueHandle_t ui_queue_handle;
    QueueHandle_t sd_queue_handle;
    QueueHandle_t wifi_queue_handle;
    QueueHandle_t server_queue_handle;
    QueueHandle_t profile_queue_handle;

    RingbufHandle_t sd_ring_buf_handle;

    uart_port_t uart_num;
    const char *prompt_str;
    esp_console_config_t esp_console_config;

    NVSManager nvs_manager;
    
    void setup_logger();
    void init_queues();
    void init_ring_buffers();
    void register_commands();

    void process_wifi_connected();
    void process_wifi_disconnected();
    void process_wifi_got_time();
    void process_server_connected();
    void process_server_disconnected();
    void process_ui_button_press(uint8_t num);
    void process_sd_mounted();
    void process_sd_unmounted();
    void process_sd_config_load(EventSDConfigLoad *payload);
    void process_sd_load_ca_file();
    void process_profile_response(EventProfileResponse *payload);
    void process_profile_update(EventProfileUpdate *payload);
    void process_command(char *cmd);

    esp_err_t send_connect_wifi();
    esp_err_t send_disconnect_wifi();
    esp_err_t send_connect_server();
    esp_err_t send_disconnect_server();
    esp_err_t send_mount_card();
    esp_err_t send_unmount_card();
    esp_err_t send_load_ca_cert();

    const char *get_wifi_ssid();
    const char *get_wifi_pass();

 public:    
    SystemManager();
    SystemManager(uart_port_t uart_num, const char *prompt_str);
    SystemManager(uart_port_t uart_num, const char *prompt_str, esp_console_config_t config);
    ~SystemManager();

    void begin();

    QueueHandle_t *get_event_queue();
    QueueHandle_t *get_ui_queue();
    QueueHandle_t *get_sd_queue();
    QueueHandle_t *get_wifi_queue();
    QueueHandle_t *get_server_queue();
    QueueHandle_t *get_profile_queue();

    RingbufHandle_t *get_sd_ring_buf();

    void poll_event();
    void init_console();
    void deinit_console();
};


#endif  // LIB_SYSTEM_MANAGER_SYSTEM_MANAGER_H_
