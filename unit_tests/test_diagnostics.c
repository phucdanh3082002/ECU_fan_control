#include "unity.h"

#include "diagnostics.h"

static SensorInput make_sensor_input(float temperature, float current, int rpm, bool sensor_valid)
{
    return (SensorInput){
        .temperature = temperature,
        .current = current,
        .rpm = rpm,
        .sensorValid = sensor_valid,
        .useAdcInput = false,
    };
}

static FanCommand make_fan_command(FanMode mode, int duty_cycle)
{
    return (FanCommand){
        .fanMode = mode,
        .dutyCycle = duty_cycle,
    };
}

static void assert_status(SystemStatus status, FanMode fan_mode, int duty_cycle, FaultCode fault, SystemState state)
{
    TEST_ASSERT_EQUAL_INT(fan_mode, status.fanMode);
    TEST_ASSERT_EQUAL_INT(duty_cycle, status.dutyCycle);
    TEST_ASSERT_EQUAL_INT(fault, status.fault);
    TEST_ASSERT_EQUAL_INT(state, status.state);
}

void test_diagnostics_init_clears_context(void)
{
    DiagnosticsContext context = {
        .fanStallElapsedMs = 999U,
        .recoveryCycles = 2U,
        .latchedFault = FAULT_OVER_CURRENT,
    };

    diagnostics_init(&context);

    TEST_ASSERT_EQUAL_UINT32(0U, context.fanStallElapsedMs);
    TEST_ASSERT_EQUAL_UINT32(0U, context.recoveryCycles);
    TEST_ASSERT_EQUAL_INT(FAULT_NONE, context.latchedFault);
}

void test_diagnostics_returns_normal_status_when_no_fault_is_active(void)
{
    DiagnosticsContext context;
    diagnostics_init(&context);

    const SensorInput input = make_sensor_input(80.0f, 0.5f, 1200, true);
    const FanCommand command = make_fan_command(FAN_MEDIUM, 70);
    const SystemStatus status = diagnostics_update(&context, &input, command, DIAGNOSTICS_CONTROL_CYCLE_MS);

    assert_status(status, FAN_MEDIUM, 70, FAULT_NONE, STATE_NORMAL);
}

void test_diagnostics_sensor_invalid_enters_safe_mode(void)
{
    DiagnosticsContext context;
    diagnostics_init(&context);

    const SensorInput input = make_sensor_input(80.0f, 0.5f, 1200, false);
    const FanCommand command = make_fan_command(FAN_MEDIUM, 70);
    const SystemStatus status = diagnostics_update(&context, &input, command, DIAGNOSTICS_CONTROL_CYCLE_MS);

    assert_status(status, FAN_HIGH, 100, FAULT_SENSOR, STATE_SAFE_MODE);
}

void test_diagnostics_temperature_out_of_range_is_sensor_fault(void)
{
    DiagnosticsContext context;
    diagnostics_init(&context);

    const FanCommand command = make_fan_command(FAN_OFF, 0);
    const SensorInput below_range = make_sensor_input(-40.1f, 0.5f, 1200, true);
    const SensorInput above_range = make_sensor_input(150.1f, 0.5f, 1200, true);

    assert_status(diagnostics_update(&context,
                                     &below_range,
                                     command,
                                     DIAGNOSTICS_CONTROL_CYCLE_MS),
                  FAN_HIGH,
                  100,
                  FAULT_SENSOR,
                  STATE_SAFE_MODE);

    diagnostics_init(&context);
    assert_status(diagnostics_update(&context,
                                     &above_range,
                                     command,
                                     DIAGNOSTICS_CONTROL_CYCLE_MS),
                  FAN_HIGH,
                  100,
                  FAULT_SENSOR,
                  STATE_SAFE_MODE);
}

void test_diagnostics_over_temperature_forces_high_fault_mode(void)
{
    DiagnosticsContext context;
    diagnostics_init(&context);

    const SensorInput input = make_sensor_input(100.0f, 0.5f, 1200, true);
    const FanCommand command = make_fan_command(FAN_HIGH, 100);
    const SystemStatus status = diagnostics_update(&context, &input, command, DIAGNOSTICS_CONTROL_CYCLE_MS);

    assert_status(status, FAN_HIGH, 100, FAULT_OVER_TEMP, STATE_FAULT_MODE);
}

void test_diagnostics_over_current_turns_fan_off_fault_mode(void)
{
    DiagnosticsContext context;
    diagnostics_init(&context);

    const SensorInput input = make_sensor_input(80.0f, 2.1f, 1200, true);
    const FanCommand command = make_fan_command(FAN_MEDIUM, 70);
    const SystemStatus status = diagnostics_update(&context, &input, command, DIAGNOSTICS_CONTROL_CYCLE_MS);

    assert_status(status, FAN_OFF, 0, FAULT_OVER_CURRENT, STATE_FAULT_MODE);
}

void test_diagnostics_current_at_limit_does_not_fault(void)
{
    DiagnosticsContext context;
    diagnostics_init(&context);

    const SensorInput input = make_sensor_input(80.0f, 2.0f, 1200, true);
    const FanCommand command = make_fan_command(FAN_MEDIUM, 70);
    const SystemStatus status = diagnostics_update(&context, &input, command, DIAGNOSTICS_CONTROL_CYCLE_MS);

    assert_status(status, FAN_MEDIUM, 70, FAULT_NONE, STATE_NORMAL);
}

void test_diagnostics_over_current_has_priority_over_over_temperature(void)
{
    DiagnosticsContext context;
    diagnostics_init(&context);

    const SensorInput input = make_sensor_input(100.0f, 2.1f, 1200, true);
    const FanCommand command = make_fan_command(FAN_HIGH, 100);
    const SystemStatus status = diagnostics_update(&context, &input, command, DIAGNOSTICS_CONTROL_CYCLE_MS);

    assert_status(status, FAN_OFF, 0, FAULT_OVER_CURRENT, STATE_FAULT_MODE);
}

void test_diagnostics_fan_stall_latches_after_timeout(void)
{
    DiagnosticsContext context;
    diagnostics_init(&context);

    const SensorInput input = make_sensor_input(80.0f, 0.5f, 0, true);
    const FanCommand command = make_fan_command(FAN_MEDIUM, 70);

    for (int cycle = 0; cycle < 9; cycle++) {
        const SystemStatus status = diagnostics_update(&context, &input, command, DIAGNOSTICS_CONTROL_CYCLE_MS);
        assert_status(status, FAN_MEDIUM, 70, FAULT_NONE, STATE_NORMAL);
    }

    const SystemStatus status = diagnostics_update(&context, &input, command, DIAGNOSTICS_CONTROL_CYCLE_MS);
    assert_status(status, FAN_OFF, 0, FAULT_FAN_STALL, STATE_FAULT_MODE);
}

void test_diagnostics_fan_stall_timer_resets_when_rpm_recovers(void)
{
    DiagnosticsContext context;
    diagnostics_init(&context);

    const SensorInput stalled = make_sensor_input(80.0f, 0.5f, 0, true);
    const SensorInput spinning = make_sensor_input(80.0f, 0.5f, 1200, true);
    const FanCommand command = make_fan_command(FAN_MEDIUM, 70);

    for (int cycle = 0; cycle < 5; cycle++) {
        (void)diagnostics_update(&context, &stalled, command, DIAGNOSTICS_CONTROL_CYCLE_MS);
    }

    const SystemStatus status = diagnostics_update(&context, &spinning, command, DIAGNOSTICS_CONTROL_CYCLE_MS);

    assert_status(status, FAN_MEDIUM, 70, FAULT_NONE, STATE_NORMAL);
    TEST_ASSERT_EQUAL_UINT32(0U, context.fanStallElapsedMs);
}

void test_diagnostics_ignores_unavailable_rpm_for_fan_stall(void)
{
    DiagnosticsContext context;
    diagnostics_init(&context);

    const SensorInput input = make_sensor_input(80.0f, 0.5f, SENSOR_RPM_UNAVAILABLE, true);
    const FanCommand command = make_fan_command(FAN_MEDIUM, 70);

    for (int cycle = 0; cycle < 12; cycle++) {
        const SystemStatus status = diagnostics_update(&context, &input, command, DIAGNOSTICS_CONTROL_CYCLE_MS);
        assert_status(status, FAN_MEDIUM, 70, FAULT_NONE, STATE_NORMAL);
    }

    TEST_ASSERT_EQUAL_UINT32(0U, context.fanStallElapsedMs);
}

void test_diagnostics_recovery_requires_three_clear_cycles(void)
{
    DiagnosticsContext context;
    diagnostics_init(&context);

    const SensorInput faulted = make_sensor_input(80.0f, 0.5f, 1200, false);
    const SensorInput recovered = make_sensor_input(80.0f, 0.5f, 1200, true);
    const FanCommand command = make_fan_command(FAN_MEDIUM, 70);

    assert_status(diagnostics_update(&context, &faulted, command, DIAGNOSTICS_CONTROL_CYCLE_MS),
                  FAN_HIGH,
                  100,
                  FAULT_SENSOR,
                  STATE_SAFE_MODE);
    assert_status(diagnostics_update(&context, &recovered, command, DIAGNOSTICS_CONTROL_CYCLE_MS),
                  FAN_HIGH,
                  100,
                  FAULT_SENSOR,
                  STATE_SAFE_MODE);
    assert_status(diagnostics_update(&context, &recovered, command, DIAGNOSTICS_CONTROL_CYCLE_MS),
                  FAN_HIGH,
                  100,
                  FAULT_SENSOR,
                  STATE_SAFE_MODE);
    assert_status(diagnostics_update(&context, &recovered, command, DIAGNOSTICS_CONTROL_CYCLE_MS),
                  FAN_MEDIUM,
                  70,
                  FAULT_NONE,
                  STATE_NORMAL);
}

void test_diagnostics_new_active_fault_replaces_latched_fault(void)
{
    DiagnosticsContext context;
    diagnostics_init(&context);

    const SensorInput sensor_fault = make_sensor_input(80.0f, 0.5f, 1200, false);
    const SensorInput over_current = make_sensor_input(80.0f, 2.1f, 1200, true);
    const FanCommand command = make_fan_command(FAN_MEDIUM, 70);

    (void)diagnostics_update(&context, &sensor_fault, command, DIAGNOSTICS_CONTROL_CYCLE_MS);
    const SystemStatus status = diagnostics_update(&context, &over_current, command, DIAGNOSTICS_CONTROL_CYCLE_MS);

    assert_status(status, FAN_OFF, 0, FAULT_OVER_CURRENT, STATE_FAULT_MODE);
}
