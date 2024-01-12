#include <unistd.h>
#include "esp_vfs_dev.h"
#include "driver/uart.h"
#include <driver/spi_common.h>
#include <driver/spi_master.h>
#include "linenoise/linenoise.h"
#include "argtable3/argtable3.h"
#include "esp_intr_alloc.h"

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/ringbuf.h"

#include "logger.h"
#include "tb_event.h"
#include "console_task.h"
#include "commands/sd_commands.h"
#include "commands/wifi_commands.h"
#include "commands/server_commands.h"
#include "commands/ui_commands.h"
#include "commands/commands.h"
#include "commands/profile_commands.h"
#include "system_manager.h"
#include "profile_type.h"


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

    this->init_queues();
    this->init_ring_buffers();
    this->init_spi_semaphore();
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
    
    this->init_queues();
    this->init_ring_buffers();
    this->init_spi_semaphore();
}

SystemManager::SystemManager(uart_port_t uart_num, const char *prompt_str, esp_console_config_t config) {
    this->uart_num = uart_num;
    this->prompt_str = prompt_str;
    this->esp_console_config = config;

    this->init_queues();
    this->init_ring_buffers();
    this->init_spi_semaphore();
}

SystemManager::~SystemManager() {
    this->deinit_console();
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

    if (this->send_mount_card() != ESP_OK) {
        TB_LOGE(TAG, "fail to send mount card evt");
    }

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
    this->regulator_queue_handle = xQueueCreate(REGULATOR_QUEUE_SIZE, sizeof(RegulatorEvent));

    if (this->event_queue_handle == NULL ||
        this->ui_queue_handle == NULL ||
        this->sd_queue_handle == NULL ||
        this->wifi_queue_handle == NULL ||
        this->server_queue_handle == NULL ||
        this->profile_queue_handle == NULL ||
        this->regulator_queue_handle == NULL) {
        TB_LOGE(TAG, "queues init fail. Restarting...");
        vTaskDelay(pdMS_TO_TICKS(2000));
        fflush(stdout);
        esp_restart();
    }
}

void SystemManager::init_ring_buffers() {
    this->sd_ring_buf_handle = xRingbufferCreate(SD_CARD_DATA_RING_BUFFER_LEN, RINGBUF_TYPE_NOSPLIT);

    if (this->sd_ring_buf_handle == NULL) {
        TB_LOGE(TAG, "ring buffers init fail. Restarting...");
        vTaskDelay(pdMS_TO_TICKS(2000));
        fflush(stdout);
        esp_restart();
    }
}

void SystemManager::init_spi_semaphore() {
    this->spi_semaphore = xSemaphoreCreateMutex();

    if (this->spi_semaphore == NULL) {
        TB_LOGE(TAG, "spi semaphore init fail. Restarting...");
        vTaskDelay(pdMS_TO_TICKS(2000));
        fflush(stdout);
        esp_restart();
    }

    xSemaphoreTake(this->spi_semaphore, portMAX_DELAY);

    spi_bus_config_t busConfig = {};
    busConfig.miso_io_num = PIN_SPI_MISO;
    busConfig.mosi_io_num = PIN_SPI_MOSI;
    busConfig.sclk_io_num = PIN_SPI_CLK;
    busConfig.quadhd_io_num = -1;
    busConfig.quadwp_io_num = -1;

    esp_err_t err = spi_bus_initialize(SPI3_HOST, &busConfig, 0);
    if (err == ESP_ERR_INVALID_STATE) {
        TB_LOGD(TAG, "SPI bus already initialized");
    } else if (err != ESP_OK) {
        TB_LOGE(TAG, "Error initialising SPI bus: %s, restarting...", esp_err_to_name(err));
        vTaskDelay(pdMS_TO_TICKS(2000));
        fflush(stdout);
        esp_restart();
    }

    xSemaphoreGive(this->spi_semaphore);
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
                this->process_wifi_connected();
                break;
            }
            case EventType::WIFI_DISCONNECTED:
            {
                this->process_wifi_disconnected();
                break;
            }
            case EventType::WIFI_GOT_TIME:
            {
                this->process_wifi_got_time();
                break;
            }
            case EventType::SERVER_CONNECTED:
            {
                this->process_server_connected();
                break;
            }
            case EventType::SERVER_DISCONNECTED:
            {
                this->process_server_disconnected();
                break;
            }
            case EventType::SERVER_PROFILE_LOAD:
            {
                this->process_server_profile_load(reinterpret_cast<EventServerProfileLoad*>(evt.payload));
                break;
            }
            case EventType::SERVER_PROFILE_START:
            {
                this->process_server_profile_start();
                break;
            }
            case EventType::SERVER_PROFILE_STOP:
            {
                this->process_server_profile_stop();
                break;
            }
            case EventType::WIFI_STRENGTH:
            {
                int rssi;
                memcpy(&rssi, evt.payload, sizeof(int));
                
                UIEvent event = {};
                event.type = UIEventType::WIFI_STRENGTH;
                memcpy(event.payload, &rssi, sizeof(int));

                send_to_ui_queue(&event);

                break;
            }
            case EventType::UI_BUTTON_PRESS:
            {
                this->process_ui_button_press(evt.payload[0]);
                break;
            }
            case EventType::SD_MOUNTED:
            {
                this->process_sd_mounted();
                break;
            }
            case EventType::SD_UNMOUNTED:
            {
                this->process_sd_unmounted();
                break;
            }
            case EventType::SD_CONFIG_LOAD:
            {
                this->process_sd_config_load(reinterpret_cast<EventSDConfigLoad*>(evt.payload));
                break;
            }
            case EventType::SD_LOAD_CA_FILE:
            {
                this->process_sd_load_ca_file();
                break;
            }
            case EventType::SD_PROFILE_LOAD:
            {
                this->process_sd_profile_load();
                break;
            }
            case EventType::UI_PROFILES_LOAD:
            {
                if (evt.origin == EventOrigin::UI) {
                    this->process_ui_to_sd_profiles_load(evt.payload);
                }
                else if (evt.origin == EventOrigin::SD) {
                    this->process_sd_to_ui_profiles_load();
                }
                break;
            }
            case EventType::UI_PROFILE_CHOSEN:
            {
                this->process_ui_profile_chosen(evt.payload);
                break;
            }
            case EventType::UI_PROFILE_START: 
            {
                this->process_ui_profile_start();
                break;
            }
            case EventType::UI_PROFILE_STOP:
            {
                this->process_ui_profile_stop();
                break;
            }
            case EventType::PROFILE_END:
            {
                this->process_profile_end();
                break;
            }
            case EventType::PROFILE_RESPONSE:
            {
                EventProfileResponse *payload = reinterpret_cast<EventProfileResponse*>(evt.payload);
                this->process_profile_response(payload);
                break;
            }
            case EventType::NEW_PROFILE_INFO:
            {
                EventNewProfileInfo *payload = reinterpret_cast<EventNewProfileInfo*>(evt.payload);
                this->process_new_profile_info(payload);
                break;
            }
            case EventType::REGULATOR_UPDATE:
            {
                this->process_regulator_update(reinterpret_cast<EventRegulatorUpdate*>(evt.payload));
                break;
            }
            case EventType::CONSOLE_COMMAND:
            {
                this->process_command(reinterpret_cast<char *>(evt.payload));
                break;
            }
            case EventType::NONE:
            default:
            {
                TB_LOGI(TAG, "unknown event");
                break;
            }
        }
    }
}

void SystemManager::process_wifi_connected() {
    TB_LOGI(TAG, "wifi connected");
    if (this->send_connect_server() != ESP_OK) {
        TB_LOGE(TAG, "fail to send server connect");
    }
}

void SystemManager::process_wifi_disconnected() {
    TB_LOGI(TAG, "wifi disconnected");
    if (this->send_disconnect_server() != ESP_OK) {
        TB_LOGE(TAG, "fail to send server disconnect");
    }
}

void SystemManager::process_wifi_got_time() {
    TB_LOGI(TAG, "ntp got time");
}

void SystemManager::process_server_connected() {
    TB_LOGI(TAG, "server connected");
}

void SystemManager::process_server_profile_load(EventServerProfileLoad *payload) {
    ProfileEvent evt = {};
    evt.type = ProfileEventType::NEW_PROFILE;
    memcpy(evt.payload, payload->buffer, PROFILE_QUEUE_MAX_PAYLOAD);

    if (send_to_profile_queue(&evt) != ESP_OK) {
        TB_LOGE(TAG, "fail to send new profile evt");
    }
}

void SystemManager::process_server_profile_start() {
    ProfileEvent evt = {};
    evt.type = ProfileEventType::START;

    if (send_to_profile_queue(&evt) != ESP_OK) {
        TB_LOGE(TAG, "fail to send start profile evt");
    }
}

void SystemManager::process_server_profile_stop() {
    ProfileEvent evt = {};
    evt.type = ProfileEventType::END;

    if (send_to_profile_queue(&evt) != ESP_OK) {
        TB_LOGE(TAG, "fail to send end profile evt");
    }
}


void SystemManager::process_server_disconnected() {
    TB_LOGI(TAG, "server disconnected");
}

void SystemManager::process_ui_button_press(uint8_t num) {
    TB_LOGI(TAG, "button: %" PRIu8 " press\n", num);
}

void SystemManager::process_ui_profile_chosen(uint8_t *payload) {
    SDEvent evt = {};
    evt.type = SDEventType::CAT_PROFILE;
    memcpy(evt.payload, payload, SD_QUEUE_MAX_PAYLOAD);
    if (send_to_sd_queue(&evt) != ESP_OK) {
        TB_LOGE(TAG, "fail to send cat profile evt");
    }
}

void SystemManager::process_sd_mounted() {
    TB_LOGI(TAG, "SD mounted");
    if (this->send_load_ca_cert() != ESP_OK) {
        TB_LOGE(TAG, "fail to process sd mounted");
    }
}

void SystemManager::process_sd_unmounted() {
    TB_LOGI(TAG, "SD unmounted");
}

void SystemManager::process_sd_profile_load() {
    RingbufHandle_t *ring_buf = this->get_sd_ring_buf();

    size_t len;
    char *ls_profiles = (char*) xRingbufferReceive(*ring_buf, &len, pdMS_TO_TICKS(10));
    if (ls_profiles != NULL) {
        std::string profile_str = std::string(ls_profiles, len);
        vRingbufferReturnItem(*ring_buf, (void*) ls_profiles);

        ProfileEventNewProfile arg = {};
        arg.profile = string_to_profile(profile_str);

        ProfileEvent evt = {};
        evt.type = ProfileEventType::NEW_PROFILE;
        memcpy(evt.payload, arg.buffer, PROFILE_QUEUE_MAX_PAYLOAD);

        if (send_to_profile_queue(&evt) != ESP_OK) {
            TB_LOGE(TAG, "fail to send new profile evt");
        }
    }
}

void SystemManager::process_sd_config_load(EventSDConfigLoad *payload) {
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
        if (this->send_connect_wifi() != ESP_OK) {
            TB_LOGE(TAG, "fail to send wifi connect evt");
        }
    } else {
        TB_LOGW(TAG, "config not changed");
    }
}

void SystemManager::process_sd_load_ca_file() {
    ServerEvent evt;
    evt.type = ServerEventType::READ_CA_FILE;
    if (send_to_server_queue(&evt) != ESP_OK) {
        TB_LOGE(TAG, "fail to send load ca file");
    }
    
    if (this->send_connect_server() != ESP_OK) {
        TB_LOGE(TAG, "fail to send server connect");
    }
}

void SystemManager::process_profile_end() {
    UIEvent evt = {};
    evt.type = UIEventType::PROFILE_ENDED;
    if (send_to_ui_queue(&evt) != ESP_OK) {
        TB_LOGE(TAG, "fail to send ui profile ended");
    }
}

void SystemManager::process_profile_response(EventProfileResponse *payload) {
    TB_LOGI(TAG, "profile response: %d", payload->response);

    UIEvent evt = {};
    evt.type = UIEventType::PROFILE_RESPONSE;
    memcpy(evt.payload, payload->buffer, UI_QUEUE_MAX_PAYLOAD);
    send_to_ui_queue(&evt);
}

void SystemManager::process_new_profile_info(EventNewProfileInfo *payload) {
    UIEvent evt = {};
    evt.type = UIEventType::NEW_PROFILE_INFO;
    memcpy(evt.payload, payload->buffer, UI_QUEUE_MAX_PAYLOAD);
    send_to_ui_queue(&evt);
}

void SystemManager::process_regulator_update(EventRegulatorUpdate *payload) {
    printf("Time: %" PRIu32 "\n", payload->info.time);
    printf("avg_chamber_temperature: %" PRIi32 "\n", payload->info.avg_chamber_temperature);

    {
        ServerEvent evt;
        evt.type = ServerEventType::PUBLISH_REGULATOR_UPDATE;
        memcpy(&evt.payload, payload->buffer, SERVER_QUEUE_MAX_PAYLOAD);
        if (send_to_server_queue(&evt) != ESP_OK) {
            TB_LOGE(TAG, "fail to send regulator update to server");
        }
    }

    {
        UIEvent evt;
        evt.type = UIEventType::REGULATOR_UPDATE;
        memcpy(evt.payload, payload->buffer, UI_QUEUE_MAX_PAYLOAD);
        if (send_to_ui_queue(&evt) != ESP_OK) {
            TB_LOGE(TAG, "fail to send regulator update to ui");
        }
    }
}

void SystemManager::process_command(char *cmd) {
    TB_LOGI(TAG, "command: %s", cmd);

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
        TB_LOGE(TAG, "Command returned non-zero error code: 0x%x (%s)\n", ret, esp_err_to_name(ret));
    } else if (err != ESP_OK) {
        TB_LOGE(TAG, "Internal error: %s\n", esp_err_to_name(err));
    }
}

void SystemManager::process_ui_to_sd_profiles_load(uint8_t *payload) {
    SDEvent event;
    event.type = SDEventType::UI_PROFILE_LIST;
    memcpy(event.payload, payload, SD_QUEUE_MAX_PAYLOAD);

    if (send_to_sd_queue(&event) != ESP_OK) {
        TB_LOGE(TAG, "failed to send PROFILE_LIST event to sd_queue");
    }
}

void SystemManager::process_sd_to_ui_profiles_load() {
    UIEvent evt;
    evt.type = UIEventType::PROFILES_LOAD;
    if (send_to_ui_queue(&evt) != ESP_OK) {
        TB_LOGE(TAG, "failed to send PROFILES_LOAD event to ui_queue");
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

esp_err_t SystemManager::send_connect_server() {
    esp_err_t err = ESP_OK;
    if ((err = this->send_disconnect_server()) != ESP_OK) {
        return err;
    }

    const char *uri = this->nvs_manager.get_config()->mqtt_broker_uri;
    const char *username = this->nvs_manager.get_config()->mqtt_username;
    const char *password = this->nvs_manager.get_config()->mqtt_password;

    return process_server_credentials(ServerEventType::CONNECT, uri, username, password);
}

esp_err_t SystemManager::send_disconnect_server() {
    ServerEvent evt;
    evt.type = ServerEventType::DISCONNECT;

    return send_to_server_queue(&evt);
}

esp_err_t SystemManager::send_mount_card() {
    SDEvent evt;
    evt.type = SDEventType::MOUNT_CARD;

    return send_to_sd_queue(&evt);
}

esp_err_t SystemManager::send_unmount_card() {
    SDEvent evt;
    evt.type = SDEventType::UNMOUNT_CARD;

    return send_to_sd_queue(&evt);
}

esp_err_t SystemManager::send_load_ca_cert() {
    return process_path_cmd(SDEventType::LOAD_CA_CERT, MQTT_TLS_CA_SD_CARD_PATH);
}

void SystemManager::process_ui_profile_start() {
    ProfileEvent evt = {};
    evt.type = ProfileEventType::START;
    if (send_to_profile_queue(&evt) != ESP_OK) {
        TB_LOGE(TAG, "failed to send profile start event to ui_queue");
    }
}

void SystemManager::process_ui_profile_stop() {
    ProfileEvent evt = {};
    evt.type = ProfileEventType::END;
    if (send_to_profile_queue(&evt) != ESP_OK) {
        TB_LOGE(TAG, "failed to send profile stop event to ui_queue");
    }
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

QueueHandle_t *SystemManager::get_regulator_queue() {
    return &this->regulator_queue_handle;
}

RingbufHandle_t *SystemManager::get_sd_ring_buf() {
    return &this->sd_ring_buf_handle;
}

SemaphoreHandle_t *SystemManager::get_spi_semaphore() {
    return &this->spi_semaphore;
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
