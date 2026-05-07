#include "unity.h"

#include "fan_control.h"

static void assert_fan_command(float temperature, FanMode expected_mode, int expected_duty)
{
    const FanCommand command = fan_control_calculate(temperature);

    TEST_ASSERT_EQUAL_INT(expected_mode, command.fanMode);
    TEST_ASSERT_EQUAL_INT(expected_duty, command.dutyCycle);
}

void test_fan_control_below_40_is_off(void)
{
    assert_fan_command(39.9f, FAN_OFF, 0);
}

void test_fan_control_40_to_below_70_is_low(void)
{
    assert_fan_command(40.0f, FAN_LOW, 40);
    assert_fan_command(69.9f, FAN_LOW, 40);
}

void test_fan_control_70_to_below_90_is_medium(void)
{
    assert_fan_command(70.0f, FAN_MEDIUM, 70);
    assert_fan_command(89.9f, FAN_MEDIUM, 70);
}

void test_fan_control_90_and_above_is_high(void)
{
    assert_fan_command(90.0f, FAN_HIGH, 100);
    assert_fan_command(100.0f, FAN_HIGH, 100);
}
