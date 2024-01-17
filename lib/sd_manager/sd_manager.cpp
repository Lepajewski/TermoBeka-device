#include <cstring>

#include "logger.h"

#include "sd_manager.h"
#include "freertos/queue.h"
#include "global_config.h"

const char * const TAG = "SDMgr";


SDManager::SDManager() {
    this->begin();
}

SDManager::~SDManager() {
    end();
}

void SDManager::begin() {
    this->sysMgr = get_system_manager();
    this->event_queue_handle = this->sysMgr->get_event_queue();
    this->sd_queue_handle = this->sysMgr->get_sd_queue();
    this->sd_ring_buf_handle = this->sysMgr->get_sd_ring_buf();
}

void SDManager::end() {
    this->card.end();
    TB_LOGI(TAG, "SD Card unmounted");
}

char *SDManager::make_path(const char *path) {
    uint8_t mount_point_len = strlen(this->card.get_mount_point()) + 1;
    uint16_t payload_len = strlen(path) + 1;
    const char *root_char = "/";
    char *new_path = new char[payload_len + mount_point_len + strlen(root_char) + 1];
    strlcpy(new_path, this->card.get_mount_point(), mount_point_len);
    strncat(new_path, root_char, strlen(root_char) + 1);
    strncat(new_path, path, payload_len);
    return new_path;
}

void SDManager::process_sd_event(SDEvent *evt) {
    switch (evt->type) {
        case SDEventType::MOUNT_CARD:
        {
            this->process_mount_card();
            break;
        }
        case SDEventType::UNMOUNT_CARD:
        {
            this->process_unmount_card();
            break;
        }
        case SDEventType::LS:
        {
            SDEventPathArg *payload = reinterpret_cast<SDEventPathArg*>(evt->payload);
            char *path = this->make_path(payload->path);
            TB_LOGI(TAG, "list files, path: %s", path);
            this->card.ls(path);

            delete [] path;
            break;
        }
        case SDEventType::CAT:
        {
            SDEventPathArg *payload = reinterpret_cast<SDEventPathArg*>(evt->payload);
            char *path = this->make_path(payload->path);
            TB_LOGI(TAG, "cat, path: %s", path);
            this->card.cat(path);

            delete [] path;
            break;
        }
        case SDEventType::MKDIR:
        {
            SDEventPathArg *payload = reinterpret_cast<SDEventPathArg*>(evt->payload);
            char *path = this->make_path(payload->path);
            TB_LOGI(TAG, "mkdir %s", path);
            this->card.mkdir(path);

            delete [] path;
            break;
        }
        case SDEventType::TOUCH:
        {
            SDEventPathArg *payload = reinterpret_cast<SDEventPathArg*>(evt->payload);
            char *path = this->make_path(payload->path);
            TB_LOGI(TAG, "touch %s", path);
            this->card.touch(path);

            delete [] path;
            break;
        }
        case SDEventType::RM_FILE:
        {
            SDEventPathArg *payload = reinterpret_cast<SDEventPathArg*>(evt->payload);
            char *path = this->make_path(payload->path);
            TB_LOGI(TAG, "rm_file %s", path);
            this->card.rm(path);

            delete [] path;
            break;
        }
        case SDEventType::RM_DIR:
        {
            SDEventPathArg *payload = reinterpret_cast<SDEventPathArg*>(evt->payload);
            char *path = this->make_path(payload->path);
            TB_LOGI(TAG, "rm_dir %s", path);
            this->card.rmdir(path);

            delete [] path;
            break;
        }
        case SDEventType::SAVE_TO_FILE:
        {
            SDEventPathBufArg *payload = reinterpret_cast<SDEventPathBufArg*>(evt->payload);
            char *path = this->make_path(payload->params.path);
            TB_LOGI(TAG, "save_to_file: %s buf: %s", path, payload->params.record);
            
            this->card.save_buf(path, payload->params.record);

            delete [] path;
            break;
        }
        case SDEventType::LOAD_CA_CERT:
        {
            SDEventPathArg *payload = reinterpret_cast<SDEventPathArg*>(evt->payload);
            char *path = this->make_path(payload->path);
            TB_LOGI(TAG, "load_ca_cert %s", path);

            if (this->process_load_ca_cert(path) == ESP_OK) {
                this->send_evt_sd_load_ca_file();
            }

            delete [] path;
            break;
        }
        case SDEventType::UI_PROFILE_LIST:
        {
            SDEventPathArg *payload = reinterpret_cast<SDEventPathArg*>(evt->payload);
            char *path = this->make_path(payload->path);

            if (this->process_profile_list(path) == ESP_OK) {
                this->send_evt_sd_response(SDResponse::UI_PROFILE_LIST_SUCCESS);
                this->send_evt_ui_profile_list();
            }
            else {
                this->send_evt_sd_response(SDResponse::UI_PROFILE_LIST_FAIL);
            }

            delete [] path;
            break;
        }
        case SDEventType::CAT_PROFILE:
        {
            SDEventPathArg *payload = reinterpret_cast<SDEventPathArg*>(evt->payload);
            char *path = this->make_path(payload->path);

            if (this->process_profile_cat(path) == ESP_OK) {
                this->send_evt_sd_response(SDResponse::CAT_PROFILE_SUCCESS);
                this->send_evt_sd_profile_load(path);
            }
            else {
                this->send_evt_sd_response(SDResponse::CAT_PROFILE_FAIL);
            }

            delete [] path;
            break;
        }
        case SDEventType::NONE:
        default:
            break;
    }
}

void SDManager::process_mount_card() {
    TB_LOGI(TAG, "mount card");
    esp_err_t err = this->card.begin();
    if (err == ESP_OK) {
        this->send_evt_sd_response(SDResponse::MOUNT_SUCCESS);
        this->send_evt_sd_mounted();
        this->load_and_send_config_ini();
    }
    else {
        this->send_evt_sd_response(SDResponse::MOUNT_FAIL);
    }
}

void SDManager::process_unmount_card() {
    TB_LOGI(TAG, "unmount card");
    if (this->card.end() == ESP_OK) {
        this->send_evt_sd_unmounted();
    }
    send_evt_sd_response(SDResponse::UNMOUNTED);
}

void SDManager::poll_sd_events() {
    SDEvent evt;

    while (uxQueueMessagesWaiting(*this->sd_queue_handle)) {
        if (xQueueReceive(*this->sd_queue_handle, &evt, pdMS_TO_TICKS(10)) == pdPASS) {
            TB_LOGI(TAG, "new event, type: %d", evt.type);
            process_sd_event(&evt);
        }
    }
}

void SDManager::load_and_send_config_ini() {
    Event evt = {};
    evt.type = EventType::SD_CONFIG_LOAD;
    evt.origin = EventOrigin::SD;
    EventSDConfigLoad payload = {};

    char *path = make_path(CONFIG_INI_PATH);
    bool ret = this->card.load_config_ini(path, &payload.config);
    delete [] path;

    if (!ret) {
        TB_LOGE(TAG, "error loading config ini");
        return;
    }

    memcpy(&evt.payload, &payload.buffer, sizeof(EventSDConfigLoad));

    if (xQueueSend(*this->event_queue_handle, &evt, portMAX_DELAY) != pdTRUE) {
        TB_LOGE(TAG, "sending config ini failed");
    }
}

void SDManager::process_events() {
    poll_sd_events();
}

void SDManager::send_evt(Event *evt) {
    evt->origin = EventOrigin::SD;
    if (xQueueSend(*this->event_queue_handle, &*evt, portMAX_DELAY) != pdTRUE) {
        TB_LOGE(TAG, "event send fail");
    }
}

void SDManager::send_evt_sd_mounted() {
    Event evt = {};
    evt.type = EventType::SD_MOUNTED;
    this->send_evt(&evt);
}

void SDManager::send_evt_sd_unmounted() {
    Event evt = {};
    evt.type = EventType::SD_UNMOUNTED;
    this->send_evt(&evt);
}

void SDManager::send_evt_sd_config_load() {
    ;
}

void SDManager::send_evt_sd_load_ca_file() {
    Event evt = {};
    evt.type = EventType::SD_LOAD_CA_FILE;
    this->send_evt(&evt);
}

void SDManager::send_evt_ui_profile_list() {
    Event evt = {};
    evt.type = EventType::UI_PROFILES_LOAD;
    this->send_evt(&evt);
}

void SDManager::send_evt_sd_profile_load(const char *path) {
    Event evt = {};
    evt.type = EventType::SD_PROFILE_LOAD;

    char *name = strrchr(path, '/') + 1;
    size_t name_len = strlen(name);
    size_t min_len = MIN(name_len, PROFILE_NAME_SIZE);

    memcpy(evt.payload, name, min_len);

    this->send_evt(&evt);
}

void SDManager::send_evt_sd_response(SDResponse resp) {
    Event evt = {};
    evt.type = EventType::SD_RESPONSE;
    EventSDResponse response;
    response.response = resp;
    memcpy(evt.payload, response.buffer, EVENT_QUEUE_MAX_PAYLOAD);
    this->send_evt(&evt);
}

esp_err_t SDManager::process_load_ca_cert(const char *path) {
    if (this->card.cat(path) == ESP_OK) {
        char *buf = this->card.get_sd_buf();
        UBaseType_t res = xRingbufferSend(*this->sd_ring_buf_handle, buf, strnlen(buf, SD_OPERATIONS_BUFFER_SIZE), pdMS_TO_TICKS(1000));
        if (res != pdTRUE) {
            TB_LOGE(TAG, "fail to send ca.crt to ringbuf");
        }
        return ESP_OK;
    }
    return ESP_FAIL;
}

esp_err_t SDManager::process_profile_list(const char *path) {
    if (this->card.ls(path) != ESP_OK) {
        return ESP_FAIL;
    }
    
    char *buf = this->card.get_sd_buf();
    UBaseType_t res = xRingbufferSend(*this->sd_ring_buf_handle, buf, strnlen(buf, SD_OPERATIONS_BUFFER_SIZE), pdMS_TO_TICKS(10000));
    if (res != pdTRUE) {
        TB_LOGE(TAG, "fail to send `ls profile_path` to ringbuf");
    }
    return ESP_OK;
}

esp_err_t SDManager::process_profile_cat(const char *path)
{
    if (this->card.cat(path) != ESP_OK) {
        return ESP_FAIL;
    }

    char *buf = this->card.get_sd_buf();
    UBaseType_t res = xRingbufferSend(*this->sd_ring_buf_handle, buf, strnlen(buf, SD_OPERATIONS_BUFFER_SIZE), pdMS_TO_TICKS(10000));
    if (res != pdTRUE) {
        TB_LOGE(TAG, "fail to send `cat profile_path` to ringbuf");
    }
    return ESP_OK;
}
