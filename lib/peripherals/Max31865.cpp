#include "Max31865.h"
#include <driver/gpio.h>
#include <esp_log.h>
#include <freertos/task.h>
#include <math.h>
#include <string.h>
#include <climits>

static const char *TAG = "Max31865";

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

Max31865::Max31865(int cs) : cs(cs)
{
    gpio_config_t gpioConfig = {};
    gpioConfig.intr_type = GPIO_INTR_DISABLE;
    gpioConfig.mode = GPIO_MODE_OUTPUT;
    gpioConfig.pull_up_en = GPIO_PULLUP_ENABLE;
    gpioConfig.pin_bit_mask = 1ULL << cs;
    gpio_config(&gpioConfig);

    gpio_set_level(static_cast<gpio_num_t>(cs), 1);
}

Max31865::~Max31865() {
    this->end();
}

esp_err_t Max31865::begin(max31865_config_t config) {
    esp_err_t err = ESP_OK;

    spi_device_interface_config_t deviceConfig = {};
    deviceConfig.spics_io_num = -1;
    deviceConfig.clock_speed_hz = 5 * 1000 * 1000;
    deviceConfig.mode = 3;
    deviceConfig.address_bits = CHAR_BIT;
    deviceConfig.command_bits = 0;
    deviceConfig.flags = SPI_DEVICE_HALFDUPLEX;
    deviceConfig.queue_size = 1;
    err = spi_bus_add_device(SPI3_HOST, &deviceConfig, &deviceHandle);
    if (err != ESP_OK) {
      ESP_LOGE(TAG, "Error adding SPI device: %s", esp_err_to_name(err));
      return err;
    }
    return setConfig(config);
}

esp_err_t Max31865::end() {
    return spi_bus_remove_device(deviceHandle);
}

esp_err_t Max31865::writeSPI(uint8_t addr, uint8_t *data, size_t size) {
    assert(size <= 4);  // we're using the transaction buffers
    spi_transaction_t transaction = {};
    transaction.length = CHAR_BIT * size;
    transaction.rxlength = 0;
    transaction.addr = addr | MAX31865_REG_WRITE_OFFSET;
    transaction.flags = SPI_TRANS_USE_TXDATA;
    memcpy(transaction.tx_data, data, size);
    gpio_set_level(static_cast<gpio_num_t>(cs), 0);
    esp_err_t err = spi_device_polling_transmit(deviceHandle, &transaction);
    gpio_set_level(static_cast<gpio_num_t>(cs), 1);
    return err;
}

esp_err_t Max31865::readSPI(uint8_t addr, uint8_t *result, size_t size) {
    assert(size <= 4);  // we're using the transaction buffers
    spi_transaction_t transaction = {};
    transaction.length = 0;
    transaction.rxlength = CHAR_BIT * size;
    transaction.addr = addr & (MAX31865_REG_WRITE_OFFSET - 1);
    transaction.flags = SPI_TRANS_USE_RXDATA;
    gpio_set_level(static_cast<gpio_num_t>(cs), 0);
    esp_err_t err = spi_device_polling_transmit(deviceHandle, &transaction);
    gpio_set_level(static_cast<gpio_num_t>(cs), 1);
    if (err != ESP_OK) {
      ESP_LOGE(TAG, "Error sending SPI transaction: %s", esp_err_to_name(err));
      return err;
    }
    memcpy(result, transaction.rx_data, size);
    return ESP_OK;
}

esp_err_t Max31865::setConfig(max31865_config_t config) {
    chipConfig = config;
    uint8_t configByte = 0;
    if (config.vbias) {
      configByte |= 1UL << MAX31865_CONFIG_VBIAS_BIT;
    }
    if (config.autoConversion) {
      configByte |= 1UL << MAX31865_CONFIG_CONVERSIONMODE_BIT;
    }
    if (config.nWires == Max31865NWires::Three) {
      configByte |= 1UL << MAX31865_CONFIG_NWIRES_BIT;
    }
    if (config.faultDetection != Max31865FaultDetection::NoAction) {
      configByte |= static_cast<uint8_t>(config.faultDetection)
                    << MAX31865_CONFIG_FAULTDETECTION_BIT;
    }
    if (config.filter != Max31865Filter::Hz60) {
      configByte |= 1UL << MAX31865_CONFIG_MAINSFILTER_BIT;
    }

    ESP_LOGI(TAG, "SET CONFIG: %" PRIu8, configByte);

    return writeSPI(MAX31865_CONFIG_REG, &configByte, 1);
}

esp_err_t Max31865::getConfig(max31865_config_t *config) {
    uint8_t configByte = 0;
    esp_err_t err = readSPI(MAX31865_CONFIG_REG, &configByte, 1);
    if (err != ESP_OK) {
      ESP_LOGE(TAG, "Error reading config: %s", esp_err_to_name(err));
      return err;
    }

    ESP_LOGI(TAG, "CONFIG: %" PRIu8, configByte);

    config->vbias = ((configByte >> MAX31865_CONFIG_VBIAS_BIT) & 1U) != 0;
    config->autoConversion =
        ((configByte >> MAX31865_CONFIG_CONVERSIONMODE_BIT) & 1U) != 0;
    config->nWires = static_cast<Max31865NWires>(
        (configByte >> MAX31865_CONFIG_NWIRES_BIT) & 1U);
    config->faultDetection = static_cast<Max31865FaultDetection>(
        (configByte >> MAX31865_CONFIG_FAULTDETECTION_BIT) & 1U);
    config->filter = static_cast<Max31865Filter>(
        (configByte >> MAX31865_CONFIG_MAINSFILTER_BIT) & 1U);
    return ESP_OK;
}

esp_err_t Max31865::setRTDThresholds(uint16_t min, uint16_t max) {
    assert((min < (1 << 15)) && (max < (1 << 15)));
    uint8_t thresholds[4];
    thresholds[0] = static_cast<uint8_t>((max << 1) >> CHAR_BIT);
    thresholds[1] = static_cast<uint8_t>(max << 1);
    thresholds[2] = static_cast<uint8_t>((min << 1) >> CHAR_BIT);
    thresholds[3] = static_cast<uint8_t>(min << 1);
    return writeSPI(MAX31865_HIGH_FAULT_REG, thresholds, sizeof(thresholds));
}

esp_err_t Max31865::clearFault() {
    uint8_t configByte = 0;
    esp_err_t err = readSPI(MAX31865_CONFIG_REG, &configByte, 1);
    if (err != ESP_OK) {
      ESP_LOGE(TAG, "Error reading config: %s", esp_err_to_name(err));
      return err;
    }
    configByte |= 1U << MAX31865_CONFIG_FAULTSTATUS_BIT;
    return writeSPI(MAX31865_CONFIG_REG, &configByte, 1);
}

esp_err_t Max31865::readFaultStatus(Max31865Error *fault) {
    *fault = Max31865Error::NoError;
    uint8_t faultByte = 0;
    esp_err_t err = readSPI(MAX31865_FAULT_STATUS_REG, &faultByte, 1);
    if (err != ESP_OK) {
      ESP_LOGE(TAG, "Error reading fault status: %s", esp_err_to_name(err));
      return err;
    }
    if (faultByte != 0) {
      *fault = static_cast<Max31865Error>(CHAR_BIT * sizeof(unsigned int) - 1 -
                                          __builtin_clz(faultByte));
    }
    return clearFault();
}

esp_err_t Max31865::getRTD(uint16_t *rtd, Max31865Error *fault) {
    max31865_config_t oldConfig = chipConfig;
    bool restoreConfig = false;
    if (!chipConfig.vbias) {
      restoreConfig = true;
      chipConfig.vbias = true;
      esp_err_t err = setConfig(chipConfig);
      if (err != ESP_OK) {
        ESP_LOGE(TAG, "Error setting config: %s", esp_err_to_name(err));
        return err;
      }
      vTaskDelay(pdMS_TO_TICKS(10));
    }

    if (!chipConfig.autoConversion) {

      restoreConfig = true;
      uint8_t configByte = 0;

      esp_err_t err = readSPI(MAX31865_CONFIG_REG, &configByte, 1);
      if (err != ESP_OK) {
        ESP_LOGE(TAG, "Error reading config: %s", esp_err_to_name(err));
        return err;
      }
      configByte |= 1U << MAX31865_CONFIG_1SHOT_BIT;
      err = writeSPI(MAX31865_CONFIG_REG, &configByte, 1);
      if (err != ESP_OK) {
        ESP_LOGE(TAG, "Error writing config: %s", esp_err_to_name(err));
        return err;
      }
      vTaskDelay(pdMS_TO_TICKS(65));
  }

    uint8_t rtdBytes[2];
    esp_err_t err = readSPI(MAX31865_RTD_REG, rtdBytes, 2);
    if (err != ESP_OK) {
      ESP_LOGE(TAG, "Error reading RTD: %s", esp_err_to_name(err));
      return err;
    }

    if (static_cast<bool>(rtdBytes[1] & 1U)) {
      *rtd = 0;
      if (fault == nullptr) {
        auto tmp = Max31865Error::NoError;
        fault = &tmp;
      }
      readFaultStatus(fault);
      ESP_LOGW(TAG, "Sensor fault detected: %s", errorToString(*fault));
      return ESP_ERR_INVALID_RESPONSE;
    }

    *rtd = rtdBytes[0] << CHAR_BIT;
    *rtd |= rtdBytes[1];
    *rtd >>= 1U;

    return restoreConfig ? setConfig(oldConfig) : ESP_OK;
}