#ifndef LIB_PERIPHERALS_GPIO_EXPANDER_H_
#define LIB_PERIPHERALS_GPIO_EXPANDER_H_


#include "drivers/pca9539_driver.h"


class GPIOExpander {
 private:
   pca9539_cfg_t pca9539_cfg;

   void init();
   void deinit();

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
