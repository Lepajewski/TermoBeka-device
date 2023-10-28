#ifndef LIB_PERIPHERALS_DRIVERS_PCA9539_INTR_DRIVER_H_
#define LIB_PERIPHERALS_DRIVERS_PCA9539_INTR_DRIVER_H_


#include "pca9539_driver.h"

#ifdef __cplusplus
extern "C" {
#endif


esp_err_t pca9539_set_intr_task_handle(TaskHandle_t *task_handle);
esp_err_t pca9539_set_intr_evt_queue(QueueHandle_t *evt_queue);


void PCA9539IntrTask(void *pvParameters);


#ifdef __cplusplus
}
#endif

#endif  // LIB_PERIPHERALS_DRIVERS_PCA9539_INTR_DRIVER_H_
