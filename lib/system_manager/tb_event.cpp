#include "tb_event.h"


const char *event_origin_to_s(EventOrigin origin) {
    switch (origin) {
        case EventOrigin::SYSTEM_MANAGER:                   return "System Manager";
        case EventOrigin::CONSOLE:                          return "Console";
        case EventOrigin::UI:                               return "UI";
        case EventOrigin::SD:                               return "SD";
        case EventOrigin::WIFI:                             return "WiFi";
        case EventOrigin::SERVER:                           return "Server";
        case EventOrigin::PROFILE:                          return "Profile";
        case EventOrigin::NONE:                             return "none";
        case EventOrigin::UNKNOWN:                          return "unknown";
        default:                                            return "invalid";
    }
}

const char *event_type_to_s(EventType type) {
    switch (type) {
        case EventType::WIFI_SCAN:                          return "wifi scan";
        case EventType::WIFI_CONNECTED:                     return "wifi connected";
        case EventType::WIFI_DISCONNECTED:                  return "wifi disconnected";
        case EventType::WIFI_GOT_TIME:                      return "wifi got time";

        case EventType::SERVER_CONNECTED:                   return "server connected";
        case EventType::SERVER_DISCONNECTED:                return "server disconnected";

        case EventType::ERROR_INIT_CONFIG:                  return "error init config";
        case EventType::ERROR_INIT_WIFI:                    return "error init wifi";
        case EventType::ERROR_INIT_TASK:                    return "error init task";
        case EventType::ERROR_INIT_QUEUE:                   return "error init queue";
        case EventType::ERROR_INIT_PERIPHERAL:              return "error init peripheral";
        case EventType::ERROR_OTHER:                        return "error other";

        case EventType::UI_BUTTON_PRESS:                    return "UI button press";
        case EventType::UI_PROFILES_LOAD:                   return "UI profiles load";
        case EventType::UI_PROFILE_CHOSEN:                  return "UI profile chosen";
        case EventType::UI_PROFILE_PREVIEW:                 return "UI profile preview";
        case EventType::UI_PROFILE_START:                   return "UI profile start";
        case EventType::UI_PROFILE_STOP:                    return "UI profile stop";

        case EventType::SD_MOUNTED:                         return "SD mounted";
        case EventType::SD_UNMOUNTED:                       return "SD unmounted";
        case EventType::SD_CONFIG_LOAD:                     return "SD config load";
        case EventType::SD_LOAD_CA_FILE:                    return "SD load ca file";
        case EventType::SD_PROFILE_LIST:                    return "SD profile list";
        case EventType::SD_PROFILE_LOAD:                    return "SD profile load";
        case EventType::SD_PROFILE_DELETE:                  return "SD profile delete";
        case EventType::SD_LOG:                             return "SD log";

        case EventType::PROFILE_START:                      return "Profile start";
        case EventType::PROFILE_STOP:                       return "Profile stop";
        case EventType::PROFILE_RESUME:                     return "Profile resume";
        case EventType::PROFILE_END:                        return "Profile end";
        case EventType::PROFILE_RESPONSE:                   return "Profile response";
        case EventType::PROFILE_UPDATE:                     return "Profile update";

        case EventType::CONSOLE_COMMAND:                    return "console command";

        case EventType::NONE:
        default:                                            return "invalid";
    }
}

const char *ui_event_type_to_s(UIEventType type) {
    switch (type) {
        case UIEventType::BUZZER_BEEP:                      return "buzzer beep";
        case UIEventType::ERROR_SHOW:                       return "error show";
        case UIEventType::NONE:                             return "none";
        default:                                            return "invalid";
    }
}

const char *sd_event_type_to_s(SDEventType type) {
    switch (type) {
        case SDEventType::MOUNT_CARD:                       return "mount card";
        case SDEventType::UNMOUNT_CARD:                     return "unmount card";
        case SDEventType::LS:                               return "ls";
        case SDEventType::CAT:                              return "cat";
        case SDEventType::MKDIR:                            return "mkdir";
        case SDEventType::TOUCH:                            return "touch";
        case SDEventType::RM_FILE:                          return "rm file";
        case SDEventType::RM_DIR:                           return "rm dir";
        case SDEventType::SAVE_TO_FILE:                     return "save to file";
        case SDEventType::LOAD_CA_CERT:                     return "load ca cert";
        case SDEventType::NONE:                             return "none";
        default:                                            return "invalid";
    }
}

const char *wifi_event_type_to_s(WiFiEventType type) {
    switch (type) {
        case WiFiEventType::CONNECT:                        return "connect";
        case WiFiEventType::DISCONNECT:                     return "disconnect";
        case WiFiEventType::IS_CONNECTED:                   return "is connected";
        case WiFiEventType::GET_TIME:                       return "get time";
        case WiFiEventType::SCAN:                           return "scan";
        case WiFiEventType::NONE:                           return "none";
        default:                                            return "invalid";
    }
}

const char *server_event_type_to_s(ServerEventType type) {
    switch (type) {
        case ServerEventType::CONNECT:                      return "connect";
        case ServerEventType::DISCONNECT:                   return "disconnect";
        case ServerEventType::IS_CONNECTED:                 return "is connected";
        case ServerEventType::PUBLISH_PROFILE_UPDATE:       return "publish profile update";
        case ServerEventType::PUBLISH_REGULATOR_UPDATE:     return "publish regulator update";
        case ServerEventType::READ_CA_FILE:                 return "read ca file";
        case ServerEventType::NONE:                         return "none";
        default:                                            return "invalid";
    }
}

const char *profile_event_type_to_s(ProfileEventType type) {
    switch (type) {
        case ProfileEventType::NEW_PROFILE:                 return "new profile";
        case ProfileEventType::START:                       return "start";
        case ProfileEventType::STOP:                        return "stop";
        case ProfileEventType::RESUME:                      return "resume";
        case ProfileEventType::END:                         return "end";
        case ProfileEventType::INFO:                        return "info";
        case ProfileEventType::NONE:                        return "none";
        default:                                            return "invalid";
    }
}
