#ifndef LIB_PERIPHERALS_RTD_CONTROLLER_H_
#define LIB_PERIPHERALS_RTD_CONTROLLER_H_


#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

#include "global_config.h"
#include "Max31865.h"
#include "system_manager.h"


class RTDController {
 private:
    Max31865 sensors = Max31865({
        .cs = {
            PIN_RTD_CS_0,
            PIN_RTD_CS_1,
            PIN_RTD_CS_2,
            PIN_RTD_CS_3,
            PIN_RTD_CS_4, 
        }
    });
    SystemManager *sysMgr;
    SemaphoreHandle_t *spi_semaphore;

 public:
    RTDController();
    ~RTDController();

    void begin();
    void end();

    esp_err_t get_avg_temperature(float *t);
};


#endif  // LIB_PERIPHERALS_RTD_CONTROLLER_H_
