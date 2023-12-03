#include <cstring>

#include "esp_err.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "argtable3/argtable3.h"

#include "system_manager.h"
#include "logger.h"
#include "tb_event.h"
#include "profile_commands.h"


const char * const TAG = "CMDprof";


static esp_err_t send_to_profile_queue(ProfileEvent *evt) {
    SystemManager *sysMgr = get_system_manager();
    QueueHandle_t *queue = sysMgr->get_profile_queue();
    evt->origin = EventOrigin::SYSTEM_MANAGER;

    if (xQueueSend(*queue, &*evt, portMAX_DELAY) != pdTRUE) {
        TB_LOGE(TAG, "cmd sd event send fail");
        return ESP_FAIL;
    }
    return ESP_OK;
}


static int cmd_new_profile(int argc, char **argv) {
    TB_ACK(TAG, "new profile");
    ProfileEvent evt = {};
    evt.type = ProfileEventType::NEW_PROFILE;
    ProfileEventNewProfile payload = {};

    for (uint8_t i = 0; i < PROFILE_MAX_VERTICES; i++) {
        payload.profile.points[i] = {-1, UINT32_MAX};
    }

    payload.profile.points[0] = {2000,  0};
    payload.profile.points[1] = {10000, 30000};
    payload.profile.points[2] = {10000, 150000};
    payload.profile.points[3] = {4000,  160000};
    payload.profile.points[4] = {13000, 180000};
    payload.profile.points[5] = {13000, 210000};
    payload.profile.points[6] = {2000,  220000};
    
    memcpy(&evt.payload, &payload.buffer, sizeof(ProfileEventNewProfile));

    return send_to_profile_queue(&evt);
}

static int cmd_start_profile(int argc, char **argv) {
    TB_ACK(TAG, "start profile");
    ProfileEvent evt = {};
    evt.type = ProfileEventType::START;

    return send_to_profile_queue(&evt);
}

static int cmd_stop_profile(int argc, char **argv) {
    TB_ACK(TAG, "stop profile");
    ProfileEvent evt = {};
    evt.type = ProfileEventType::STOP;

    return send_to_profile_queue(&evt);
}

static int cmd_resume_profile(int argc, char **argv) {
    TB_ACK(TAG, "resume profile");
    ProfileEvent evt = {};
    evt.type = ProfileEventType::RESUME;

    return send_to_profile_queue(&evt);
}

static int cmd_end_profile(int argc, char **argv) {
    TB_ACK(TAG, "end profile");
    ProfileEvent evt = {};
    evt.type = ProfileEventType::END;

    return send_to_profile_queue(&evt);
}

static int cmd_profile_info(int argc, char **argv) {
    TB_ACK(TAG, "profile info");
    ProfileEvent evt = {};
    evt.type = ProfileEventType::INFO;

    return send_to_profile_queue(&evt);
}

static const esp_console_cmd_t commands[] = {
//    command           help print                      hint        callback                arguments
    { "new_profile",    "load example profile",         NULL,       &cmd_new_profile,       NULL                },
    { "start_profile",  "start loaded profile",         NULL,       &cmd_start_profile,     NULL                },
    { "stop_profile",   "stop running profile",         NULL,       &cmd_stop_profile,      NULL                },
    { "resume_profile", "resume stopped profile",       NULL,       &cmd_resume_profile,    NULL                },
    { "end_profile",    "end running profile",          NULL,       &cmd_end_profile,       NULL                },
    { "profile_info",   "print profile info",           NULL,       &cmd_profile_info,      NULL                },
};


void register_profile() {
    for (auto i : commands) {
        ESP_ERROR_CHECK(esp_console_cmd_register(&i));
    }
}
