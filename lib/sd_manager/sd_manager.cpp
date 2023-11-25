#include <cstring>

#include "logger.h"

#include "sd_manager.h"


const char * const TAG = "SDMgr";


SDManager::SDManager() {
}

SDManager::~SDManager() {
    end();
}

void SDManager::begin() {
    this->sysMgr = get_system_manager();
    this->event_queue_handle = this->sysMgr->get_event_queue();
    this->sd_queue_handle = this->sysMgr->get_sd_queue();

    this->card.begin();

    TB_LOGI(TAG, "SD Card mounted");
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
            TB_LOGI(TAG, "mount card");
            this->card.begin();
            break;
        }
        case SDEventType::UNMOUNT_CARD:
        {
            TB_LOGI(TAG, "unmount card");
            this->card.end();
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
        case SDEventType::NONE:
        default:
            break;
    }
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

void SDManager::process_events() {
    poll_sd_events();
}
