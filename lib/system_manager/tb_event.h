#ifndef LIB_SYSTEM_MANAGER_TB_EVENT_H_
#define LIB_SYSTEM_MANAGER_TB_EVENT_H_

#include <inttypes.h>

#define MAX_EVENT_PAYLOAD       100  // bytes
#define EVENT_QUEUE_LENGTH      20


typedef enum {
    ORIGIN_MAIN,
    ORIGIN_SYSTEM_MANAGER,
    ORIGIN_CONSOLE,
    ORIGIN_UI,
    ORIGIN_SERVER,
    ORIGIN_SD,
    ORIGIN_PROFILE_CONTROLLER,
    ORIGIN_UNKNOWN,
    ORIGIN_NONE,
} tb_event_origin;

typedef enum {
    TYPE_WIFI_SCAN,
    TYPE_WIFI_CONNECTED,
    TYPE_WIFI_DISCONNECTED,
    TYPE_WIFI_GOT_IP,
    TYPE_WIFI_GOT_TIME,

    TYPE_MQTT_CONNECTED,
    TYPE_MQTT_DISCONNECTED,
    TYPE_MQTT_DATA_SEND,

    TYPE_ERROR_INIT_CONFIG,
    TYPE_ERROR_INIT_WIFI,
    TYPE_ERROR_INIT_TASK,
    TYPE_ERROR_INIT_QUEUE,
    TYPE_ERROR_INIT_PERIPHERAL,
    TYPE_ERROR_OTHER,

    TYPE_UI_PROFILES_LOAD,
    TYPE_UI_PROFILE_CHOSEN,
    TYPE_UI_PROFILE_PREVIEW,
    TYPE_UI_PROFILE_START,
    TYPE_UI_PROFILE_STOP,

    TYPE_SD_CONFIG_LOAD,
    TYPE_SD_PROFILE_LIST,
    TYPE_SD_PROFILE_LOAD,
    TYPE_SD_PROFILE_DELETE,
    TYPE_SD_LOG,

    TYPE_PROFILE_CONTROLLER_STATUS_UPDATE,

    TYPE_SERVER_CONNECTED,
    TYPE_SERVER_DISCONNECTED,
    TYPE_SERVER_PROFILE_DOWNLOAD,

    TYPE_CONSOLE_COMMAND,

    TYPE_UNKNOWN,
    TYPE_NONE,
} tb_event_type;

typedef struct {
    tb_event_origin origin;
    tb_event_type type;
    uint8_t payload[MAX_EVENT_PAYLOAD];
} tb_event_t;

const char *event_origin_to_s(tb_event_origin origin);
const char *event_type_to_s(tb_event_type type);

#endif  // LIB_SYSTEM_MANAGER_TB_EVENT_H_
