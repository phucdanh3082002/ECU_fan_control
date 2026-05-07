#include "unity.h"

void test_fan_control_below_40_is_off(void);
void test_fan_control_40_to_below_70_is_low(void);
void test_fan_control_70_to_below_90_is_medium(void);
void test_fan_control_90_and_above_is_high(void);

void test_diagnostics_init_clears_context(void);
void test_diagnostics_returns_normal_status_when_no_fault_is_active(void);
void test_diagnostics_sensor_invalid_enters_safe_mode(void);
void test_diagnostics_temperature_out_of_range_is_sensor_fault(void);
void test_diagnostics_over_temperature_forces_high_fault_mode(void);
void test_diagnostics_over_current_turns_fan_off_fault_mode(void);
void test_diagnostics_current_at_limit_does_not_fault(void);
void test_diagnostics_over_current_has_priority_over_over_temperature(void);
void test_diagnostics_fan_stall_latches_after_timeout(void);
void test_diagnostics_fan_stall_timer_resets_when_rpm_recovers(void);
void test_diagnostics_recovery_requires_three_clear_cycles(void);
void test_diagnostics_new_active_fault_replaces_latched_fault(void);

void test_uart_protocol_parses_status_commands(void);
void test_uart_protocol_parses_numeric_commands(void);
void test_uart_protocol_parses_boolean_commands(void);
void test_uart_protocol_rejects_invalid_commands(void);
void test_uart_protocol_formats_status_response(void);
void test_uart_protocol_formats_error_response(void);

void test_system_types_convert_fan_modes_to_strings(void);
void test_system_types_convert_fault_codes_to_strings(void);
void test_system_types_convert_states_to_strings(void);

void test_sensor_input_sim_defaults_to_adc_mode(void);
void test_sensor_input_sim_selects_adc_or_uart_temperature(void);
void test_sensor_input_sim_updates_current_and_rpm(void);
void test_sensor_input_sim_combines_uart_valid_and_button_state(void);

void setUp(void)
{
}

void tearDown(void)
{
}

int main(void)
{
    UNITY_BEGIN();

    RUN_TEST(test_fan_control_below_40_is_off);
    RUN_TEST(test_fan_control_40_to_below_70_is_low);
    RUN_TEST(test_fan_control_70_to_below_90_is_medium);
    RUN_TEST(test_fan_control_90_and_above_is_high);

    RUN_TEST(test_diagnostics_init_clears_context);
    RUN_TEST(test_diagnostics_returns_normal_status_when_no_fault_is_active);
    RUN_TEST(test_diagnostics_sensor_invalid_enters_safe_mode);
    RUN_TEST(test_diagnostics_temperature_out_of_range_is_sensor_fault);
    RUN_TEST(test_diagnostics_over_temperature_forces_high_fault_mode);
    RUN_TEST(test_diagnostics_over_current_turns_fan_off_fault_mode);
    RUN_TEST(test_diagnostics_current_at_limit_does_not_fault);
    RUN_TEST(test_diagnostics_over_current_has_priority_over_over_temperature);
    RUN_TEST(test_diagnostics_fan_stall_latches_after_timeout);
    RUN_TEST(test_diagnostics_fan_stall_timer_resets_when_rpm_recovers);
    RUN_TEST(test_diagnostics_recovery_requires_three_clear_cycles);
    RUN_TEST(test_diagnostics_new_active_fault_replaces_latched_fault);

    RUN_TEST(test_uart_protocol_parses_status_commands);
    RUN_TEST(test_uart_protocol_parses_numeric_commands);
    RUN_TEST(test_uart_protocol_parses_boolean_commands);
    RUN_TEST(test_uart_protocol_rejects_invalid_commands);
    RUN_TEST(test_uart_protocol_formats_status_response);
    RUN_TEST(test_uart_protocol_formats_error_response);

    RUN_TEST(test_system_types_convert_fan_modes_to_strings);
    RUN_TEST(test_system_types_convert_fault_codes_to_strings);
    RUN_TEST(test_system_types_convert_states_to_strings);

    RUN_TEST(test_sensor_input_sim_defaults_to_adc_mode);
    RUN_TEST(test_sensor_input_sim_selects_adc_or_uart_temperature);
    RUN_TEST(test_sensor_input_sim_updates_current_and_rpm);
    RUN_TEST(test_sensor_input_sim_combines_uart_valid_and_button_state);

    return UNITY_END();
}
