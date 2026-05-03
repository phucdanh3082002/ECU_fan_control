idf_component_register(SRCS "main.c"
                            "fan_control.c"
                            "diagnostics.c"
                            "system_types.c"
                       INCLUDE_DIRS "."
                       REQUIRES esp_adc esp_driver_gpio esp_driver_ledc)
