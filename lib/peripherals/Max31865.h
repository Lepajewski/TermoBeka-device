#ifndef ESP32_MAX31865_H
#define ESP32_MAX31865_H


#include <driver/spi_common.h>
#include <driver/spi_master.h>
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include "global_config.h"


#define MAX31865_CONFIG_REG                     0x00
#define MAX31865_RTD_REG                        0x01
#define MAX31865_HIGH_FAULT_REG                 0x03
#define MAX31865_LOW_FAULT_REG                  0x05
#define MAX31865_FAULT_STATUS_REG               0x07

#define MAX31865_REG_WRITE_OFFSET               0x80

#define MAX31865_CONFIG_VBIAS_BIT               7
#define MAX31865_CONFIG_CONVERSIONMODE_BIT      6
#define MAX31865_CONFIG_1SHOT_BIT               5
#define MAX31865_CONFIG_NWIRES_BIT              4
#define MAX31865_CONFIG_FAULTDETECTION_BIT_1    3
#define MAX31865_CONFIG_FAULTDETECTION_BIT_0    2
#define MAX31865_CONFIG_FAULTSTATUS_BIT         1
#define MAX31865_CONFIG_MAINSFILTER_BIT         0

enum class Max31865NWires : uint8_t { 
    Three = 1,
    Two = 0, 
    Four = 0
};

enum class Max31865FaultDetection : uint8_t {
    NoAction = 0b00,
    AutoDelay = 0b01,
    ManualDelayCycle1 = 0b10,
    ManualDelayCycle2 = 0b11
};

enum class Max31865Filter : uint8_t {
    Hz50 = 1,
    Hz60 = 0
};

enum class Max31865Error : uint8_t {
    NoError = 0,
    Voltage = 2,
    RTDInLow,
    RefLow,
    RefHigh,
    RTDLow,
    RTDHigh
};


typedef struct {
  int cs[NUMBER_OF_RTDS];
} rtds_chip_select;

typedef struct {
  uint16_t rtd[NUMBER_OF_RTDS];
  bool fault_occured[NUMBER_OF_RTDS];
} rtd_conv_result;


class Max31865 {
 public:
  static const char *errorToString(Max31865Error error);

  Max31865(rtds_chip_select rtd_cs);
  ~Max31865();

  esp_err_t begin();
  esp_err_t setConfig();
  esp_err_t setRTDThresholds(uint16_t min, uint16_t max);
  esp_err_t getRTD(rtd_conv_result *result);
  esp_err_t getAvgTemperature(float *t);

 private:
  rtds_chip_select rtd_cs;
  uint8_t chipConfig[NUMBER_OF_RTDS];
  spi_device_handle_t deviceHandle[NUMBER_OF_RTDS];

  esp_err_t writeSPI(uint8_t addr, uint8_t *data, int index, size_t size = 1);
  esp_err_t readSPI(uint8_t addr, uint8_t *result, int index, size_t size = 1);
  esp_err_t setVbias(bool vbias);
  esp_err_t startOneShotConversion();

};

#endif  // ESP32_MAX31865_H
