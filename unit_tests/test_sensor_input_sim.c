#include "unity.h"

#include "sensor_input_sim.h"

void test_sensor_input_sim_defaults_to_adc_mode(void)
{
    sensor_input_sim_init();

    const SensorInputHal *hal = sensor_input_sim_get_hal();
    const SensorInput input = sensor_input_hal_read(hal);

    TEST_ASSERT_FLOAT_WITHIN(0.001f, 0.0f, input.temperature);
    TEST_ASSERT_FLOAT_WITHIN(0.001f, 0.5f, input.current);
    TEST_ASSERT_EQUAL_INT(1200, input.rpm);
    TEST_ASSERT_TRUE(input.sensorValid);
    TEST_ASSERT_TRUE(input.useAdcInput);
}

void test_sensor_input_sim_selects_adc_or_uart_temperature(void)
{
    sensor_input_sim_init();

    const SensorInputHal *hal = sensor_input_sim_get_hal();

    sensor_input_sim_set_adc_temperature(25.0f);
    sensor_input_sim_set_uart_temperature(80.0f);
    sensor_input_sim_set_use_adc_input(true);
    TEST_ASSERT_FLOAT_WITHIN(0.001f, 25.0f, sensor_input_hal_read(hal).temperature);

    sensor_input_sim_set_use_adc_input(false);
    TEST_ASSERT_FLOAT_WITHIN(0.001f, 80.0f, sensor_input_hal_read(hal).temperature);
}

void test_sensor_input_sim_updates_current_and_rpm(void)
{
    sensor_input_sim_init();

    const SensorInputHal *hal = sensor_input_sim_get_hal();

    sensor_input_sim_set_current(2.1f);
    sensor_input_sim_set_rpm(0);

    const SensorInput input = sensor_input_hal_read(hal);

    TEST_ASSERT_FLOAT_WITHIN(0.001f, 2.1f, input.current);
    TEST_ASSERT_EQUAL_INT(0, input.rpm);
}

void test_sensor_input_sim_combines_uart_valid_and_button_state(void)
{
    sensor_input_sim_init();

    const SensorInputHal *hal = sensor_input_sim_get_hal();

    TEST_ASSERT_TRUE(sensor_input_hal_read(hal).sensorValid);

    sensor_input_sim_set_button_pressed(true);
    TEST_ASSERT_FALSE(sensor_input_hal_read(hal).sensorValid);

    sensor_input_sim_set_button_pressed(false);
    TEST_ASSERT_TRUE(sensor_input_hal_read(hal).sensorValid);

    sensor_input_sim_set_uart_sensor_valid(false);
    TEST_ASSERT_FALSE(sensor_input_hal_read(hal).sensorValid);
}
