#include <cstring>

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "argtable3/argtable3.h"

#include "logger.h"
#include "tb_event.h"
#include "system_manager.h"
#include "ui_commands.h"


extern SystemManager sysMgr;


const char * const TAG = "CMDui";


static esp_err_t send_to_ui_queue(UIEvent *evt) {
    QueueHandle_t *queue = sysMgr.get_ui_queue();
    evt->origin = EventOrigin::SYSTEM_MANAGER;

    if (xQueueSend(*queue, &*evt, portMAX_DELAY) != pdTRUE) {
        TB_LOGE(TAG, "cmd sd event send fail");
        return ESP_FAIL;
    }
    return ESP_OK;
}


static struct {
    struct arg_int *duration;
    struct arg_end *end;
} cmd_beep_args;

static int cmd_beep(int argc, char **argv) {
    TB_ACK(TAG, "beep");

    int nerrors = arg_parse(argc, argv, (void **) &cmd_beep_args);
    if (nerrors != 0) {
        arg_print_errors(stderr, cmd_beep_args.end, argv[0]);
        TB_NAK(TAG, "invalid arg");
        return ESP_FAIL;
    }

    if (cmd_beep_args.duration->count == 0) {
        cmd_beep_args.duration->ival[0] = 500;
    }

    UIEvent evt;
    evt.type = UIEventType::BUZZER_BEEP;
    memcpy(evt.payload, &cmd_beep_args.duration->ival[0], sizeof(cmd_beep_args.duration->ival[0]));

    return send_to_ui_queue(&evt);
}


static const esp_console_cmd_t commands[] = {
    { "beep",           "buzzer beep",                  NULL,       &cmd_beep,          &cmd_beep_args  }
};

void register_user_interface() {
    cmd_beep_args.duration = arg_int0(NULL, NULL, "<timeout>", "Beep duration, ms");
    cmd_beep_args.end = arg_end(1);

    for (auto i : commands) {
        ESP_ERROR_CHECK(esp_console_cmd_register(&i));
    }
}
