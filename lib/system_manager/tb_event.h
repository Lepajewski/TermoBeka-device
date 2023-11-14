#ifndef LIB_SYSTEM_MANAGER_TB_EVENT_H_
#define LIB_SYSTEM_MANAGER_TB_EVENT_H_

#include <inttypes.h>

#define EVENT_QUEUE_MAX_PAYLOAD         100  // bytes
#define EVENT_QUEUE_SIZE                20

#define UI_QUEUE_SIZE                   EVENT_QUEUE_SIZE
#define UI_QUEUE_MAX_PAYLOAD            EVENT_QUEUE_MAX_PAYLOAD

#define SD_QUEUE_SIZE                   EVENT_QUEUE_SIZE
#define SD_QUEUE_MAX_PAYLOAD            EVENT_QUEUE_MAX_PAYLOAD


enum class EventOrigin {
    MAIN,
    SYSTEM_MANAGER,
    CONSOLE,
    UI,
    SERVER,
    SD,
    PROFILE_CONTROLLER,
    UNKNOWN,
    NONE,
};

enum class EventType {
    WIFI_SCAN,
    WIFI_CONNECTED,
    WIFI_DISCONNECTED,
    WIFI_GOT_IP,
    WIFI_GOT_TIME,

    MQTT_CONNECTED,
    MQTT_DISCONNECTED,
    MQTT_DATA_SEND,

    ERROR_INIT_CONFIG,
    ERROR_INIT_WIFI,
    ERROR_INIT_TASK,
    ERROR_INIT_QUEUE,
    ERROR_INIT_PERIPHERAL,
    ERROR_OTHER,

    UI_BUTTON_PRESS,
    UI_PROFILES_LOAD,
    UI_PROFILE_CHOSEN,
    UI_PROFILE_PREVIEW,
    UI_PROFILE_START,
    UI_PROFILE_STOP,

    SD_CONFIG_LOAD,
    SD_PROFILE_LIST,
    SD_PROFILE_LOAD,
    SD_PROFILE_DELETE,
    SD_LOG,

    PROFILE_CONTROLLER_STATUS_UPDATE,

    SERVER_CONNECTED,
    SERVER_DISCONNECTED,
    SERVER_PROFILE_DOWNLOAD,

    CONSOLE_COMMAND,

    UNKNOWN,
    NONE,
};

typedef struct {
    EventOrigin origin;
    EventType type;
    uint8_t payload[EVENT_QUEUE_MAX_PAYLOAD];
} Event;

const char *event_origin_to_s(EventOrigin origin);
const char *event_type_to_s(EventType type);


enum class UIEventType {
    BUZZER_BEEP,
    ERROR_SHOW,
    NONE
};

typedef struct {
    EventOrigin origin;
    UIEventType type;
    uint8_t payload[UI_QUEUE_MAX_PAYLOAD];
} UIEvent;


enum class SDEventType {
    MOUNT_CARD,
    UNMOUNT_CARD,
    LIST_FILES,
    CAT_FILE,
    MKDIR,
    TOUCH,
    RM_FILE,
    RM_DIR,
    SAVE_TO_FILE,
    NONE
};

typedef struct {
    EventOrigin origin;
    SDEventType type;
    uint8_t payload[SD_QUEUE_MAX_PAYLOAD];
} SDEvent;


#endif  // LIB_SYSTEM_MANAGER_TB_EVENT_H_
