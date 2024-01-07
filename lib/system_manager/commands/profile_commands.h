#ifndef LIB_SYSTEM_MANAGER_PROFILE_COMMANDS_H_
#define LIB_SYSTEM_MANAGER_PROFILE_COMMANDS_H_


esp_err_t send_to_profile_queue(ProfileEvent *evt);
void register_profile();


#endif  // LIB_SYSTEM_MANAGER_PROFILE_COMMANDS_H_
