#include "sensor_input_sim.h"

#define SIM_DEFAULT_TEMPERATURE_C 0.0f
#define SIM_DEFAULT_CURRENT_A 0.5f
#define SIM_DEFAULT_RPM 1200

static float s_adcTemperatureC;
static float s_uartTemperatureC;
static float s_currentA;
static int s_rpm;
static bool s_uartSensorValid;
static bool s_buttonPressed;
static bool s_useAdcInput;

static float sim_read_temperature(void)
{
    return s_useAdcInput ? s_adcTemperatureC : s_uartTemperatureC;
}

static float sim_read_current(void)
{
    return s_currentA;
}

static int sim_read_rpm(void)
{
    return s_rpm;
}

static bool sim_is_sensor_valid(void)
{
    return s_uartSensorValid && !s_buttonPressed;
}

static bool sim_is_adc_input_enabled(void)
{
    return s_useAdcInput;
}

static const SensorInputHal s_sensorInputSimHal = {
    .read_temperature = sim_read_temperature,
    .read_current = sim_read_current,
    .read_rpm = sim_read_rpm,
    .is_sensor_valid = sim_is_sensor_valid,
    .is_adc_input_enabled = sim_is_adc_input_enabled,
};

void sensor_input_sim_init(void)
{
    s_adcTemperatureC = SIM_DEFAULT_TEMPERATURE_C;
    s_uartTemperatureC = SIM_DEFAULT_TEMPERATURE_C;
    s_currentA = SIM_DEFAULT_CURRENT_A;
    s_rpm = SIM_DEFAULT_RPM;
    s_uartSensorValid = true;
    s_buttonPressed = false;
    s_useAdcInput = true;
}

const SensorInputHal *sensor_input_sim_get_hal(void)
{
    return &s_sensorInputSimHal;
}

void sensor_input_sim_set_adc_temperature(float temperature_c)
{
    s_adcTemperatureC = temperature_c;
}

void sensor_input_sim_set_uart_temperature(float temperature_c)
{
    s_uartTemperatureC = temperature_c;
}

void sensor_input_sim_set_current(float current_a)
{
    s_currentA = current_a;
}

void sensor_input_sim_set_rpm(int rpm)
{
    s_rpm = rpm;
}

void sensor_input_sim_set_uart_sensor_valid(bool sensor_valid)
{
    s_uartSensorValid = sensor_valid;
}

void sensor_input_sim_set_button_pressed(bool button_pressed)
{
    s_buttonPressed = button_pressed;
}

void sensor_input_sim_set_use_adc_input(bool use_adc_input)
{
    s_useAdcInput = use_adc_input;
}
