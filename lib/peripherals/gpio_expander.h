#ifndef LIB_PERIPHERALS_GPIO_EXPANDER_H_
#define LIB_PERIPHERALS_GPIO_EXPANDER_H_


#include "drivers/pca9539_driver.h"
#include "drivers/pca9539_intr_driver.h"


#define EXPANDER_EVT_INTR_QUEUE_SIZE      10


class GPIOExpander {
 private:
   pca9539_cfg_t pca9539_cfg;
   TaskHandle_t pca9539_intr_task_handle;
   QueueHandle_t pca9539_intr_evt_queue;

   void init();
   void deinit();
   esp_err_t init_evt_intr_queue();
   void deinit_evt_intr_queue();
   void start_evt_intr_task();

 public:
   GPIOExpander();
   GPIOExpander(pca9539_cfg_t *pca9539_cfg);
   ~GPIOExpander();

   void begin();
   void end();

   void set_polarity();
   void setup_buttons();

   pca9539_pin_mode get_pin_mode(pca9539_pin_num pin);
   void set_pin_mode(pca9539_pin_num pin, pca9539_pin_mode mode);

   pca9539_pin_state get_input_pin_state(pca9539_pin_num pin);
   pca9539_pin_state set_input_pin_state(pca9539_pin_num pin);
   pca9539_pin_state get_output_pin_state(pca9539_pin_num pin);
   pca9539_pin_state set_output_pin_state(pca9539_pin_num pin);
   
};

#endif  // LIB_PERIPHERALS_GPIO_EXPANDER_H_
