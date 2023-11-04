#include "tb_event.h"

const char *event_origin_to_s(tb_event_origin origin) {
    switch (origin) {
        case ORIGIN_MAIN:                           return "MAIN";
        case ORIGIN_SYSTEM_MANAGER:                 return "System Manager";
        case ORIGIN_CONSOLE:                        return "Console";
        case ORIGIN_UI:                             return "UI";
        case ORIGIN_SERVER:                         return " Server";
        case ORIGIN_SD:                             return "SD";
        case ORIGIN_PROFILE_CONTROLLER:             return "Profile Controller";
        case ORIGIN_NONE:                           return "none";
        case ORIGIN_UNKNOWN:                        return "unknown";
        default:                                    return "invalid";
    }
}

const char *event_type_to_s(tb_event_type type) {
    switch (type) {
        case TYPE_WIFI_SCAN:                        return "wifi scan";
        case TYPE_WIFI_CONNECTED:                   return "wifi connected";
        case TYPE_WIFI_DISCONNECTED:                return "wifi disconnected";
        case TYPE_WIFI_GOT_IP:                      return "wifi got IP";
        case TYPE_WIFI_GOT_TIME:                    return "wifi got time";
        case TYPE_MQTT_CONNECTED:                   return "MQTT connected";
        case TYPE_MQTT_DISCONNECTED:                return "MQTT disconnected";
        case TYPE_MQTT_DATA_SEND:                   return "MQTT data send";
        case TYPE_ERROR_INIT_CONFIG:                return "error init config";
        case TYPE_ERROR_INIT_WIFI:                  return "error init wifi";
        case TYPE_ERROR_INIT_TASK:                  return "error init task";
        case TYPE_ERROR_INIT_QUEUE:                 return "error init queue";
        case TYPE_ERROR_INIT_PERIPHERAL:            return "error init peripheral";
        case TYPE_ERROR_OTHER:                      return "error other";
        case TYPE_UI_PROFILES_LOAD:                 return "UI profiles load";
        case TYPE_UI_PROFILE_CHOSEN:                return "UI profile chosen";
        case TYPE_UI_PROFILE_PREVIEW:               return "UI profile preview";
        case TYPE_UI_PROFILE_START:                 return "UI profile start";
        case TYPE_UI_PROFILE_STOP:                  return "UI profile stop";
        case TYPE_SD_CONFIG_LOAD:                   return "SD config load";
        case TYPE_SD_PROFILE_LIST:                  return "SD profile list";
        case TYPE_SD_PROFILE_LOAD:                  return "SD profile load";
        case TYPE_SD_PROFILE_DELETE:                return "SD profile delete";
        case TYPE_SD_LOG:                           return "SD log";
        case TYPE_PROFILE_CONTROLLER_STATUS_UPDATE: return "profile controller status update";
        case TYPE_SERVER_CONNECTED:                 return "server connected";
        case TYPE_SERVER_DISCONNECTED:              return "server disconnected";
        case TYPE_SERVER_PROFILE_DOWNLOAD:          return "server profile download";
        case TYPE_CONSOLE_COMMAND:                  return "console command";
        case TYPE_UNKNOWN:                          return "unknown";
        case TYPE_NONE:                             return "none";
        default:                                    return "invalid";
    }
}
