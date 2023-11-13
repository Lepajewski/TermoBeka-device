#include <cstring>

#include "esp_err.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "argtable3/argtable3.h"

#include "system_manager.h"
#include "logger.h"
#include "tb_event.h"
#include "sd_commands.h"


extern SystemManager sysMgr;


const char * const TAG = "CMDsd";


static esp_err_t send_to_sd_queue(SDEvent *evt) {
    QueueHandle_t *queue = sysMgr.get_sd_queue();
    evt->origin = EventOrigin::SYSTEM_MANAGER;

    if (xQueueSend(*queue, &*evt, portMAX_DELAY) != pdTRUE) {
        TB_LOGE(TAG, "cmd sd event send fail");
        return ESP_FAIL;
    }
    return ESP_OK;
}

static int cmd_mount_card(int argc, char **argv) {
    TB_ACK(TAG, "mount card");
    SDEvent evt;
    evt.type = SDEventType::MOUNT_CARD;

    return send_to_sd_queue(&evt);
}


static struct {
    struct arg_str *path;
    struct arg_end *end;
} cmd_path;

static int cmd_list_files(int argc, char **argv) {
    int nerrors = arg_parse(argc, argv, (void **) &cmd_path);
    if (nerrors != 0) {
        arg_print_errors(stderr, cmd_path.end, argv[0]);
        TB_NAK(TAG, "invalid arg");
        return ESP_FAIL;
    }
    
    TB_ACK(TAG, "list files, path: %s", cmd_path.path->sval[0]);

    SDEvent evt;
    evt.type = SDEventType::LIST_FILES;

    if (strlen(cmd_path.path->sval[0]) + 1 < SD_QUEUE_MAX_PAYLOAD) {
        memcpy(evt.payload, cmd_path.path->sval[0], strlen(cmd_path.path->sval[0]) + 1);
    }

    return send_to_sd_queue(&evt);
}


static const esp_console_cmd_t commands[] = {
    {
        "mount_card",
        "mount SD card",
        NULL,
        &cmd_mount_card,
        NULL
    },
    {
        "ls",
        "list files on SD card",
        NULL,
        &cmd_list_files,
        &cmd_path
    }
};

void register_sd_card() {
    cmd_path.path = arg_str0(NULL, NULL, "<path>", "path beggining from /");
    cmd_path.end = arg_end(1);

    for (auto i : commands) {
        ESP_ERROR_CHECK(esp_console_cmd_register(&i));
    }
}
