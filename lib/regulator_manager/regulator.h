#ifndef LIB_REGULATOR_MANAGER_REGULATOR_H_
#define LIB_REGULATOR_MANAGER_REGULATOR_H_


#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"

#include <pb.h>
#include "status_update.pb.h"

#include "regulator_type.h"


class Regulator {
 private:
    regulator_config_t config;
    RegulatorStatusUpdate info;
    EventGroupHandle_t regulator_event_group;

    esp_err_t process_next_sample();

 public:
    Regulator(regulator_config_t config);
    ~Regulator();

    esp_err_t start();
    esp_err_t stop();

    void process_regulator();
    RegulatorStatusUpdate get_regulator_run_info();
    bool is_running();

    EventGroupHandle_t *get_regulator_event_group();

    void print_info();
};


#endif  // LIB_REGULATOR_MANAGER_REGULATOR_H_
