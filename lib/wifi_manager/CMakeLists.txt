idf_component_register{
    SRCS
        "ntp_driver.c"
        "wifi_driver.c"
        "wifi_manager.cpp"
        "wifi_manager_task.cpp"
    REQUIRES
        esp_wifi
        esp_sntp
        global_config
        logger
        system_manager
}
