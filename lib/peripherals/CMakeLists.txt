idf_component_register(
    SRCS
        "drivers/buzzer_driver.c"
        "buzzer.cpp"
    PRIV_REQUIRES
        "driver/gpio"
)