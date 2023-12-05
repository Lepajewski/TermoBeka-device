cmake_minimum_required(VERSION 3.16.0)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)

list(APPEND EXTRA_COMPONENTS_DIRS
    lib
)

include_directories(
    .pio/libdeps/debug/inipp/inipp #this lib doesnt have include folder :/
)

project(TermoBeka-device)
