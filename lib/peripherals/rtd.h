#ifndef LIB_REGULATOR_MANAGER_RTD_H_
#define LIB_REGULATOR_MANAGER_RTD_H_


#include "driver/gpio.h"

#include "Max31865.h"


class RTD {
 private:
    Max31865 *sensor;
    int cs;

 public:
    RTD(int cs);
    ~RTD();

    esp_err_t get_rtd(uint16_t *rtd, Max31865Error *fault);
};


#endif  // LIB_REGULATOR_MANAGER_RTD_H_
