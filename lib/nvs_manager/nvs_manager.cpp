#include "string.h"

#include "esp_system.h"
#include "nvs_flash.h"
#include "nvs.h"
#include "logger.h"

#include "nvs_manager.h"


const char * const TAG = "NVS";


NVSManager::NVSManager() {
    strlcpy(this->default_config.wifi_ssid, WIFI_DEFAULT_SSID, WIFI_MAX_SSID_LEN);
    strlcpy(this->default_config.wifi_pass, WIFI_DEFAULT_PASS, WIFI_MAX_PASS_LEN);
    this->default_config.log_level = DEFAULT_LOG_LEVEL;
}

NVSManager::~NVSManager() {}

void NVSManager::begin() {
    esp_err_t err = this->nvs_check();
    if (err != ESP_OK) {
        TB_LOGE(TAG, "init error");
    } else {
        TB_LOGI(TAG, "init");
        err = save_default_config();
        if (err != ESP_OK) {
            TB_LOGE(TAG, "fail to save default config");
        } else {
            TB_LOGI(TAG, "saved default config");
        }
    }
}

esp_err_t NVSManager::nvs_check() {
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        // NVS partition was truncated and needs to be erased
        // Retry nvs_flash_init
        TB_LOGE(TAG, "init error, erasing");
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    return err;
}

esp_err_t NVSManager::save(const char* key, nvs_device_config_t *config) {
    esp_err_t err = ESP_OK;
    nvs_handle_t nvs_handle;
    if ((err = nvs_open("storage", NVS_READWRITE, &nvs_handle)) != ESP_OK) {
        return err;
    }
    
    if ((err = nvs_set_blob(nvs_handle, key, (void *) config, sizeof(nvs_device_config_t))) != ESP_OK) {
        nvs_close(nvs_handle);
        return err;
    }

    if ((err = nvs_commit(nvs_handle)) != ESP_OK) {
        nvs_close(nvs_handle);
        return err;
    }

    nvs_close(nvs_handle);
    return err;
}

esp_err_t NVSManager::load(const char* key, nvs_device_config_t *config) {
    esp_err_t err = ESP_OK;
    nvs_handle_t nvs_handle;

    if ((err = nvs_open("storage", NVS_READONLY, &nvs_handle)) != ESP_OK) {
        return err;
    }

    size_t required_size = 0;
    if ((err = nvs_get_blob(nvs_handle, key, NULL, &required_size)) != ESP_OK) {
        nvs_close(nvs_handle);
        return err;
    }

    nvs_device_config_t *_cfg = (nvs_device_config_t *) malloc(required_size + sizeof(nvs_device_config_t));
    if (required_size > 0) {
        err = nvs_get_blob(nvs_handle, key, _cfg, &required_size);
        if (err != ESP_OK) return err;
    }

    memcpy(config, _cfg, sizeof(nvs_device_config_t));
    free(_cfg);

    nvs_close(nvs_handle);
    return err;
}

esp_err_t NVSManager::save_default_config() {
    return this->save("default_config", &this->default_config);
}

esp_err_t NVSManager::load_default_config() {
    return this->load("default_config", &this->default_config);
}

esp_err_t NVSManager::save_config(nvs_device_config_t *config) {
    bool config_changed = false;

    if (strncmp(this->config.wifi_ssid, config->wifi_ssid, WIFI_MAX_SSID_LEN) == 0) {
        TB_LOGI(TAG, "wifi_ssid no change");
    } else {
        TB_LOGI(TAG, "%s -> %s", this->config.wifi_ssid, config->wifi_ssid);
        strlcpy(this->config.wifi_ssid, config->wifi_ssid, WIFI_MAX_SSID_LEN);
        config_changed = true;
    }

    if (strncmp(this->config.wifi_pass, config->wifi_pass, WIFI_MAX_SSID_LEN) == 0) {
        TB_LOGI(TAG, "wifi_pass no change");
    } else {
        TB_LOGI(TAG, "%s -> %s", this->config.wifi_pass, config->wifi_pass);
        strlcpy(this->config.wifi_pass, config->wifi_pass, WIFI_MAX_SSID_LEN);
        config_changed = true;
    }

    if (this->config.log_level == config->log_level) {
        TB_LOGI(TAG, "log_level no change");
    } else {
        TB_LOGI(TAG, "%d -> %d", this->config.log_level, config->log_level);
        this->config.log_level = config->log_level;
        config_changed = true;
    }

    return config_changed ? this->save("config", &this->config) : ESP_FAIL;
}

esp_err_t NVSManager::load_config() {
    esp_err_t err = this->load("config", &this->config);
    if (err == ESP_ERR_NVS_NOT_FOUND) {
        TB_LOGW(TAG, "config not found. Loading default config");
        err = this->load("default_config", &this->config);
    }
    return err;
}

nvs_device_config_t *NVSManager::get_default_config() {
    return &this->default_config;
}

nvs_device_config_t *NVSManager::get_config() {
    return &this->config;
}
