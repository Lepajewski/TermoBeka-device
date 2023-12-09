#ifndef LIB_REGULATOR_MANAGER_REGULATOR_MANAGER_H_
#define LIB_REGULATOR_MANAGER_REGULATOR_MANAGER_H_


#include "inttypes.h"

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "system_manager.h"
#include "tb_event.h"
#include "regulator.h"


class RegulatorManager {
 private:
    SystemManager *sysMgr;
    QueueHandle_t *event_queue_handle;
    QueueHandle_t *regulator_queue_handle;

    Regulator *regulator;

    void setup();
    void end();

    esp_err_t start_regulator();
    esp_err_t stop_regulator();
    esp_err_t process_temperature_update(int16_t temperature);

    void process_regulator_event(RegulatorEvent *evt);
    void poll_regulator_events();
    void poll_running_regulator_events();

    void send_evt(Event *evt);
    void send_evt_start();
    void send_evt_stop();
    void send_evt_update();

 public:
    RegulatorManager();
    ~RegulatorManager();

    void process_events();
};


#endif  // LIB_REGULATOR_MANAGER_REGULATOR_MANAGER_H_
