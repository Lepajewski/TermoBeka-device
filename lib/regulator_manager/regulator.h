#ifndef LIB_REGULATOR_MANAGER_REGULATOR_H_
#define LIB_REGULATOR_MANAGER_REGULATOR_H_


#include "inttypes.h"

#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"

#include <pb.h>
#include "from_device_msg.pb.h"

#include "regulator_type.h"


class Regulator {
 private:
    regulator_config_t config;
    RegulatorStatusUpdate info;
    EventGroupHandle_t regulator_event_group;
    int16_t set_temperature;
    uint64_t last_sample_time;

    esp_err_t process_next_sample();
    int16_t get_ambient_temperature();
 public:
    Regulator(regulator_config_t config);
    ~Regulator();

    esp_err_t start();
    esp_err_t stop();

    void process_regulator();
    RegulatorStatusUpdate get_regulator_run_info();
    bool is_running();
    int16_t get_min_temperature();
    int16_t get_max_temperature();
    void update_temperature(int16_t temperature);

    EventGroupHandle_t *get_regulator_event_group();

    void print_info();
};


#endif  // LIB_REGULATOR_MANAGER_REGULATOR_H_
