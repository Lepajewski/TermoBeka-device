#ifndef LIB_REGULATOR_MANAGER_RELAY_EXPANDER_H_
#define LIB_REGULATOR_MANAGER_RELAY_EXPANDER_H_


#include "global_config.h"
#include "drivers/pca9539_driver.h"
#include "expander_controller.h"
#include "relay.h"


static const pca9539_cfg_t expander_config = {
   .i2c_port = I2C_NUM_0,
   .i2c_config = {
      .mode = I2C_MODE_MASTER,
      .sda_io_num = PIN_GPIO_EXPANDER_SDA,
      .scl_io_num = PIN_GPIO_EXPANDER_SCL,
      .sda_pullup_en = GPIO_PULLUP_ENABLE,
      .scl_pullup_en = GPIO_PULLUP_ENABLE,
      .master = {
         .clk_speed = GPIO_EXPANDER_FREQ_HZ,
      },
      .clk_flags = 0,
   },
   .addr = PCA9539_I2C_ADDRESS_LH,
   .intr_gpio_num = GPIO_NUM_NC,
   .rst_gpio_num = PIN_GPIO_EXPANDER_2_RESET
};


class RelayExpander {
 private:
    ExpanderController controller = ExpanderController(expander_config);
    Relay relays[USED_RELAYS] = {
      Relay(controller, PIN_POLARITY_NORMAL, P1_0, RelayType::RELAY_HEATER_1),
      Relay(controller, PIN_POLARITY_NORMAL, P1_1, RelayType::RELAY_HEATER_2),
      Relay(controller, PIN_POLARITY_NORMAL, P1_2, RelayType::RELAY_FAN_1),
      Relay(controller, PIN_POLARITY_NORMAL, P1_3, RelayType::RELAY_FAN_2),
      Relay(controller, PIN_POLARITY_NORMAL, P1_4, RelayType::RELAY_4),
      Relay(controller, PIN_POLARITY_NORMAL, P1_5, RelayType::RELAY_5),
      Relay(controller, PIN_POLARITY_NORMAL, P1_6, RelayType::RELAY_6),
      Relay(controller, PIN_POLARITY_NORMAL, P1_7, RelayType::RELAY_7)
    };

   void setup_relays(); 
   Relay *lookup_relay(RelayType type);

 public:
    RelayExpander();
    ~RelayExpander();

    void begin();
    void end();

    void relay_on(RelayType type);
    void relay_off(RelayType type);
};


#endif  // LIB_REGULATOR_MANAGER_RELAY_EXPANDER_H_
