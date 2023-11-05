#include "logger.h"


const char* log_level_to_s(esp_log_level_t level) {
    switch (level) {
        case ESP_LOG_NONE: return "NONE";
        case ESP_LOG_ERROR: return "ERROR";
        case ESP_LOG_WARN: return "WARN";
        case ESP_LOG_INFO: return "INFO";
        case ESP_LOG_DEBUG: return "DEBUG";
        case ESP_LOG_VERBOSE: return "VERBOSE";
        default: return "UNKNOWN";
    }
}
