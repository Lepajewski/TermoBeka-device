#ifndef LIB_PERIPHERALS_DRIVERS_PCA9539_INTR_DRIVER_H_
#define LIB_PERIPHERALS_DRIVERS_PCA9539_INTR_DRIVER_H_


#include "pca9539_driver.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    PIN_CHANGE_1_TO_0,
    PIN_CHANGE_0_TO_1,
    PIN_NO_CHANGE
} pin_change_type;

typedef struct {
    pca9539_port_num port_num;
    pca9539_pin_num pin_num;
    pin_change_type change_type;
    uint64_t timestamp;
} pca9539_intr_evt_t;


esp_err_t pca9539_set_intr_task_handle(TaskHandle_t *task_handle);
esp_err_t pca9539_set_intr_evt_queue(QueueHandle_t *evt_queue);


void PCA9539IntrTask(void *pvParameters);


#ifdef __cplusplus
}
#endif

#endif  // LIB_PERIPHERALS_DRIVERS_PCA9539_INTR_DRIVER_H_
