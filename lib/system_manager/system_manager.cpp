#include <unistd.h>
#include "esp_system.h"
#include "esp_log.h"
#include "esp_console.h"
#include "esp_vfs_dev.h"
#include "driver/uart.h"
#include "linenoise/linenoise.h"
#include "argtable3/argtable3.h"
#include "esp_intr_alloc.h"

// commands required
#include "esp_flash.h"
#include "esp_chip_info.h"
#include "sdkconfig.h"

#include "system_manager.h"
#include "logger.h"
#include "tb_event.h"
#include "console_task.h"


const char * const TAG = "SysMgr";


SystemManager::SystemManager() {
#ifdef CONFIG_ESP_CONSOLE_UART_NUM
    this->uart_num = CONFIG_ESP_CONSOLE_UART_NUM;
#endif

    this->prompt_str = "esp $ ";

    /* Initialize the console */
    this->esp_console_config = {
        .max_cmdline_length = 256,
        .max_cmdline_args = 8,
#if CONFIG_LOG_COLORS
        .hint_color = atoi(LOG_COLOR_CYAN),
#endif
        .hint_bold = 0,
    };

    init_event_queue();
}

SystemManager::SystemManager(uart_port_t uart_num, const char *prompt_str) {
    this->uart_num = uart_num;
    this->prompt_str = prompt_str;
    /* Initialize the console */
    this->esp_console_config = {
        .max_cmdline_length = 256,
        .max_cmdline_args = 8,
#if CONFIG_LOG_COLORS
        .hint_color = atoi(LOG_COLOR_CYAN),
#endif
        .hint_bold = 0,
    };

    init_event_queue();
}

SystemManager::SystemManager(uart_port_t uart_num, const char *prompt_str, esp_console_config_t config) {
    this->uart_num = uart_num;
    this->prompt_str = prompt_str;
    this->esp_console_config = config;

    init_event_queue();
}

SystemManager::~SystemManager() {
    deinit_console();
}

void SystemManager::init_event_queue() {
    this->event_queue_handle = xQueueCreate(EVENT_QUEUE_LENGTH, sizeof(tb_event_t));

    if (this->event_queue_handle == NULL) {
        TB_LOGE(TAG, "event queue init fail. Restarting...");
        vTaskDelay(pdMS_TO_TICKS(2000));
        fflush(stdout);
        esp_restart();
    }
}

void SystemManager::configure_uart() {
    fflush(stdout);
    fsync(fileno(stdout));

    setvbuf(stdin, NULL, _IONBF, 0);

    esp_vfs_dev_uart_port_set_rx_line_endings(this->uart_num, ESP_LINE_ENDINGS_CR);
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

    ESP_ERROR_CHECK(uart_driver_install(this->uart_num, UART_BUF_SIZE, 0, 0, NULL, 0));
    ESP_ERROR_CHECK(uart_param_config(this->uart_num, &uart_config));

    esp_vfs_dev_uart_use_driver(this->uart_num);
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

    xTaskCreate(consoleTask, "ConsoleTask", 4096, (void *) this->prompt_str, 1, NULL);
}

void SystemManager::deinit_console() {
    TB_LOGI(TAG, "terminating console");
    esp_console_deinit();
}

void SystemManager::poll_event() {
    tb_event_t evt;

    if (xQueueReceive(this->event_queue_handle, &evt, portMAX_DELAY) == pdPASS) {
        TB_LOGI(TAG, "new event: %s, type: %s", event_origin_to_s(evt.origin), event_type_to_s(evt.type));
        TB_LOGI(TAG, "new event: %d, type: %d", evt.origin, evt.type);

        switch (evt.type) {
            case TYPE_CONSOLE_COMMAND:
            {
                TB_LOGI(TAG, "command: %s", reinterpret_cast<char *>(evt.payload));

                process_command(reinterpret_cast<char *>(evt.payload));
                break;
            }
            case TYPE_UNKNOWN:
            default:
            {
                TB_LOGI(TAG, "unknown event");
                break;
            }
        }

    }
}

void SystemManager::process_command(char *cmd) {
    /* Add the command to the history if not empty*/

    if (strlen(cmd) > 0) {
        linenoiseHistoryAdd(cmd);
    }

    /* Try to run the command */
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

    /* linenoise allocates line buffer on the heap, so need to free it */
    // linenoiseFree(cmd);
}

QueueHandle_t *SystemManager::get_event_queue() {
    return &this->event_queue_handle;
}

const esp_console_cmd_t SystemManager::system_commands[] = {
    {
        "get_free",  // command
        "Get the current size of free heap memory",  // info
        NULL,  // hint
        [](int argc, char **argv) -> int {  // function callback
            TB_ACK(TAG, "free heap memory: %" PRIu32, esp_get_free_heap_size());
            return ESP_OK;
        },
        NULL  // argument table
    },
    {
        "soft_reset",
        "Software reset of the chip",
        NULL,
        [](int argc, char **argv) -> int {
            TB_ACK(TAG, "soft_reset");
            fflush(stdout);
            esp_restart();
            // no need to return as chip restarts
        },
        NULL
    },
    {
        "get_heap",
        "Get minimum size of free heap memory that was available during program execution",
        NULL,
        [](int argc, char **argv) -> int {
            TB_ACK(TAG, "min heap size: %" PRIu32, heap_caps_get_minimum_free_size(MALLOC_CAP_DEFAULT));
            return ESP_OK;
        },
        NULL
    },
    {
        "get_chip_info",
        "Get information about chip and SDK",
        NULL,
        [](int argc, char **argv) -> int {
            esp_chip_info_t chip_info;
            uint32_t flash_size;
            esp_chip_info(&chip_info);

            if(esp_flash_get_size(NULL, &flash_size) != ESP_OK) {
                TB_NAK(TAG, "Get flash size failed");
                return ESP_FAIL;
            }
            TB_ACK(TAG, "IDF Version:%s ", esp_get_idf_version());
            TB_ACK(TAG, "Chip %s: cores: %d", CONFIG_IDF_TARGET, chip_info.cores);
            TB_ACK(TAG, "Features: %s%s%s%s%" PRIu32 "%s",
                chip_info.features & CHIP_FEATURE_WIFI_BGN ? "/802.11bgn" : "",
                chip_info.features & CHIP_FEATURE_BLE ? "/BLE" : "",
                chip_info.features & CHIP_FEATURE_BT ? "/BT" : "",
                chip_info.features & CHIP_FEATURE_EMB_FLASH ? "/Embedded-Flash: " : "/External-Flash: ",
                flash_size / (1024 * 1024), " MB");
            TB_ACK(TAG, "Silicon revision: v%d.%d", chip_info.revision / 100, chip_info.revision % 100);

            return ESP_OK;
        },
        NULL
    },
    {
        "get_tasks_info",
        "Get information about running tasks",
        NULL,
        [](int argc, char **argv) -> int {
            TB_ACK(TAG, "get_tasks_info");
            const size_t bytes_per_task = 40;  // see vTaskList description
            char *task_list_buffer = new char [uxTaskGetNumberOfTasks() * bytes_per_task];
            if (task_list_buffer == NULL) {
                TB_LOGE(TAG, "failed to allocate buffer for vTaskList output");
                return ESP_FAIL;
            }
            fputs("Task Name\tStatus\tPrio\tHWM\tTask#", stdout);
        #ifdef CONFIG_FREERTOS_VTASKLIST_INCLUDE_COREID
            fputs("\tAffinity", stdout);
        #endif
            fputs("\n", stdout);
            vTaskList(task_list_buffer);
            fputs(task_list_buffer, stdout);
            delete [] task_list_buffer;
        return ESP_OK;
        },
        NULL
    },
    {
        "get_log_level",
        "Get current log level",
        NULL,
        [](int argc, char **argv) -> int {
            TB_ACK(TAG, "log level: %s", log_level_to_s(esp_log_level_get("*")));
            return ESP_OK;
        },
        NULL
    },
    {
        "set_log_level",
        "Set current log level",
        NULL,
        [](int argc, char **argv) -> int {
            TB_ACK(TAG, "set log level");
            return ESP_OK;
        },
        NULL
    }
};

void SystemManager::register_commands() {
    esp_console_register_help_command();

    for (auto i : this->system_commands) {
        ESP_ERROR_CHECK(esp_console_cmd_register(&i));
    }
}
