#ifndef LIB_PERIPHERALS_RELAY_TYPE_H_
#define LIB_PERIPHERALS_RELAY_TYPE_H_


#define USED_RELAYS              8


enum class RelayType {
   RELAY_HEATER_1 = 0,
   RELAY_HEATER_2,
   RELAY_FAN_1,
   RELAY_FAN_2,
   RELAY_4,
   RELAY_5,
   RELAY_6,
   RELAY_7,
   RELAY_NONE
};


#endif  // LIB_PERIPHERALS_RELAY_TYPE_H_
