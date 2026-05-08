#ifndef HAL_SENSOR_INPUT_REAL_H
#define HAL_SENSOR_INPUT_REAL_H

#include "esp_err.h"
#include "sensor_input.h"

esp_err_t sensor_input_real_init(void);
const SensorInputHal *sensor_input_real_get_hal(void);
int sensor_input_real_get_adc_raw(void);
void sensor_input_real_set_calibration_offset(float offset_c);
void sensor_input_real_ina219_diagnostic(void);

#endif
