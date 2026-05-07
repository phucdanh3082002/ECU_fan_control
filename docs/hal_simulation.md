# HAL Simulation

This document records Phase 10: Hardware Abstraction Layer for simulation mode.

## Purpose

The firmware now has a sensor input HAL so fan control and diagnostics do not need to know whether values came from UART commands, the potentiometer ADC, the button, or future real hardware.

This prepares the project for Phase 12 real hardware integration without duplicating fan-control or diagnostics logic.

## Files

| File | Role |
|---|---|
| `esp32_firmware/main/hal/sensor_input.h` | Generic HAL interface used by application tasks |
| `esp32_firmware/main/hal/sensor_input_sim.h` | Simulation setter API for UART/ADC/button producers |
| `esp32_firmware/main/hal/sensor_input_sim.c` | Simulation HAL state and read implementation |

## Interface

The generic HAL exposes these read functions through `SensorInputHal`:

- `read_temperature()`
- `read_current()`
- `read_rpm()`
- `is_sensor_valid()`
- `is_adc_input_enabled()`

`sensor_input_hal_read()` returns a complete `SensorInput` snapshot.

## Simulation Sources

The simulation HAL owns the current simulated sensor state:

| Input Source | HAL State Updated |
|---|---|
| Potentiometer ADC task | ADC temperature |
| UART `SET_TEMP` | UART temperature and ADC mode disabled |
| UART `SET_CURRENT` | Simulated current |
| UART `SET_RPM` | Simulated RPM |
| UART `SET_SENSOR_VALID` | UART sensor-valid flag |
| Button task | Button-pressed flag |
| UART `USE_ADC_INPUT` | ADC/UART temperature source selection |

The simulation validity rule remains:

```text
sensorValid = uartSensorValid && !buttonPressed
```

This preserves Phase 1-2 behavior: pressing the button still triggers sensor fault/safe mode, and the potentiometer still controls temperature when ADC mode is enabled.

## CMake Selection

`esp32_firmware/main/CMakeLists.txt` defaults to:

```cmake
ECU_SENSOR_HAL=SIM
```

When `ECU_SENSOR_HAL` is `SIM`, the build includes:

```text
hal/sensor_input_sim.c
```

Unsupported HAL selections fail at CMake configure time. This makes future real-hardware HAL integration explicit.

## Verification

Completed:

```text
idf.py build -> Project build complete.
python unit_tests/run_tests.py -> 29 Tests 0 Failures
Coverage -> TOTAL: 212/219 lines covered (96.8%)
Cppcheck -> EXIT=0
```

Pending hardware verification:

```text
idf.py -p COM8 flash
python pi_test_bench/test_runner.py --port COM8
```

Reason pending: `COM8` was not enumerated by Windows during Phase 10 verification.
