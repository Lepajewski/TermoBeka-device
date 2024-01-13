#ifndef LIB_UI_MANAGER_UI_SYSTEM_STATE_H_
#define LIB_UI_MANAGER_UI_SYSTEM_STATE_H_

#include <string>
#include <functional>
#include <inttypes.h>

enum class ProfileState { unloaded, loaded, loading, starting, running, stopping };

struct WaitingMessageArgs {
    std::function<bool(void)> waiting_function;
    std::function<bool(void)> success_function;
    std::string waiting_strs[2];
    std::string success_strs[2];
    std::string fail_strs[2];
};

struct ProfileInfo {
    std::string selected_profile = "";
    ProfileState profile_state = ProfileState::unloaded;

    uint32_t profile_time = 0;
    uint32_t profile_duration = 0;

    int32_t avg_temperature = 0;
};

struct UISystemState {
    int wifi_rssi = 0;
    
    WaitingMessageArgs waiting_message_args;
    ProfileInfo profile_info;

};

#endif