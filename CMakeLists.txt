cmake_minimum_required(VERSION 3.16.0)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)

list(APPEND EXTRA_COMPONENTS_DIRS
    lib/global_config
    lib/logger
    lib/nvs_manager
    lib/system_manager
    lib/peripherals
    lib/ui_manager
)

project(TermoBeka-device)
