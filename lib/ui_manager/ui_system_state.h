#ifndef LIB_UI_MANAGER_UI_SYSTEM_STATE_H_
#define LIB_UI_MANAGER_UI_SYSTEM_STATE_H_

#include <string>

enum class ProfileState { unloaded, loaded, loading, running };

struct UISystemState {
    int wifi_rssi = 0;
    std::string selected_profile = "";
    ProfileState profile_state = ProfileState::unloaded;
};


#endif