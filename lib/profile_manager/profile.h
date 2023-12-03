#ifndef LIB_PROFILE_MANAGER_PROFILE_H_
#define LIB_PROFILE_MANAGER_PROFILE_H_


#include "etl/list.h"
#include "esp_err.h"

#include <pb.h>
#include "profile_status_update.pb.h"

#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"

#include "profile_type.h"


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

    ProfileStatus get_status();
 public:
    Profile(profile_config_t config);
    ~Profile();

    esp_err_t start();
    esp_err_t stop();
    esp_err_t resume();
    esp_err_t end();

    void process_profile();
    ProfileStatusUpdate get_profile_run_info();

    bool is_running();

    int16_t get_min_temp();
    int16_t get_max_temp();

    EventGroupHandle_t *get_profile_event_group();

    void print_raw_profile();
    void print_profile();
    void print_info();
};


#endif  // LIB_PROFILE_MANAGER_PROFILE_H_
