#ifndef LIB_PROFILE_MANAGER_PROFILE_TYPE_H_
#define LIB_PROFILE_MANAGER_PROFILE_TYPE_H_

#include <string>
#include <memory>

#include "inttypes.h"

#include <pb.h>
#include "from_device_msg.pb.h"

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"

#include "global_config.h"


typedef struct {
    int16_t temperature;     // temperature in *C * 100
    uint32_t time_ms;        // time in ms
} __attribute__ ((packed)) profile_point;


typedef struct {
    profile_point points[PROFILE_MAX_VERTICES];
} profile_t;


typedef struct {
    profile_t profile;
    int16_t min_temp;
    int16_t max_temp;
    uint32_t step_time;
    uint32_t min_duration;
    uint32_t max_duration;
    uint32_t update_interval;
    QueueHandle_t *regulator_queue_handle;
} profile_config_t;

typedef struct {
    bool running;
    uint32_t current_duration;
    uint32_t total_duration;
    uint32_t step_start_time;
    uint32_t step_end_time;
    uint32_t step_time_left;
    uint32_t profile_time_left;
    int32_t current_temperature;
    float progress_percent;
    uint64_t absolute_start_time;           // ms from uC start
    uint64_t absolute_ended_time;           // total profile end time with total halt time
    uint8_t current_vertices;               // #vertices left
    uint8_t total_vertices;                 // total #vertices
} profile_run_info;

typedef enum {
    PROFILE_LOAD_SUCCESS,
    PROFILE_LOAD_FAIL,
    PROFILE_START_SUCCESS,
    PROFILE_START_FAIL,
    PROFILE_END_SUCCESS,
    PROFILE_END_FAIL,
    PROFILE_EVENT_NONE
} profile_event_response;

std::string profile_to_string(profile_t *profile);
profile_t string_to_profile(std::string string);

#endif  // LIB_PROFILE_MANAGER_PROFILE_TYPE_H_
