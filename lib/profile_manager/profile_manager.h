#ifndef LIB_PROFILE_MANAGER_PROFILE_MANAGER_H_
#define LIB_PROFILE_MANAGER_PROFILE_MANAGER_H_


#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"

#include "tb_event.h"
#include "system_manager.h"
#include "profile_type.h"
#include "profile.h"


class ProfileManager {
 private:
    SystemManager *sysMgr;
    QueueHandle_t *event_queue_handle;
    QueueHandle_t *profile_queue_handle;
    profile_config_t config;
    Profile *profile;

    void process_profile_queue_event(ProfileEvent *evt);
    void poll_profile_queue_events();
    void poll_running_profile_events();

    profile_event_response process_new_profile(profile_t *profile);
    profile_event_response start_profile();
    profile_event_response stop_profile();
    profile_event_response resume_profile();
    profile_event_response end_profile();
    void print_profile_info();

    void send_evt(Event *evt);
    void send_evt_start();
    void send_evt_stop();
    void send_evt_resume();
    void send_evt_end();
    void send_evt_response(profile_event_response response);
    void send_evt_update();
 public:
    ProfileManager();
    ~ProfileManager();

    void begin();
    void end();
    void process_events();
};


#endif  // LIB_PROFILE_MANAGER_PROFILE_MANAGER_H_
