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
| `esp32_firmware/main/hal/sensor_input_real.h` | Real hardware HAL API for LM35/INA219 builds |
| `esp32_firmware/main/hal/sensor_input_real.c` | Real hardware HAL implementation; RPM unavailable for the current 2-wire fan |

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

When `ECU_SENSOR_HAL` is `REAL`, the build includes:

```text
hal/sensor_input_real.c
```

Unsupported HAL selections fail at CMake configure time.

## Verification

Completed:

```text
idf.py build -> Project build complete.
idf.py -B build_real -DECU_SENSOR_HAL=REAL build -> Project build complete.
idf.py -p COM8 flash -> Done
python unit_tests/run_tests.py -> 30 Tests 0 Failures
Coverage -> TOTAL: 213/220 lines covered (96.8%)
Cppcheck -> EXIT=0
Direct UART smoke/regression on COM8 -> PASS
```

Hardware verification commands:

```text
idf.py -p COM8 flash
direct UART smoke/regression over COM8
```

Phase 10 hardware verification covered UART temperature selection, ADC-mode command, simulated current, simulated RPM, sensor-valid fault path, over-temperature, over-current, fan-stall timeout, and invalid-command handling.
