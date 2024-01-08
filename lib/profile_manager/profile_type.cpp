#include "profile_type.h"

std::string profile_to_string(profile_t *profile) {
    std::string res = "";
    for (profile_point point : profile->points) {
        if (point.temperature < 0) {
            break;
        }
        
        res += std::to_string(point.temperature) + "," + std::to_string(point.time_ms) + "\n";
    }
    return res;
}

profile_t string_to_profile(std::string string) {
    profile_t profile = {};
    for (uint8_t i = 0; i < PROFILE_MAX_VERTICES; i++) {
        profile.points[i] = {-1, UINT32_MAX};
    }
    
    size_t new_line_pos = std::string::npos;
    int i = 0;
    while ((new_line_pos = string.find("\n")) != std::string::npos && i < PROFILE_MAX_VERTICES) {
        size_t comma_pos = string.find(",");
        std::string temperature = string.substr(0, comma_pos);
        std::string time = string.substr(comma_pos + 1, new_line_pos - comma_pos - 1);

        profile.points[i].temperature = std::stoi(temperature);
        profile.points[i].time_ms = std::stoul(time);

        i++;
        string.erase(0, new_line_pos+1);
    }
    return profile;
}