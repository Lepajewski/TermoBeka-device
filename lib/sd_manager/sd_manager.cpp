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

void SDManager::process_sd_event(SDEvent *evt) {
    switch (evt->type) {
        case SDEventType::MOUNT_CARD:
        {
            TB_LOGI(TAG, "mount card");
            this->card.begin();
            break;
        }
        case SDEventType::LIST_FILES:
        {
            char *path = new char[strlen(reinterpret_cast<char *>(evt->payload)) + strlen(this->card.get_mount_point()) + 1];
            strcpy(path, this->card.get_mount_point());
            strcat(path, reinterpret_cast<char *>(evt->payload));

            TB_LOGI(TAG, "list files, path: %s", path);
            this->card.list_files(path);

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
