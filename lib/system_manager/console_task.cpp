#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "linenoise/linenoise.h"
#include "string.h"
#include "tb_event.h"
#include "logger.h"
#include "system_manager.h"

#include "console_task.h"


const char * const TAG = "CLI";


void consoleTask(void *pvParameters) {
    const char *prompt_str = (const char *) pvParameters;
    SystemManager *sysMgr = get_system_manager();

    QueueHandle_t *event_queue_handle = sysMgr->get_event_queue();

    while (1) {
        /* Get a line using linenoise.
        * The line is returned when ENTER is pressed.
        */
        char *line = linenoise(prompt_str);

        if (line == NULL) { /* Break on EOF or error */
            // return ESP_ERR_INVALID_STATE;
            TB_NAK(TAG, "empty command");
            continue;
        }

        Event evt;
        evt.origin = EventOrigin::CONSOLE;
        evt.type = EventType::CONSOLE_COMMAND;

        if (strlen(line) + 1 < EVENT_QUEUE_MAX_PAYLOAD) {
            memcpy(evt.payload, line, strlen(line) + 1);
        }

        if (xQueueSend(*event_queue_handle, &evt, portMAX_DELAY) != pdTRUE) {
            TB_LOGE(TAG, "cmd event send fail");
        }

        /* linenoise allocates line buffer on the heap, so need to free it */
        linenoiseFree(line);
    }
}