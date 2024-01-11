#ifndef LIB_UI_MANAGER_UI_SYSTEM_STATE_H_
#define LIB_UI_MANAGER_UI_SYSTEM_STATE_H_

#include <string>
#include <functional>
#include <inttypes.h>

enum class ProfileState { unloaded, loaded, loading, starting, running, stopping };

enum class MessageType { profile_selected, profile_started, profile_stopped, none };

struct WaitingMessageArgs {
    MessageType type = MessageType::none;
    std::function<bool(void)> waiting_function;
    std::function<bool(void)> success_function;
};

struct ProfileInfo {
    std::string selected_profile = "";
    ProfileState profile_state = ProfileState::unloaded;

    uint32_t profile_time = 0;
    uint32_t profile_duration = 0;

    int32_t avg_temperature = 0;

    float get_profile_run_percentage() {
        return (float) profile_time / (float) profile_duration;
    }
};

struct UISystemState {
    int wifi_rssi = 0;
    
    WaitingMessageArgs waiting_message_args;
    ProfileInfo profile_info;

};

#endif