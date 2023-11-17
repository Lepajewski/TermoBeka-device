#include <cstring>

#include "logger.h"
#include "system_manager.h"

#include "sd_manager.h"


extern SystemManager sysMgr;


const char * const TAG = "SDMgr";


SDManager::SDManager() {

}

SDManager::~SDManager() {
    end();
}

void SDManager::begin() {
    this->event_queue_handle = sysMgr.get_event_queue();
    this->sd_queue_handle = sysMgr.get_sd_queue();

    this->card.begin();

    TB_LOGI(TAG, "SD Card mounted");
}

void SDManager::end() {
    this->card.end();
    TB_LOGI(TAG, "SD Card unmounted");
}

char *SDManager::get_args(SDEvent *evt) {
    uint8_t mount_point_len = strlen(this->card.get_mount_point()) + 1;
    uint16_t payload_len = strlen(reinterpret_cast<char *>(evt->payload)) + 1;
    const char *root_char = "/";
    char *path = new char[payload_len + mount_point_len + strlen(root_char) + 1];
    strncpy(path, this->card.get_mount_point(), mount_point_len);
    strncat(path, root_char, strlen(root_char) + 1);
    strncat(path, reinterpret_cast<char *>(evt->payload), payload_len);
    return path;
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
            char *path = get_args(evt);
            TB_LOGI(TAG, "list files, path: %s", path);
            this->card.ls(path);

            delete [] path;
            break;
        }
        case SDEventType::CAT:
        {
            char *path = get_args(evt);
            TB_LOGI(TAG, "cat, path: %s", path);
            this->card.cat(path);

            delete [] path;
            break;
        }
        case SDEventType::MKDIR:
        {
            char *path = get_args(evt);
            TB_LOGI(TAG, "mkdir %s", path);
            this->card.mkdir(path);

            delete [] path;
            break;
        }
        case SDEventType::TOUCH:
        {
            char *path = get_args(evt);
            TB_LOGI(TAG, "touch %s", path);
            this->card.touch(path);

            delete [] path;
            break;
        }
        case SDEventType::RM_FILE:
        {
            char *path = get_args(evt);
            TB_LOGI(TAG, "rm_file %s", path);
            this->card.rm(path);

            delete [] path;
            break;
        }
        case SDEventType::RM_DIR:
        {
            char *path = get_args(evt);
            TB_LOGI(TAG, "rm_dir %s", path);
            this->card.rmdir(path);

            delete [] path;
            break;
        }
        case SDEventType::SAVE_TO_FILE:
        {
            char *path_line = get_args(evt);
            char *line;
            strtok_r(path_line, " ", &line);
            TB_LOGI(TAG, "save_to_file: %s buf: %s", path_line, line);
            
            this->card.save_buf(path_line, line);

            delete [] path_line;
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
