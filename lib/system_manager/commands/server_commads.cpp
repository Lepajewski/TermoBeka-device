#include <cstring>

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "argtable3/argtable3.h"

#include "global_config.h"
#include "tb_event.h"
#include "system_manager.h"
#include "logger.h"
#include "server_commands.h"


const char * const TAG = "CMDserver";


esp_err_t send_to_server_queue(ServerEvent *evt) {
    SystemManager *sysMgr = get_system_manager();
    QueueHandle_t *queue = sysMgr->get_server_queue();
    evt->origin = EventOrigin::SYSTEM_MANAGER;

    if (xQueueSend(*queue, &*evt, portMAX_DELAY) != pdTRUE) {
        TB_LOGE(TAG, "cmd server event send fail");
        return ESP_FAIL;
    }
    return ESP_OK;
}

esp_err_t process_server_credentials(ServerEventType e_type, const char *uri, const char *uname, const char *pass) {
    ServerEvent evt = {};
    evt.type = e_type;
    ServerEventCredentials payload = {};

    strlcpy(payload.credentials.uri, uri, MQTT_MAX_BROKER_URI_LEN);
    strlcpy(payload.credentials.username, uname, MQTT_MAX_USERNAME_LEN);
    strlcpy(payload.credentials.password, pass, MQTT_MAX_PASSWORD_LEN);
    memcpy(&evt.payload, &payload.buffer, sizeof(ServerEventCredentials));

    return send_to_server_queue(&evt);
}

static struct {
    struct arg_str *uri;
    struct arg_str *username;
    struct arg_str *password;
    struct arg_end *end;
} cmd_server_credentials;

static void init_cmd_server_credentials_args() {
    cmd_server_credentials.uri = arg_str0(NULL, NULL, "<uri>", "Broker URI");
    cmd_server_credentials.username = arg_str0(NULL, NULL, "<uname>", "Client username");
    cmd_server_credentials.password = arg_str0(NULL, NULL, "<pass>", "Client passsword");
    cmd_server_credentials.end = arg_end(2);
}

static int cmd_mqtt_connect(int argc, char **argv) {
    TB_ACK(TAG, "connect");
    int nerrors = arg_parse(argc, argv, (void **) &cmd_server_credentials);
    if (nerrors != 0) {
        arg_print_errors(stderr, cmd_server_credentials.end, argv[0]);
        TB_NAK(TAG, "invalid arg");
        return ESP_FAIL;
    }

    if (cmd_server_credentials.uri->count == 0) {
        cmd_server_credentials.uri->sval[0] = MQTT_DEFAULT_BROKER_URI;
    }

    if (cmd_server_credentials.username->count == 0) {
        cmd_server_credentials.username->sval[0] = MQTT_DEFAULT_USERNAME;
    }

    if (cmd_server_credentials.password->count == 0) {
        cmd_server_credentials.password->sval[0] = MQTT_DEFAULT_PASSWORD;
    }

    TB_ACK(TAG, "server credentials: %s", cmd_server_credentials.uri->sval[0]);

    return process_server_credentials(
        ServerEventType::CONNECT,
        cmd_server_credentials.uri->sval[0],
        cmd_server_credentials.username->sval[0],
        cmd_server_credentials.password->sval[0]
    );
}

static int cmd_mqtt_disconnect(int argc, char **argv) {
    TB_ACK(TAG, "disconnect");
    ServerEvent evt;
    evt.type = ServerEventType::DISCONNECT;

    return send_to_server_queue(&evt);
}

static int cmd_mqtt_is_connected(int argc, char **argv) {
    TB_ACK(TAG, "is_connected");
    ServerEvent evt;
    evt.type = ServerEventType::IS_CONNECTED;

    return send_to_server_queue(&evt);
}


static const esp_console_cmd_t commands[] = {
//    command               help print                      hint        callback                arguments
    { "mqtt_connect",       "connect to MQTT broker",       NULL,       &cmd_mqtt_connect,      &cmd_server_credentials     },
    { "mqtt_disconnect",    "disconnect from MQTT broker",  NULL,       &cmd_mqtt_disconnect,   NULL                        },
    { "mqtt_is_connected",  "query MQTT status",            NULL,       &cmd_mqtt_is_connected, NULL                        },
};

void register_server() {
    init_cmd_server_credentials_args();

    for (auto i : commands) {
        ESP_ERROR_CHECK(esp_console_cmd_register(&i));
    }
}
