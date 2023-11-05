#include "tb_event.h"

const char *event_origin_to_s(EventOrigin origin) {
    switch (origin) {
        case EventOrigin::MAIN:                           return "MAIN";
        case EventOrigin::SYSTEM_MANAGER:                 return "System Manager";
        case EventOrigin::CONSOLE:                        return "Console";
        case EventOrigin::UI:                             return "UI";
        case EventOrigin::SERVER:                         return " Server";
        case EventOrigin::SD:                             return "SD";
        case EventOrigin::PROFILE_CONTROLLER:             return "Profile Controller";
        case EventOrigin::NONE:                           return "none";
        case EventOrigin::UNKNOWN:                        return "unknown";
        default:                                                 return "invalid";
    }
}

const char *event_type_to_s(EventType type) {
    switch (type) {
        case EventType::WIFI_SCAN:                        return "wifi scan";
        case EventType::WIFI_CONNECTED:                   return "wifi connected";
        case EventType::WIFI_DISCONNECTED:                return "wifi disconnected";
        case EventType::WIFI_GOT_IP:                      return "wifi got IP";
        case EventType::WIFI_GOT_TIME:                    return "wifi got time";
        case EventType::MQTT_CONNECTED:                   return "MQTT connected";
        case EventType::MQTT_DISCONNECTED:                return "MQTT disconnected";
        case EventType::MQTT_DATA_SEND:                   return "MQTT data send";
        case EventType::ERROR_INIT_CONFIG:                return "error init config";
        case EventType::ERROR_INIT_WIFI:                  return "error init wifi";
        case EventType::ERROR_INIT_TASK:                  return "error init task";
        case EventType::ERROR_INIT_QUEUE:                 return "error init queue";
        case EventType::ERROR_INIT_PERIPHERAL:            return "error init peripheral";
        case EventType::ERROR_OTHER:                      return "error other";
        case EventType::UI_BUTTON_PRESS:                  return "UI button press";
        case EventType::UI_PROFILES_LOAD:                 return "UI profiles load";
        case EventType::UI_PROFILE_CHOSEN:                return "UI profile chosen";
        case EventType::UI_PROFILE_PREVIEW:               return "UI profile preview";
        case EventType::UI_PROFILE_START:                 return "UI profile start";
        case EventType::UI_PROFILE_STOP:                  return "UI profile stop";
        case EventType::SD_CONFIG_LOAD:                   return "SD config load";
        case EventType::SD_PROFILE_LIST:                  return "SD profile list";
        case EventType::SD_PROFILE_LOAD:                  return "SD profile load";
        case EventType::SD_PROFILE_DELETE:                return "SD profile delete";
        case EventType::SD_LOG:                           return "SD log";
        case EventType::PROFILE_CONTROLLER_STATUS_UPDATE: return "profile controller status update";
        case EventType::SERVER_CONNECTED:                 return "server connected";
        case EventType::SERVER_DISCONNECTED:              return "server disconnected";
        case EventType::SERVER_PROFILE_DOWNLOAD:          return "server profile download";
        case EventType::CONSOLE_COMMAND:                  return "console command";
        case EventType::UNKNOWN:                          return "unknown";
        case EventType::NONE:                             return "none";
        default:                                               return "invalid";
    }
}
