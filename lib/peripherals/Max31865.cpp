#include "Max31865.h"
#include <driver/gpio.h>
#include <esp_log.h>
#include <freertos/task.h>
#include <math.h>
#include <string.h>
#include <climits>


#define RTD_CONVERT_SLOPE       30.90411507
#define RTD_CONVERT_INTERCEPT   8234.256681893283


static const char *TAG = "Max31865";


static float rtd_to_temperature(uint16_t rtd) {
    return (rtd - RTD_CONVERT_INTERCEPT) / RTD_CONVERT_SLOPE;
}


const char *Max31865::errorToString(Max31865Error error) {
  switch (error) {
    case Max31865Error::NoError: {
      return "No error";
    }
    case Max31865Error::Voltage: {
      return "Over/under voltage fault";
    }
    case Max31865Error::RTDInLow: {
      return "RTDIN- < 0.85*VBIAS (FORCE- open)";
    }
    case Max31865Error::RefLow: {
      return "REFIN- < 0.85*VBIAS (FORCE- open)";
    }
    case Max31865Error::RefHigh: {
      return "REFIN- > 0.85*VBIAS";
    }
    case Max31865Error::RTDLow: {
      return "RTD below low threshold";
    }
    case Max31865Error::RTDHigh: {
      return "RTD above high threshold";
    }
  }
  return "";
}

Max31865::Max31865(rtds_chip_select rtd_cs) : rtd_cs(rtd_cs)
{
  for (int i = 0; i < NUMBER_OF_RTDS; i++) {
    gpio_config_t gpioConfig = {};
    gpioConfig.intr_type = GPIO_INTR_DISABLE;
    gpioConfig.mode = GPIO_MODE_OUTPUT;
    gpioConfig.pull_up_en = GPIO_PULLUP_ENABLE;
    gpioConfig.pin_bit_mask = 1ULL << static_cast<uint32_t>(rtd_cs.cs[i]);
    gpio_config(&gpioConfig);

    gpio_set_level(static_cast<gpio_num_t>(rtd_cs.cs[i]), 1);
  }
}

Max31865::~Max31865() {
  for (int i = 0; i < NUMBER_OF_RTDS; i++) {
    esp_err_t err = spi_bus_remove_device(deviceHandle[i]);
    if (err != ESP_OK) {
      ESP_LOGE(TAG, "spi_bus_remove_device %d: %s", i, esp_err_to_name(err)); 
    }

    gpio_set_level(static_cast<gpio_num_t>(rtd_cs.cs[i]), 0);
  }
}

esp_err_t Max31865::begin() {
  esp_err_t err = ESP_OK;

  for (int i = 0; i < NUMBER_OF_RTDS; i++) {
    spi_device_interface_config_t deviceConfig = {};
    deviceConfig.spics_io_num = -1;
    deviceConfig.clock_speed_hz = 5 * 1000 * 1000;
    deviceConfig.mode = 3;
    deviceConfig.address_bits = CHAR_BIT;
    deviceConfig.command_bits = 0;
    deviceConfig.flags = SPI_DEVICE_HALFDUPLEX;
    deviceConfig.queue_size = 1;
    err = spi_bus_add_device(SPI3_HOST, &deviceConfig, &deviceHandle[i]);
    if (err != ESP_OK) {
      ESP_LOGE(TAG, "Error adding SPI device: %s", esp_err_to_name(err));
      return err;
    }
  }

  vTaskDelay(pdMS_TO_TICKS(5));

  err = setConfig();
  if (err != ESP_OK) {
    return err;
  }

  return setRTDThresholds(7000, 18000);  
}

esp_err_t Max31865::writeSPI(uint8_t addr, uint8_t *data, int index, size_t size) {
  assert(size <= 4);  // we're using the transaction buffers
  spi_transaction_t transaction = {};
  transaction.length = CHAR_BIT * size;
  transaction.rxlength = 0;
  transaction.addr = addr | MAX31865_REG_WRITE_OFFSET;
  transaction.flags = SPI_TRANS_USE_TXDATA;
  memcpy(transaction.tx_data, data, size);
  gpio_set_level(static_cast<gpio_num_t>(rtd_cs.cs[index]), 0);
  esp_err_t err = spi_device_polling_transmit(deviceHandle[index], &transaction);
  gpio_set_level(static_cast<gpio_num_t>(rtd_cs.cs[index]), 1);
  return err;
}

esp_err_t Max31865::readSPI(uint8_t addr, uint8_t *result, int index, size_t size) {
  assert(size <= 4);  // we're using the transaction buffers
  spi_transaction_t transaction = {};
  transaction.length = 0;
  transaction.rxlength = CHAR_BIT * size;
  transaction.addr = addr & (MAX31865_REG_WRITE_OFFSET - 1);
  transaction.flags = SPI_TRANS_USE_RXDATA;
  gpio_set_level(static_cast<gpio_num_t>(rtd_cs.cs[index]), 0);
  esp_err_t err = spi_device_polling_transmit(deviceHandle[index], &transaction);
  gpio_set_level(static_cast<gpio_num_t>(rtd_cs.cs[index]), 1);
  if (err != ESP_OK) {
    ESP_LOGE(TAG, "Error sending SPI transaction: %s", esp_err_to_name(err));
    return err;
  }
  memcpy(result, transaction.rx_data, size);
  return ESP_OK;
}


esp_err_t Max31865::setVbias(bool vbias) {
  esp_err_t err = ESP_OK;

  for (int i = 0; i < NUMBER_OF_RTDS; i++) {
    if (vbias) {
      chipConfig[i] |= 1U << MAX31865_CONFIG_VBIAS_BIT;
    } else {
      chipConfig[i] &= ~(1U << MAX31865_CONFIG_VBIAS_BIT);
    }

    err = writeSPI(MAX31865_CONFIG_REG, &chipConfig[i], i, 1);
    if (err != ESP_OK) {
      return err;
    }
  }

  return err;
}

esp_err_t Max31865::startOneShotConversion() {
  esp_err_t err = ESP_OK;

  for (int i = 0; i < NUMBER_OF_RTDS; i++) {
    uint8_t cfg = chipConfig[i];
    cfg |= 1U << MAX31865_CONFIG_1SHOT_BIT;
    err = writeSPI(MAX31865_CONFIG_REG, &cfg, i, 1);
    if (err != ESP_OK) {
      ESP_LOGE(TAG, "Error writing config: %s", esp_err_to_name(err));
      return err;
    }
  }

  return err;
}

esp_err_t Max31865::setConfig() {
  esp_err_t err = ESP_OK;

  for (int i = 0; i < NUMBER_OF_RTDS; i++) {
    uint8_t cfg = 0;

    cfg |= 0U << MAX31865_CONFIG_CONVERSIONMODE_BIT;
    cfg |= (uint32_t)(Max31865NWires::Four) << MAX31865_CONFIG_NWIRES_BIT;
    cfg |= 0U << MAX31865_CONFIG_FAULTDETECTION_BIT;
    cfg |= (uint32_t)(Max31865Filter::Hz60) << MAX31865_CONFIG_MAINSFILTER_BIT;
    cfg |= 0U << MAX31865_CONFIG_VBIAS_BIT;

    err = writeSPI(MAX31865_CONFIG_REG, &cfg, i, 1);
    if (err != ESP_OK) {
      return err;
    }
    ESP_LOGI(TAG, "SET CONFIG %d: %" PRIu8, i, cfg);
    chipConfig[i] = cfg;
  }
  
  return err;
}

esp_err_t Max31865::setRTDThresholds(uint16_t min, uint16_t max) {
  assert((min < (1 << 15)) && (max < (1 << 15)));
  esp_err_t err = ESP_OK;

  for (int i = 0; i < NUMBER_OF_RTDS; i++) {
    uint8_t thresholds[4];
    thresholds[0] = static_cast<uint8_t>((max << 1) >> CHAR_BIT);
    thresholds[1] = static_cast<uint8_t>(max << 1);
    thresholds[2] = static_cast<uint8_t>((min << 1) >> CHAR_BIT);
    thresholds[3] = static_cast<uint8_t>(min << 1);

    err = writeSPI(MAX31865_HIGH_FAULT_REG, thresholds, i, sizeof(thresholds));
    if (err != ESP_OK) {
      return err;
    }
  }

  return err;  
}

esp_err_t Max31865::getRTD(rtd_conv_result *result) {
  esp_err_t err = ESP_OK;

  err = setVbias(true);
  if (err != ESP_OK) {
    return err;
  }
  vTaskDelay(pdMS_TO_TICKS(6));

  err = startOneShotConversion();
  if (err != ESP_OK) {
    return err;
  }
  vTaskDelay(pdMS_TO_TICKS(70));

  for (int i = 0; i < NUMBER_OF_RTDS; i++) {
    uint8_t cfg = 0;
    err = readSPI(MAX31865_CONFIG_REG, &cfg, i, 1);
    if (err != ESP_OK) {
      ESP_LOGE(TAG, "Error reading config: %s", esp_err_to_name(err));
      return err;
    }

    if ((cfg & BIT(MAX31865_CONFIG_1SHOT_BIT)) != 0) {
      ESP_LOGE(TAG, "Conversion not finished: %d", i);
      return ESP_ERR_INVALID_RESPONSE;
    }

    uint8_t rtd_data[2] = {};
    err = readSPI(MAX31865_RTD_REG, rtd_data, i, 2);
    if (err != ESP_OK) {
      ESP_LOGE(TAG, "fail to read RTD reg");
      return err;
    }

    result->rtd[i] = rtd_data[0] << 8;
    result->rtd[i] |= rtd_data[1];
  }

  err = setVbias(true);
  if (err != ESP_OK) {
    return err;
  }

  for (int i = 0; i < NUMBER_OF_RTDS; i++) {
    if (static_cast<bool>(result->rtd[i] & 1U)) {
      result->fault_occured[i] = true;
      result->rtd[i] = 0;
    } else {
      result->rtd[i] >>= 1U;
    }
    // ESP_LOGI(TAG, "Raw RTD %d: %" PRIu16, i, result->rtd[i]);
  }

  return err;
}

esp_err_t Max31865::getAvgTemperature(float *t) {
  esp_err_t err = ESP_OK;
  rtd_conv_result result = {};
  float sum = 0.0;
  int working_sensors = 0;

  err = this->getRTD(&result);
  if (err != ESP_OK) {
    return err;
  }

  for (int i = 0; i < NUMBER_OF_RTDS; i++) {
    if (!result.fault_occured[i]) {
      float t = rtd_to_temperature(result.rtd[i]);
      ESP_LOGI(TAG, "%i: %f*C", i, t);
      sum += t;
      working_sensors++;
    }
  }

  if (working_sensors == 0) {
    return ESP_FAIL;
  }
  
  *t = sum / (float)working_sensors;

  return err;
}

