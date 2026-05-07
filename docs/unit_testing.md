# Host Unit Testing And Coverage

This document records the Phase 9 host-based unit test setup and result.

## Purpose

Phase 9 verifies firmware core logic without flashing ESP32 hardware. This catches logic regressions quickly and prepares coverage measurement for safety-critical control behavior.

## Scope

Tested modules:

| Module | Coverage Purpose |
|---|---|
| `fan_control.c` | Fan thresholds at `40C`, `70C`, `90C`, and `100C` |
| `diagnostics.c` | Sensor fault, over-temperature, over-current, fan stall, and recovery cycles |
| `system_types.c` | String conversion used by UART status output |
| `uart_protocol.c` | Command parser and response formatter |
| `hal/sensor_input_sim.c` | Simulation HAL sensor source selection and validity composition |

Not tested in Phase 9:

- `app_tasks.c`, because it is hardware/RTOS integration code and belongs to integration testing.
- Full ESP32 firmware coverage, because GCOV here is host-based core logic coverage only.

## Test Harness

The tests use a small Unity-style harness in:

```text
unit_tests/unity/
```

It provides the Unity macros needed by this project, including `UNITY_BEGIN`, `RUN_TEST`, `TEST_ASSERT_EQUAL_INT`, `TEST_ASSERT_EQUAL_STRING`, and float assertions.

This keeps the host test dependency small and does not add anything to the ESP32 firmware binary.

## Run

From the repository root:

```bash
python unit_tests/run_tests.py
```

The script performs these steps:

1. Cleans `unit_tests/build/` and previous coverage artifacts.
2. Compiles core modules and unit tests with host `gcc`.
3. Runs the host test executable.
4. Runs `gcov` for core modules.
5. Writes `coverage/coverage_summary.txt`.

## Latest Result

```text
30 Tests 0 Failures
```

Coverage result:

```text
fan_control.c: 8/8 lines covered (100.0%)
diagnostics.c: 41/42 lines covered (97.6%)
system_types.c: 21/21 lines covered (100.0%)
uart_protocol.c: 101/107 lines covered (94.4%)
sensor_input_sim.c: 42/42 lines covered (100.0%)

TOTAL: 213/220 lines covered (96.8%)
Coverage target: >80%
Result: PASS
```

## LCOV Note

`gcov` is available on this Windows machine. `lcov` and `genhtml` are not currently installed.

The Phase 9 script still generates coverage data (`gcda`, `gcno`, and `gcov` output). On a Linux/Raspberry Pi host with LCOV installed, the same build artifacts can be converted into LCOV `.info` and HTML reports.
