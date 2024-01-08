#ifndef LIB_UI_MANAGER_UI_SYSTEM_STATE_H_
#define LIB_UI_MANAGER_UI_SYSTEM_STATE_H_

#include <string>
#include <functional>

enum class ProfileState { unloaded, loaded, loading, starting, running, stopping };

enum class MessageType { profile_selected, profile_started, profile_stopped, none };

struct WaitingMessageArgs {
    MessageType type = MessageType::none;
    std::function<bool(void)> waiting_function;
    std::function<bool(void)> success_function;
};

struct UISystemState {
    int wifi_rssi = 0;
    std::string selected_profile = "";
    ProfileState profile_state = ProfileState::unloaded;
    WaitingMessageArgs waiting_message_args;
};

#endif