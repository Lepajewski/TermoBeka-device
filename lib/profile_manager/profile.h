#ifndef LIB_PROFILE_MANAGER_PROFILE_H_
#define LIB_PROFILE_MANAGER_PROFILE_H_


#include "etl/list.h"
#include "esp_err.h"

#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"

#include "profile_type.h"


typedef struct {
    profile_t profile;
    uint16_t min_temp;
    uint16_t max_temp;
    uint32_t step_time;
    uint32_t min_duration;
    uint32_t max_duration;
} profile_config_t;


class Profile {
 private:
    bool running;
    uint64_t start_time;
    uint8_t vertices;
    uint32_t duration;
    profile_config_t config;
    etl::list<profile_point, PROFILE_MAX_VERTICES> profile;
    EventGroupHandle_t profile_event_group;

    float current_a;
    float current_b;

    esp_err_t process_raw_profile();
    esp_err_t calculate_vertices();
    esp_err_t calculate_duration();
    esp_err_t calculate_next_step();
 public:
    Profile(profile_config_t config);
    ~Profile();

    esp_err_t prepare();
    esp_err_t start();
    void process_next_step();

    esp_err_t end();

    uint8_t get_vertices();
    bool is_running();

    void set_absolute_start_time(uint64_t time);
    uint64_t get_absolute_start_time();
    uint64_t get_absolute_end_time();
    uint32_t get_duration_total();
    uint32_t get_duration_so_far();

    uint16_t get_min_temp();
    uint16_t get_max_temp();

    EventGroupHandle_t *get_profile_event_group();

    void print_raw_profile();
    void print_profile();
};


#endif  // LIB_PROFILE_MANAGER_PROFILE_H_
