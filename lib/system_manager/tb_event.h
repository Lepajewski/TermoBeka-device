#ifndef LIB_SYSTEM_MANAGER_TB_EVENT_H_
#define LIB_SYSTEM_MANAGER_TB_EVENT_H_

#include <inttypes.h>

#include <pb.h>
#include "from_device_msg.pb.h"
#include "to_device_msg.pb.h"

#include "global_config.h"
#include "nvs_config.h"
#include "profile_type.h"


#define QUEUE_DEFAULT_PAYLOAD           320  // bytes

#define EVENT_QUEUE_MAX_PAYLOAD         QUEUE_DEFAULT_PAYLOAD
#define EVENT_QUEUE_SIZE                20

#define UI_QUEUE_SIZE                   EVENT_QUEUE_SIZE
#define UI_QUEUE_MAX_PAYLOAD            QUEUE_DEFAULT_PAYLOAD

#define SD_QUEUE_SIZE                   EVENT_QUEUE_SIZE
#define SD_QUEUE_MAX_PAYLOAD            QUEUE_DEFAULT_PAYLOAD

#define WIFI_QUEUE_SIZE                 EVENT_QUEUE_SIZE
#define WIFI_QUEUE_MAX_PAYLOAD          QUEUE_DEFAULT_PAYLOAD

#define SERVER_QUEUE_SIZE               EVENT_QUEUE_SIZE
#define SERVER_QUEUE_MAX_PAYLOAD        QUEUE_DEFAULT_PAYLOAD

#define PROFILE_QUEUE_SIZE              EVENT_QUEUE_SIZE
#define PROFILE_QUEUE_MAX_PAYLOAD       QUEUE_DEFAULT_PAYLOAD


#define REGULATOR_QUEUE_SIZE            EVENT_QUEUE_SIZE
#define REGULATOR_QUEUE_MAX_PAYLOAD     QUEUE_DEFAULT_PAYLOAD


#define MAX_PATH_LENGTH                 64
#define MAX_RECORD_SIZE                 SD_QUEUE_MAX_PAYLOAD - MAX_PATH_LENGTH

#define PROFILE_NAME_SIZE               32

enum class EventOrigin {
    SYSTEM_MANAGER,
    CONSOLE,
    UI,
    SD,
    WIFI,
    SERVER,
    PROFILE,
    REGULATOR,
    NONE,
};

enum class EventType {
    WIFI_SCAN,
    WIFI_CONNECTED,
    WIFI_DISCONNECTED,
    WIFI_GOT_TIME,
    WIFI_STRENGTH,

    SERVER_CONNECTED,
    SERVER_DISCONNECTED,
    SERVER_PROFILE_LOAD,
    SERVER_PROFILE_START,
    SERVER_PROFILE_STOP,

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

    SD_MOUNTED,
    SD_UNMOUNTED,
    SD_CONFIG_LOAD,
    SD_LOAD_CA_FILE,
    SD_PROFILE_LIST,
    SD_PROFILE_LOAD,
    SD_PROFILE_DELETE,
    SD_LOG,

    PROFILE_START,
    PROFILE_END,
    PROFILE_RESPONSE,
    PROFILE_UPDATE,
    NEW_PROFILE_INFO,

    REGULATOR_START,
    REGULATOR_STOP,
    REGULATOR_UPDATE,

    CONSOLE_COMMAND,

    NONE,
};

typedef struct {
    EventOrigin origin;
    EventType type;
    uint8_t payload[EVENT_QUEUE_MAX_PAYLOAD];
} Event;

typedef union {
    struct {
        uint8_t num;
        uint8_t type;
    } press_data;
    uint8_t buffer[EVENT_QUEUE_MAX_PAYLOAD];
} EventUIButtonPress;

typedef union {
    nvs_device_config_t config;
    uint8_t buffer[MAX(sizeof(nvs_device_config_t), EVENT_QUEUE_MAX_PAYLOAD)];
} EventSDConfigLoad;

typedef union {
    profile_event_response response;
    uint8_t buffer[EVENT_QUEUE_MAX_PAYLOAD];
} EventProfileResponse;

typedef union {
    struct {
        uint32_t duration;
        char name[32];
    } info;
    uint8_t buffer[EVENT_QUEUE_MAX_PAYLOAD];
} EventNewProfileInfo;

typedef union {
    RegulatorStatusUpdate info;
    uint8_t buffer[EVENT_QUEUE_MAX_PAYLOAD];
} EventRegulatorUpdate;



enum class UIEventType {
    BUZZER_BEEP,
    ERROR_SHOW,
    WIFI_STRENGTH,
    PROFILES_LOAD,
    PROFILE_ENDED,
    PROFILE_RESPONSE,
    REGULATOR_UPDATE,
    NEW_PROFILE_INFO,
    NONE
};

typedef struct {
    EventOrigin origin;
    UIEventType type;
    uint8_t payload[UI_QUEUE_MAX_PAYLOAD];
} UIEvent;

typedef union {
    uint32_t duration;
    uint8_t buffer[UI_QUEUE_MAX_PAYLOAD];
} UIEventBuzzerBeep;



enum class SDEventType {
    MOUNT_CARD,
    UNMOUNT_CARD,
    LS,
    CAT,
    MKDIR,
    TOUCH,
    RM_FILE,
    RM_DIR,
    SAVE_TO_FILE,
    LOAD_CA_CERT,
    UI_PROFILE_LIST,
    CAT_PROFILE,
    NONE
};

typedef struct {
    EventOrigin origin;
    SDEventType type;
    uint8_t payload[SD_QUEUE_MAX_PAYLOAD];
} SDEvent;

typedef union {
    char path[SD_QUEUE_MAX_PAYLOAD];
    uint8_t buffer[SD_QUEUE_MAX_PAYLOAD];
} SDEventPathArg;

typedef union {
    struct {
        char path[MAX_PATH_LENGTH];
        char record[MAX_RECORD_SIZE];
    } params;
    uint8_t buffer[SD_QUEUE_MAX_PAYLOAD];
} SDEventPathBufArg;


enum class WiFiEventType {
    CONNECT,            // <ssid> <pass>
    DISCONNECT,
    IS_CONNECTED,
    GET_TIME,
    SCAN,
    NONE
};

typedef struct {
    EventOrigin origin;
    WiFiEventType type;
    uint8_t payload[WIFI_QUEUE_MAX_PAYLOAD];
} WiFiEvent;

typedef union {
    wifi_credentials credentials;
    uint8_t buffer[WIFI_QUEUE_MAX_PAYLOAD];
} WiFiEventCredentials;


enum class ServerEventType {
    CONNECT,            // <broker uri>
    DISCONNECT,
    IS_CONNECTED,
    PUBLISH_PROFILE_UPDATE,
    PUBLISH_REGULATOR_UPDATE,
    READ_CA_FILE,
    NONE
};

typedef struct {
    EventOrigin origin;
    ServerEventType type;
    uint8_t payload[SERVER_QUEUE_MAX_PAYLOAD];
} ServerEvent;

typedef union {
    mqtt_credentials credentials;
    uint8_t buffer[SERVER_QUEUE_MAX_PAYLOAD];
} ServerEventCredentials;

typedef union {
    RegulatorStatusUpdate info;
    uint8_t buffer[SERVER_QUEUE_MAX_PAYLOAD];
} ServerEventPubRegulatorUpdate;



enum class ProfileEventType {
    NEW_PROFILE,
    START,
    END,
    INFO,
    NONE
};

typedef struct {
    EventOrigin origin;
    ProfileEventType type;
    uint8_t payload[PROFILE_QUEUE_MAX_PAYLOAD];
} ProfileEvent;

typedef union {
    struct {
        profile_t profile;
        char profile_name[PROFILE_NAME_SIZE];
    };
    uint8_t buffer[PROFILE_QUEUE_MAX_PAYLOAD];
} ProfileEventNewProfile;



enum class RegulatorEventType {
    START,
    STOP,
    TEMPERATURE_UPDATE,
    NONE
};

typedef struct {
    EventOrigin origin;
    RegulatorEventType type;
    uint8_t payload[REGULATOR_QUEUE_MAX_PAYLOAD];
} RegulatorEvent;

typedef union {
    int16_t temperature;
    uint8_t buffer[REGULATOR_QUEUE_MAX_PAYLOAD];
} RegulatorEventTemperatureUpdate;


const char *event_origin_to_s(EventOrigin origin);
const char *event_type_to_s(EventType type);
const char *ui_event_type_to_s(UIEventType type);
const char *sd_event_type_to_s(SDEventType type);
const char *wifi_event_type_to_s(WiFiEventType type);
const char *server_event_type_to_s(ServerEventType type);
const char *profile_event_type_to_s(ProfileEventType type);
const char *regulator_event_type_to_s(RegulatorEventType type);

#endif  // LIB_SYSTEM_MANAGER_TB_EVENT_H_
