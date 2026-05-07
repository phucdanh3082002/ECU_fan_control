if(NOT DEFINED ECU_SENSOR_HAL)
    set(ECU_SENSOR_HAL "SIM")
endif()

set(SENSOR_INPUT_HAL_SRCS "")
set(SENSOR_INPUT_HAL_DEFS "")

if(ECU_SENSOR_HAL STREQUAL "SIM")
    list(APPEND SENSOR_INPUT_HAL_SRCS "hal/sensor_input_sim.c")
    list(APPEND SENSOR_INPUT_HAL_DEFS ECU_SENSOR_HAL_SIM=1)
elseif(ECU_SENSOR_HAL STREQUAL "REAL")
    list(APPEND SENSOR_INPUT_HAL_SRCS "hal/sensor_input_real.c")
    list(APPEND SENSOR_INPUT_HAL_DEFS ECU_SENSOR_HAL_REAL=1)
else()
    message(FATAL_ERROR "Unsupported ECU_SENSOR_HAL=${ECU_SENSOR_HAL}")
endif()

idf_component_register(SRCS "main.c"
                            "app_tasks.c"
                            "fan_control.c"
                            "diagnostics.c"
                            "system_types.c"
                            "uart_protocol.c"
                            ${SENSOR_INPUT_HAL_SRCS}
                       INCLUDE_DIRS "." "hal"
                       REQUIRES esp_adc esp_driver_gpio esp_driver_i2c esp_driver_ledc esp_driver_uart)

target_compile_definitions(${COMPONENT_LIB} PRIVATE ${SENSOR_INPUT_HAL_DEFS})
