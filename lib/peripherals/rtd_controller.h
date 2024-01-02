#ifndef LIB_PERIPHERALS_RTD_CONTROLLER_H_
#define LIB_PERIPHERALS_RTD_CONTROLLER_H_


#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

#include "global_config.h"
#include "rtd.h"
#include "system_manager.h"


typedef struct {
    float temperature[NUMBER_OF_RTDS];
    Max31865Error fault[NUMBER_OF_RTDS];
} rtd_temperature;


class RTDController {
 private:
    max31865_rtd_config_t config;
    SystemManager *sysMgr;
    SemaphoreHandle_t *spi_semaphore;

    RTD sensors[NUMBER_OF_RTDS] = {
        RTD(PIN_RTD_CS_0),
        RTD(PIN_RTD_CS_1),
        RTD(PIN_RTD_CS_2),
        RTD(PIN_RTD_CS_3),
        RTD(PIN_RTD_CS_4),        
    };

 public:
    RTDController();
    RTDController(max31865_rtd_config_t config);
    ~RTDController();

    void begin();
    void end();

    esp_err_t get_temperatures(rtd_temperature *t);
};


#endif  // LIB_PERIPHERALS_RTD_CONTROLLER_H_
