idf_component_register(
    SRCS "drivers/buzzer_driver.c" "buzzer.cpp" "ui_manager.cpp" "ui_manager_task.cpp"
    REQUIRES "driver/gpio"
)