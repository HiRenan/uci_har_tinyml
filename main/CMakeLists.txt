idf_component_register(
    SRCS "main.c"
    INCLUDE_DIRS "."
    REQUIRES
        freertos
        log
        esp_common
        esp_system
        esp_driver_gpio
        esp_driver_i2c
)
