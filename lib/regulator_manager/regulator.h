#ifndef LIB_REGULATOR_MANAGER_REGULATOR_H_
#define LIB_REGULATOR_MANAGER_REGULATOR_H_


#include "inttypes.h"

#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"

#include "etl/list.h"

#include <pb.h>
#include "from_device_msg.pb.h"

#include "global_config.h"
#include "external_temperature_sensor.h"
#include "relay_expander.h"
#include "rtd_controller.h"
#include "regulator_type.h"


class Regulator {
 private:
    regulator_config_t config;
    RegulatorStatusUpdate info;
    EventGroupHandle_t regulator_event_group;
    float set_temperature;
    uint64_t last_sample_time;

    RelayExpander expander;
    ExternalTemperatureSensor ds18b20 = ExternalTemperatureSensor(PIN_EXTERNAL_TEMP_SENSORS);
    RTDController controller;

    void setup_rtds();
    void setup();
    esp_err_t process_next_sample();
    void get_cpu_temperature();
    void get_external_temperature();
    float get_avg_rtd_temperature();

    void heaters_on();
    void heaters_off();
    void fans_on();
    void fans_off();
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
