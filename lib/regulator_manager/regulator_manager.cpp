#include "inttypes.h"

#include "global_config.h"
#include "logger.h"
#include "regulator_type.h"
#include "regulator_timer.h"
#include "regulator_manager.h"


const char * const TAG = "RegulatorMgr";


RegulatorManager::RegulatorManager()
{
    regulator_config_t config = {};
    config.min_temp = PROFILE_MIN_TEMPERATURE;
    config.max_temp = PROFILE_MAX_TEMPERATURE;
    config.sampling_rate = REGULATOR_SAMPLING_RATE_MS;
    config.update_interval = REGULATOR_UPDATE_TIMER_INTERVAL_MS;

    this->regulator = new Regulator(config);
    this->setup();
}

RegulatorManager::~RegulatorManager() {
    this->end();
}

void RegulatorManager::end() {
    delete this->regulator;
}

void RegulatorManager::setup() {
    this->sysMgr = get_system_manager();
    this->event_queue_handle = this->sysMgr->get_event_queue();
    this->regulator_queue_handle = this->sysMgr->get_regulator_queue();
}

void RegulatorManager::process_regulator_event(RegulatorEvent *evt) {
    TB_LOGI(TAG, "%s", regulator_event_type_to_s(evt->type));
    switch (evt->type) {
        case RegulatorEventType::START:
        {
            if (this->start_regulator() != ESP_OK) {
                TB_LOGE(TAG, "fail to start, already running");
            }
            break;
        }
        case RegulatorEventType::STOP:
        {
            if (this->stop_regulator() != ESP_OK) {
                TB_LOGE(TAG, "fail to stop, already stopped");
            }
            break;
        }
        case RegulatorEventType::TEMPERATURE_UPDATE:
        {
            RegulatorEventTemperatureUpdate *payload = reinterpret_cast<RegulatorEventTemperatureUpdate*>(evt->payload);
            if (this->process_temperature_update(payload->temperature) != ESP_OK) {
                TB_LOGI(TAG, "fail to update temperature, not running");
            }
            break;
        }
        case RegulatorEventType::NONE:
        default:
            break;
    }
}

void RegulatorManager::poll_regulator_events() {
    RegulatorEvent evt = {};

    while (uxQueueMessagesWaiting(*this->regulator_queue_handle)) {
        if (xQueueReceive(*this->regulator_queue_handle, &evt, pdMS_TO_TICKS(10)) == pdPASS) {
            TB_LOGI(TAG, "new event, type: %d", evt.type);
            this->process_regulator_event(&evt);
        }
    }
}

void RegulatorManager::poll_running_regulator_events() {
    EventBits_t bits = xEventGroupWaitBits(
        *this->regulator->get_regulator_event_group(),
        BITS_REGULATOR_CONTROL,
        pdTRUE,
        pdFALSE,
        pdMS_TO_TICKS(1)
    );

    if ((bits & BIT_REGULATOR_START) == BIT_REGULATOR_START) {
        TB_LOGI(TAG, "regulator started");
        this->send_evt_start();
    } else if ((bits & BIT_REGULATOR_STOP) == BIT_REGULATOR_STOP) {
        TB_LOGI(TAG, "regulator stopped");
        this->send_evt_stop();
    } else if ((bits & BIT_REGULATOR_UPDATE) == BIT_REGULATOR_UPDATE) {
        TB_LOGI(TAG, "regulator update");
    }

    if (bits) {
        this->send_evt_update();
    }
}

void RegulatorManager::process_events() {
    this->poll_regulator_events();
    this->poll_running_regulator_events();
    this->regulator->process_regulator();
}

esp_err_t RegulatorManager::start_regulator() {
    return this->regulator->start();
}

esp_err_t RegulatorManager::stop_regulator() {
    return this->regulator->stop();
}

esp_err_t RegulatorManager::process_temperature_update(int16_t temperature) {
    if (!this->regulator->is_running()) {
        return ESP_FAIL;
    }

    TB_LOGI(TAG, "regulating to: %.2f*C", (float)temperature / 100.0f);

    return ESP_OK;
}

void RegulatorManager::send_evt(Event *evt) {
    evt->origin = EventOrigin::REGULATOR;
    if (xQueueSend(*this->event_queue_handle, &*evt, portMAX_DELAY) != pdTRUE) {
        TB_LOGE(TAG, "event send fail");
    }
}

void RegulatorManager::send_evt_start() {
    Event evt = {};
    evt.type = EventType::REGULATOR_START;
    this->send_evt(&evt);
}

void RegulatorManager::send_evt_stop() {
    Event evt = {};
    evt.type = EventType::REGULATOR_STOP;
    this->send_evt(&evt);
}

void RegulatorManager::send_evt_update() {
    Event evt = {};
    evt.type = EventType::REGULATOR_UPDATE;
    EventRegulatorUpdate payload = {};
    payload.info = this->regulator->get_regulator_run_info();
    memcpy(&evt.payload, &payload.buffer, sizeof(EventRegulatorUpdate));
    this->send_evt(&evt);
}

