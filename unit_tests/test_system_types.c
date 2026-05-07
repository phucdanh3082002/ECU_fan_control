#include "unity.h"

#include "system_types.h"

void test_system_types_convert_fan_modes_to_strings(void)
{
    TEST_ASSERT_EQUAL_STRING("OFF", fan_mode_to_string(FAN_OFF));
    TEST_ASSERT_EQUAL_STRING("LOW", fan_mode_to_string(FAN_LOW));
    TEST_ASSERT_EQUAL_STRING("MEDIUM", fan_mode_to_string(FAN_MEDIUM));
    TEST_ASSERT_EQUAL_STRING("HIGH", fan_mode_to_string(FAN_HIGH));
    TEST_ASSERT_EQUAL_STRING("UNKNOWN", fan_mode_to_string((FanMode)99));
}

void test_system_types_convert_fault_codes_to_strings(void)
{
    TEST_ASSERT_EQUAL_STRING("NONE", fault_code_to_string(FAULT_NONE));
    TEST_ASSERT_EQUAL_STRING("SENSOR_FAULT", fault_code_to_string(FAULT_SENSOR));
    TEST_ASSERT_EQUAL_STRING("OVER_TEMPERATURE", fault_code_to_string(FAULT_OVER_TEMP));
    TEST_ASSERT_EQUAL_STRING("OVER_CURRENT", fault_code_to_string(FAULT_OVER_CURRENT));
    TEST_ASSERT_EQUAL_STRING("FAN_STALL", fault_code_to_string(FAULT_FAN_STALL));
    TEST_ASSERT_EQUAL_STRING("UNKNOWN", fault_code_to_string((FaultCode)99));
}

void test_system_types_convert_states_to_strings(void)
{
    TEST_ASSERT_EQUAL_STRING("NORMAL", system_state_to_string(STATE_NORMAL));
    TEST_ASSERT_EQUAL_STRING("SAFE_MODE", system_state_to_string(STATE_SAFE_MODE));
    TEST_ASSERT_EQUAL_STRING("FAULT_MODE", system_state_to_string(STATE_FAULT_MODE));
    TEST_ASSERT_EQUAL_STRING("UNKNOWN", system_state_to_string((SystemState)99));
}
