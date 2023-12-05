#include <unistd.h>
#include "esp_vfs_dev.h"
#include "driver/uart.h"
#include "linenoise/linenoise.h"
#include "argtable3/argtable3.h"
#include "esp_intr_alloc.h"

#include "logger.h"
#include "tb_event.h"
#include "console_task.h"
#include "commands/wifi_commands.h"
#include "commands/server_commands.h"
#include "commands/commands.h"
#include "system_manager.h"


const char * const TAG = "SysMgr";


static SystemManager sysMgr;

SystemManager *get_system_manager() {
   return &sysMgr;
}


SystemManager::SystemManager() {
#ifdef CONFIG_ESP_CONSOLE_UART_NUM
    this->uart_num = CONFIG_ESP_CONSOLE_UART_NUM;
#endif

    this->prompt_str = "esp $ ";

    this->esp_console_config = {
        .max_cmdline_length = UART_BUF_SIZE,
        .max_cmdline_args = 8,
#if CONFIG_LOG_COLORS
        .hint_color = atoi(LOG_COLOR_CYAN),
#endif
        .hint_bold = 0,
    };

    init_queues();
}

SystemManager::SystemManager(uart_port_t uart_num, const char *prompt_str) {
    this->uart_num = uart_num;
    this->prompt_str = prompt_str;
    this->esp_console_config = {
        .max_cmdline_length = UART_BUF_SIZE,
        .max_cmdline_args = 8,
#if CONFIG_LOG_COLORS
        .hint_color = atoi(LOG_COLOR_CYAN),
#endif
        .hint_bold = 0,
    };

    init_queues();
}

SystemManager::SystemManager(uart_port_t uart_num, const char *prompt_str, esp_console_config_t config) {
    this->uart_num = uart_num;
    this->prompt_str = prompt_str;
    this->esp_console_config = config;

    init_queues();
}

SystemManager::~SystemManager() {
    deinit_console();
}

void SystemManager::begin() {
    this->nvs_manager.begin();

    this->nvs_manager.load_default_config();
    this->nvs_manager.load_config();

    nvs_device_config_t *default_config = this->nvs_manager.get_default_config();
    nvs_device_config_t *config = this->nvs_manager.get_config();

    TB_LOGI(TAG, "DEFAULT CONFIG: | %s %s %s %s %s %d |", 
        default_config->wifi_ssid,
        default_config->wifi_pass,
        default_config->mqtt_broker_uri,
        default_config->mqtt_username,
        default_config->mqtt_password,
        default_config->log_level
    );
    TB_LOGI(TAG, "CONFIG: | %s %s %s %s %s %d |",
        config->wifi_ssid,
        config->wifi_pass,
        default_config->mqtt_broker_uri,
        default_config->mqtt_username,
        default_config->mqtt_password,
        config->log_level
    );

    this->setup_logger();
    this->init_console();
    if (this->send_connect_wifi() != ESP_OK) {
        TB_LOGE(TAG, "fail to send wifi connect evt");
    }
}

void SystemManager::setup_logger() {
    esp_log_level_set("*", this->nvs_manager.get_config()->log_level);
}

void SystemManager::init_queues() {
    this->event_queue_handle = xQueueCreate(EVENT_QUEUE_SIZE, sizeof(Event));
    this->ui_queue_handle = xQueueCreate(UI_QUEUE_SIZE, sizeof(UIEvent));
    this->sd_queue_handle = xQueueCreate(SD_QUEUE_SIZE, sizeof(SDEvent));
    this->wifi_queue_handle = xQueueCreate(WIFI_QUEUE_SIZE, sizeof(WiFiEvent));
    this->server_queue_handle = xQueueCreate(SERVER_QUEUE_SIZE, sizeof(ServerEvent));
    this->profile_queue_handle = xQueueCreate(PROFILE_QUEUE_SIZE, sizeof(ProfileEvent));

    if (this->event_queue_handle == NULL ||
        this->ui_queue_handle == NULL ||
        this->sd_queue_handle == NULL ||
        this->wifi_queue_handle == NULL ||
        this->server_queue_handle == NULL ||
        this->profile_queue_handle == NULL) {
        TB_LOGE(TAG, "queues init fail. Restarting...");
        vTaskDelay(pdMS_TO_TICKS(2000));
        fflush(stdout);
        esp_restart();
    }
}

void SystemManager::init_console() {
    /* Drain stdout before reconfiguring it */
    fflush(stdout);
    fsync(fileno(stdout));

    /* Disable buffering on stdin */
    setvbuf(stdin, NULL, _IONBF, 0);

    /* Minicom, screen, idf_monitor send CR when ENTER key is pressed */
    esp_vfs_dev_uart_port_set_rx_line_endings(this->uart_num, ESP_LINE_ENDINGS_CR);
    /* Move the caret to the beginning of the next line on '\n' */
    esp_vfs_dev_uart_port_set_tx_line_endings(this->uart_num, ESP_LINE_ENDINGS_CRLF);

    /* Configure UART. Note that REF_TICK is used so that the baud rate remains
     * correct while APB frequency is changing in light sleep mode.
     */
    const uart_config_t uart_config = {
        .baud_rate = CONFIG_ESP_CONSOLE_UART_BAUDRATE,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .rx_flow_ctrl_thresh = 0,
#if SOC_UART_SUPPORT_REF_TICK
        .source_clk = UART_SCLK_REF_TICK,
#elif SOC_UART_SUPPORT_XTAL_CLK
        .source_clk = UART_SCLK_XTAL,
#endif
    };

    /* Install UART driver for interrupt-driven reads and writes */
    ESP_ERROR_CHECK(uart_driver_install(CONFIG_ESP_CONSOLE_UART_NUM, UART_BUF_SIZE, 0, 0, NULL, 0));
    ESP_ERROR_CHECK(uart_param_config(CONFIG_ESP_CONSOLE_UART_NUM, &uart_config));
    
    /* Tell VFS to use UART driver */
    esp_vfs_dev_uart_use_driver(CONFIG_ESP_CONSOLE_UART_NUM);

    ESP_ERROR_CHECK(esp_console_init(&this->esp_console_config));

    /* Configure linenoise line completion library */
    /* Enable multiline editing. If not set, long commands will scroll within
     * single line.
     */
    linenoiseSetMultiLine(1);

    /* Tell linenoise where to get command completions and hints */
    linenoiseSetCompletionCallback(&esp_console_get_completion);
    linenoiseSetHintsCallback((linenoiseHintsCallback*) &esp_console_get_hint);

    /* Set command history size */
    linenoiseHistorySetMaxLen(100);

    /* Set command maximum length */
    linenoiseSetMaxLineLen(this->esp_console_config.max_cmdline_length);

    /* Don't return empty lines */
    linenoiseAllowEmpty(false);

    register_commands();

    TB_LOGI(TAG, "init console");
    xTaskCreate(consoleTask, "ConsoleTask", 4096, (void *) this->prompt_str, 1, NULL);
}

void SystemManager::deinit_console() {
    TB_LOGI(TAG, "terminating console");
    esp_console_deinit();
}

void SystemManager::poll_event() {
    Event evt;

    if (xQueueReceive(this->event_queue_handle, &evt, portMAX_DELAY) == pdPASS) {
        TB_LOGI(TAG, "new event: %s, type: %s", event_origin_to_s(evt.origin), event_type_to_s(evt.type));
        TB_LOGI(TAG, "new event: %d, type: %d", evt.origin, evt.type);

        switch (evt.type) {
            case EventType::WIFI_CONNECTED:
            {
                TB_LOGI(TAG, "wifi connected");
                if (this->send_connect_mqtt() != ESP_OK) {
                    TB_LOGE(TAG, "fail to send MQTT connect");
                }
                break;
            }
            case EventType::WIFI_DISCONNECTED:
            {
                TB_LOGI(TAG, "wifi disconnected");
                if (this->send_disconnect_mqtt() != ESP_OK) {
                    TB_LOGE(TAG, "fail to send MQTT disconnect");
                }
                break;
            }
            case EventType::WIFI_GOT_TIME:
            {
                TB_LOGI(TAG, "ntp got time");
                break;
            }
            case EventType::CONSOLE_COMMAND:
            {
                TB_LOGI(TAG, "command: %s", reinterpret_cast<char *>(evt.payload));
                process_command(reinterpret_cast<char *>(evt.payload));
                break;
            }
            case EventType::UI_BUTTON_PRESS:
            {
                TB_LOGI(TAG, "button: %u", evt.payload[0]);
                break;
            }
            case EventType::SD_CONFIG_LOAD:
            {
                EventSDConfigLoad *payload = reinterpret_cast<EventSDConfigLoad*>(evt.payload);
                TB_LOGI(TAG, "CONFIG: | %s %s %s %s %s %d |", 
                    payload->config.wifi_ssid,
                    payload->config.wifi_pass,
                    payload->config.mqtt_broker_uri,
                    payload->config.mqtt_username,
                    payload->config.mqtt_password,
                    payload->config.log_level
                );
                esp_err_t config_changed = this->nvs_manager.save_config(&payload->config);
                if (config_changed == ESP_OK) {
                    TB_LOGI(TAG, "sending new config to wifi");
                    if (this->send_connect_wifi() != ESP_OK) {
                        TB_LOGE(TAG, "fail to send wifi connect evt");
                    }
                } else {
                    TB_LOGW(TAG, "config not changed");
                }
                break;
            }
            case EventType::PROFILE_RESPONSE:
            {
                EventProfileResponse *payload = reinterpret_cast<EventProfileResponse*>(evt.payload);
                TB_LOGI(TAG, "profile response: %d", payload->response);
                break;
            }
            case EventType::PROFILE_UPDATE:
            {
                EventProfileUpdate *payload = reinterpret_cast<EventProfileUpdate*>(evt.payload);
                printf("Status: %d\n", payload->info.status);
                printf("Current Duration: %u ms\n", payload->info.current_duration);
                printf("Total Duration: %u ms\n", payload->info.total_duration);
                printf("Step Start Time: %u ms\n", payload->info.step_start_time);
                printf("Step Stopped Time: %u ms\n", payload->info.step_stopped_time);
                printf("Profile Stopped Time: %u ms\n", payload->info.profile_stopped_time);
                printf("Step End Time: %u ms\n", payload->info.step_end_time);
                printf("Step Time Left: %u ms\n", payload->info.step_time_left);
                printf("Profile Time Halted: %" PRIu32 " ms\n", payload->info.profile_time_halted);
                printf("Profile Time Left: %u ms\n", payload->info.profile_time_left);
                printf("Current Temperature: %d\n", payload->info.current_temperature);
                printf("Progress: %.2lf%\n", payload->info.progress_percent);

                ServerEvent evt;
                evt.type = ServerEventType::PUBLISH_PROFILE_UPDATE;
                memcpy(&evt.payload, payload->buffer, SERVER_QUEUE_MAX_PAYLOAD);
                send_to_server_queue(&evt);

                break;
            }
            case EventType::UNKNOWN:
            default:
            {
                TB_LOGI(TAG, "unknown event");
                break;
            }
        }
    }
}

void SystemManager::process_command(char *cmd) {
    if (strlen(cmd) > 0) {
        linenoiseHistoryAdd(cmd);
    }

    int ret;
    esp_err_t err = esp_console_run(cmd, &ret);

    if (err == ESP_ERR_NOT_FOUND) {
        TB_NAK(TAG, "Unrecognized command\n");
    } else if (err == ESP_ERR_INVALID_ARG) {
        // command was empty
    } else if (err == ESP_OK && ret != ESP_OK) {
        TB_LOGI(TAG, "Command returned non-zero error code: 0x%x (%s)\n", ret, esp_err_to_name(ret));
    } else if (err != ESP_OK) {
        TB_LOGI(TAG, "Internal error: %s\n", esp_err_to_name(err));
    }
}

esp_err_t SystemManager::send_connect_wifi() {
    esp_err_t err = ESP_OK;
    if ((err = this->send_disconnect_wifi()) != ESP_OK) {
        return err;
    }

    const char *ssid = this->nvs_manager.get_config()->wifi_ssid;
    const char *pass = this->nvs_manager.get_config()->wifi_pass;

    return process_wifi_credentials(WiFiEventType::CONNECT, ssid, pass);
}

esp_err_t SystemManager::send_disconnect_wifi() {
    WiFiEvent evt;
    evt.type = WiFiEventType::DISCONNECT;

    return send_to_wifi_queue(&evt);
}

esp_err_t SystemManager::send_connect_mqtt() {
    esp_err_t err = ESP_OK;
    if ((err = this->send_disconnect_mqtt()) != ESP_OK) {
        return err;
    }

    const char *uri = this->nvs_manager.get_config()->mqtt_broker_uri;
    const char *username = this->nvs_manager.get_config()->mqtt_username;
    const char *password = this->nvs_manager.get_config()->mqtt_password;

    return process_server_credentials(ServerEventType::CONNECT, uri, username, password);
}

esp_err_t SystemManager::send_disconnect_mqtt() {
    ServerEvent evt;
    evt.type = ServerEventType::DISCONNECT;

    return send_to_server_queue(&evt);
}

QueueHandle_t *SystemManager::get_event_queue() {
    return &this->event_queue_handle;
}

QueueHandle_t *SystemManager::get_ui_queue() {
    return &this->ui_queue_handle;
}

QueueHandle_t *SystemManager::get_sd_queue() {
    return &this->sd_queue_handle;
}

QueueHandle_t *SystemManager::get_wifi_queue() {
    return &this->wifi_queue_handle;
}

QueueHandle_t *SystemManager::get_server_queue() {
    return &this->server_queue_handle;
}

QueueHandle_t *SystemManager::get_profile_queue() {
    return &this->profile_queue_handle;
}

const char *SystemManager::get_wifi_ssid() {
    return this->nvs_manager.get_config()->wifi_ssid;
}

const char *SystemManager::get_wifi_pass() {
    return this->nvs_manager.get_config()->wifi_pass;
}

void SystemManager::register_commands() {
    esp_console_register_help_command();

    register_system_commands();
}
