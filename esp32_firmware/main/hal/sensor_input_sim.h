#ifndef HAL_SENSOR_INPUT_SIM_H
#define HAL_SENSOR_INPUT_SIM_H

#include <stdbool.h>

#include "sensor_input.h"

void sensor_input_sim_init(void);
const SensorInputHal *sensor_input_sim_get_hal(void);

void sensor_input_sim_set_adc_temperature(float temperature_c);
void sensor_input_sim_set_uart_temperature(float temperature_c);
void sensor_input_sim_set_current(float current_a);
void sensor_input_sim_set_rpm(int rpm);
void sensor_input_sim_set_uart_sensor_valid(bool sensor_valid);
void sensor_input_sim_set_button_pressed(bool button_pressed);
void sensor_input_sim_set_use_adc_input(bool use_adc_input);

#endif
