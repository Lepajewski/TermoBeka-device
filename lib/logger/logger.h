#ifndef LIB_LOGGER_H_
#define LIB_LOGGER_H_


#include "esp_log.h"


#ifdef DEBUG
#define TB_LOGE(TAG, ...) ESP_LOGE(TAG, __VA_ARGS__)
#define TB_LOGW(TAG, ...) ESP_LOGW(TAG, __VA_ARGS__)
#define TB_LOGI(TAG, ...) ESP_LOGI(TAG, __VA_ARGS__)
#define TB_LOGD(TAG, ...) ESP_LOGD(TAG, __VA_ARGS__)
#define TB_LOGV(TAG, ...) ESP_LOGV(TAG, __VA_ARGS__)
#define TB_ACK(TAG, ...)  ESP_LOGI(TAG, "ACK: " __VA_ARGS__)
#define TB_NAK(TAG, ...)  ESP_LOGI(TAG, "NAK: " __VA_ARGS__)
#elif defined(RELEASE)
#define TB_LOGE(TAG, ...) {}
#define TB_LOGW(TAG, ...) {}
#define TB_LOGI(TAG, ...) {}
#define TB_LOGD(TAG, ...) {}
#define TB_LOGV(TAG, ...) {}
#define TB_ACK(TAG, ...)
#define TB_NAK(TAG, ...)
#endif

#ifdef __cplusplus
extern "C" {
#endif

const char* log_level_to_s(esp_log_level_t level);

#ifdef __cplusplus
}
#endif

#endif  // LIB_LOGGER_H_
