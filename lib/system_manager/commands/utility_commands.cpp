#include "string.h"

#include "esp_system.h"
#include "esp_chip_info.h"
#include "esp_flash.h"
#include "esp_log.h"
#include "sdkconfig.h"
#include "esp_console.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "logger.h"
#include "tb_event.h"
#include "nvs_config.h"
#include "system_manager.h"
#include "utility_commands.h"


const char * const TAG = "CMDsys";


static esp_err_t send_to_event_queue(Event *evt) {
    SystemManager *sysMgr = get_system_manager();
    QueueHandle_t *queue = sysMgr->get_event_queue();
    evt->origin = EventOrigin::CONSOLE;

    if (xQueueSend(*queue, &*evt, portMAX_DELAY) != pdTRUE) {
        TB_LOGE(TAG, "cmd sd event send fail");
        return ESP_FAIL;
    }
    return ESP_OK;
}


static int cmd_get_free(int argc, char **argv) {
    TB_ACK(TAG, "free heap memory: %" PRIu32, esp_get_free_heap_size());
    return ESP_OK;
}

static int cmd_soft_reset(int argc, char **argv) {
    TB_ACK(TAG, "soft_reset");
    fflush(stdout);
    esp_restart();
    // no need to return as chip restarts
}

static int cmd_get_heap(int argc, char **argv) {
    TB_ACK(TAG, "min heap size: %" PRIu32, heap_caps_get_minimum_free_size(MALLOC_CAP_DEFAULT));
    return ESP_OK;
}

static int cmd_get_chip_info(int argc, char **argv) {
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
}

static int cmd_get_tasks_info(int argc, char **argv) {
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
}

static int cmd_get_tasks_stats(int argc, char **argv) {
    TB_ACK(TAG, "get tasks stats");

    char buffer[512];
    vTaskGetRunTimeStats(buffer);
    printf("Tasks Statistics:\n%s\n", buffer);
    return ESP_OK;
}

static int cmd_get_log_level(int argc, char **argv) {
    TB_ACK(TAG, "log level: %s", log_level_to_s(esp_log_level_get("*")));
    return ESP_OK;
}

static int cmd_set_log_level(int argc, char **argv) {
    TB_ACK(TAG, "set log level");
    return ESP_OK;
}

static int cmd_new_cfg_sim(int argc, char **argv) {
    TB_ACK(TAG, "new cfg sim");
    Event evt = {};
    evt.type = EventType::SD_CONFIG_LOAD;
    EventSDConfigLoad payload = {};

    strlcpy(payload.config.wifi_ssid, "Orange_Swiatlowod_3760", WIFI_MAX_SSID_LEN);
    strlcpy(payload.config.wifi_pass, "2FT752946HF7", WIFI_MAX_PASS_LEN);
    strlcpy(payload.config.mqtt_broker_uri, "mqtts://192.168.1.105:8883", MQTT_MAX_BROKER_URI_LEN);
    strlcpy(payload.config.mqtt_username, "termobeka", MQTT_MAX_USERNAME_LEN);
    strlcpy(payload.config.mqtt_password, "qwerty", MQTT_MAX_PASSWORD_LEN);
    payload.config.log_level = ESP_LOG_DEBUG;
    memcpy(&evt.payload, &payload.buffer, sizeof(EventSDConfigLoad));

    return send_to_event_queue(&evt);
}

static const esp_console_cmd_t commands[] = {
//    command               help print                                              hint        callback                arguments
    { "get_free",           "Get the current size of free heap memory",             NULL,       &cmd_get_free,          NULL    },
    { "soft_reset",         "Software reset of the chip",                           NULL,       &cmd_soft_reset,        NULL    },
    { "get_heap",           "min free heap memory size during program execution",   NULL,       &cmd_get_heap,          NULL    },
    { "get_chip_info",      "Get information about chip and SDK",                   NULL,       &cmd_get_chip_info,     NULL    },
    { "get_tasks_info",     "Get information about running tasks",                  NULL,       &cmd_get_tasks_info,    NULL    },
    { "get_tasks_stats",    "Get tasks run time statistics",                        NULL,       &cmd_get_tasks_stats,   NULL    },
    { "get_log_level",      "Get current log level",                                NULL,       &cmd_get_log_level,     NULL    },
    { "set_log_level",      "Set current log level",                                NULL,       &cmd_set_log_level,     NULL    },
    { "new_cfg_sim",        "simulate new config",                                  NULL,       &cmd_new_cfg_sim,       NULL    }
};

void register_system_utility() {
    for (auto i : commands) {
        ESP_ERROR_CHECK(esp_console_cmd_register(&i));
    }
}
