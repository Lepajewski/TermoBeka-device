#ifndef LIB_PROFILE_MANAGER_PROFILE_H_
#define LIB_PROFILE_MANAGER_PROFILE_H_


#include "etl/list.h"
#include "esp_err.h"

#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"

#include "profile_type.h"


typedef struct {
    profile_t profile;
    int16_t min_temp;
    int16_t max_temp;
    uint32_t step_time;
    uint32_t min_duration;
    uint32_t max_duration;
    uint32_t update_interval;
} profile_config_t;

typedef struct {
    uint64_t absolute_start_time;           // ms from uC start
    uint32_t current_duration;              // ms from uC start
    uint32_t total_duration;                // profile duration in ms
    uint32_t step_start_time;               // start time of current step in profile (ms)
    uint32_t step_end_time;                 // end time of current step in profile (ms)
    
    uint64_t absolute_time_stopped;         // ms from uC start
    uint32_t step_stopped_time;             // ms from current step start
    uint32_t profile_stopped_time;          // ms from profile start

    uint32_t step_time_left;                // ms left to stopped step end
    uint32_t profile_time_left;             // ms left to profile end

    uint32_t profile_time_halted;           // total profile halt duration ms
    uint64_t absolute_time_resumed;         // ms from uC start
    uint32_t profile_resumed_time;          // ms from profile start
    
    uint64_t absolute_ended_time;           // total profile end time with total halt time

    int16_t current_temperature;            // regulated temperature
    uint8_t current_vertices;               // #vertices left
    uint8_t total_vertices;                 // total #vertices

    float progress_percent;                 // profile progress in %

    bool running;                           // profile is running
    bool stopped;                           // profile is stopped
    bool ended;                             // profile is ended/finished
} profile_run_info;


class Profile {
 private:
    profile_run_info info;
    profile_config_t config;

    // +1 vetrice due to stop/resume functionality
    etl::list<profile_point, PROFILE_MAX_VERTICES + 1> profile;
    EventGroupHandle_t profile_event_group;


    esp_err_t parse_raw_profile();
    esp_err_t calculate_vertices();
    esp_err_t calculate_duration();
    esp_err_t prepare();

    esp_err_t process_next_step();
    esp_err_t process_stopped();
 public:
    Profile(profile_config_t config);
    ~Profile();

    esp_err_t start();
    esp_err_t stop();
    esp_err_t resume();
    esp_err_t end();

    void process_profile();
    profile_run_info get_profile_run_info();

    bool is_running();

    int16_t get_min_temp();
    int16_t get_max_temp();

    EventGroupHandle_t *get_profile_event_group();

    void print_raw_profile();
    void print_profile();
    void print_info();
};


#endif  // LIB_PROFILE_MANAGER_PROFILE_H_
