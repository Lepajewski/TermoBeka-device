#include <cstring>

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "argtable3/argtable3.h"

#include "global_config.h"
#include "system_manager.h"
#include "logger.h"
#include "wifi_commands.h"


const char * const TAG = "CMDwifi";


esp_err_t send_to_wifi_queue(WiFiEvent *evt) {
    SystemManager *sysMgr = get_system_manager();
    QueueHandle_t *queue = sysMgr->get_wifi_queue();
    evt->origin = EventOrigin::SYSTEM_MANAGER;

    if (xQueueSend(*queue, &*evt, portMAX_DELAY) != pdTRUE) {
        TB_LOGE(TAG, "cmd sd event send fail");
        return ESP_FAIL;
    }
    return ESP_OK;
}

esp_err_t process_wifi_credentials(WiFiEventType e_type, const char *ssid, const char *pass) {
    WiFiEvent evt = {};
    evt.type = e_type;
    WiFiEventCredentials payload = {};

    strlcpy(payload.credentials.ssid, ssid, WIFI_MAX_SSID_LEN);
    strlcpy(payload.credentials.pass, pass, WIFI_MAX_PASS_LEN);
    memcpy(&evt.payload, &payload.buffer, sizeof(WiFiEventCredentials));

    return send_to_wifi_queue(&evt);
}


static struct {
    struct arg_str *ssid;
    struct arg_str *pass;
    struct arg_end *end;
} cmd_wifi_credencials;

static void init_cmd_wifi_credencials_args() {
    cmd_wifi_credencials.ssid = arg_str0(NULL, NULL, "<ssid>", "SSID of AP");
    cmd_wifi_credencials.pass = arg_str0(NULL, NULL, "<pass>", "PSK of AP");
    cmd_wifi_credencials.end = arg_end(2);
}

static int cmd_connect(int argc, char **argv) {
    int nerrors = arg_parse(argc, argv, (void **) &cmd_wifi_credencials);
    if (nerrors != 0) {
        arg_print_errors(stderr, cmd_wifi_credencials.end, argv[0]);
        TB_NAK(TAG, "invalid arg");
        return ESP_FAIL;
    }

    if (cmd_wifi_credencials.ssid->count == 0 ||
        cmd_wifi_credencials.pass->count == 0) {
        cmd_wifi_credencials.ssid->sval[0] = WIFI_DEFAULT_SSID;
        cmd_wifi_credencials.pass->sval[0] = WIFI_DEFAULT_PASS;
    }

    TB_ACK(TAG, "wifi credentials: %s %s", cmd_wifi_credencials.ssid->sval[0], cmd_wifi_credencials.pass->sval[0]);

    return process_wifi_credentials(WiFiEventType::CONNECT, cmd_wifi_credencials.ssid->sval[0], cmd_wifi_credencials.pass->sval[0]);
}

static int cmd_disconnect(int argc, char **argv) {
    TB_ACK(TAG, "mount card");
    WiFiEvent evt;
    evt.type = WiFiEventType::DISCONNECT;

    return send_to_wifi_queue(&evt);
}

static int cmd_is_connected(int argc, char **argv) {
    TB_ACK(TAG, "is_connected");
    WiFiEvent evt;
    evt.type = WiFiEventType::IS_CONNECTED;

    return send_to_wifi_queue(&evt);
}

static int cmd_get_time(int argc, char **argv) {
    TB_ACK(TAG, "get_time");
    WiFiEvent evt;
    evt.type = WiFiEventType::GET_TIME;

    return send_to_wifi_queue(&evt);
}

static int cmd_wifi_scan(int argc, char **argv) {
    TB_ACK(TAG, "wifi_scan");
    WiFiEvent evt;
    evt.type = WiFiEventType::SCAN;

    return send_to_wifi_queue(&evt);
}

static const esp_console_cmd_t commands[] = {
//    command           help print                      hint        callback            arguments
    { "connect",        "connect to WiFi",              NULL,       &cmd_connect,       &cmd_wifi_credencials   },
    { "disconnect",     "disconnect from WiFi",         NULL,       &cmd_disconnect,    NULL                    },
    { "is_connected",   "query WiFi status",            NULL,       &cmd_is_connected,  NULL                    },
    { "get_time",       "get current time",             NULL,       &cmd_get_time,      NULL                    },
    { "wifi_scan",      "scan for WiFi APs",            NULL,       &cmd_wifi_scan,     NULL                    },
};

void register_wifi() {
    init_cmd_wifi_credencials_args();

    for (auto i : commands) {
        ESP_ERROR_CHECK(esp_console_cmd_register(&i));
    }
}
