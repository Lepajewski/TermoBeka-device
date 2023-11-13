idf_component_register(
    SRCS
        "drivers/sd_card_driver.c"
        "sd_card.cpp"
        "sd_manager.cpp"
        "sd_manager_task.cpp"
    INCLUDE_DIRS "."
    REQUIRES
        fatfs
        global_config
        logger
        system_manager
)