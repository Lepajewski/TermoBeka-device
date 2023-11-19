#include "utility_commands.h"
#include "ui_commands.h"
#include "sd_commands.h"
#include "wifi_commands.h"

#include "commands.h"


void register_system_commands() {
    register_system_utility();
    register_user_interface();
    register_sd_card();
    register_wifi();
}
