# Host Unit Tests

Phase 9 adds host-based unit tests for pure firmware logic. These tests run on the development machine and do not require flashing the ESP32.

## Scope

Tested core modules:

- `fan_control.c`
- `diagnostics.c`
- `system_types.c`
- `uart_protocol.c`
- `hal/sensor_input_sim.c`

`app_tasks.c` is intentionally excluded because it depends on ESP-IDF drivers, UART, GPIO, ADC, PWM, and FreeRTOS scheduling.

## Run

From the repository root:

```bash
python unit_tests/run_tests.py
```

The script compiles the tests with host `gcc`, runs the Unity-style test binary, executes `gcov`, and writes:

```text
coverage/coverage_summary.txt
```

## Coverage

The target for Phase 9 is greater than `80%` line coverage for core modules. LCOV/genhtml can consume the generated `gcda` and `gcno` files on hosts where LCOV is installed.
