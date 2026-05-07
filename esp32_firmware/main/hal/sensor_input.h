#ifndef HAL_SENSOR_INPUT_H
#define HAL_SENSOR_INPUT_H

#include <stdbool.h>

#include "system_types.h"

typedef struct {
    float (*read_temperature)(void);
    float (*read_current)(void);
    int (*read_rpm)(void);
    bool (*is_sensor_valid)(void);
    bool (*is_adc_input_enabled)(void);
} SensorInputHal;

static inline SensorInput sensor_input_hal_read(const SensorInputHal *hal)
{
    return (SensorInput){
        .temperature = hal->read_temperature(),
        .current = hal->read_current(),
        .rpm = hal->read_rpm(),
        .sensorValid = hal->is_sensor_valid(),
        .useAdcInput = hal->is_adc_input_enabled(),
    };
}

#endif
