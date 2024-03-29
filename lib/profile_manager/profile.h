#ifndef LIB_PROFILE_MANAGER_PROFILE_H_
#define LIB_PROFILE_MANAGER_PROFILE_H_


#include "etl/list.h"
#include "esp_err.h"

#include <pb.h>
#include "from_device_msg.pb.h"

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/event_groups.h"

#include "profile_type.h"


class Profile {
 private:
    profile_run_info info;
    profile_config_t config;
    QueueHandle_t *regulator_queue_handle;

    // +1 vetrice due to stop/resume functionality
    etl::list<profile_point, PROFILE_MAX_VERTICES + 1> profile;
    EventGroupHandle_t profile_event_group;

    bool is_prepared = false;

    esp_err_t parse_raw_profile();
    esp_err_t calculate_vertices();
    esp_err_t calculate_duration();

    esp_err_t process_next_step();
    esp_err_t process_stopped();


    void send_evt_regulator(RegulatorEvent *evt);
    void send_evt_regulator_start();
    void send_evt_regulator_stop();
    void send_evt_regulator_update(int16_t temperature);
 public:
    Profile(profile_config_t config);
    ~Profile();

    esp_err_t prepare();
    esp_err_t start();
    esp_err_t end();

    void process_profile();

    bool is_running();

    EventGroupHandle_t *get_profile_event_group();
    profile_run_info *get_profile_run_info();

    void print_raw_profile();
    void print_profile();
    void print_info();
};


#endif  // LIB_PROFILE_MANAGER_PROFILE_H_
