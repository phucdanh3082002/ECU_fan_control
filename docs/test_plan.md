# Test Plan

This document describes the UART integration test suite for both simulation and real hardware modes.

## Dual-Mode Architecture

The firmware supports two compilation modes via `ECU_SENSOR_HAL` in `main/CMakeLists.txt`:

| Mode | Compilation Flag | Test File | Commands Work |
|---|---|---|---|
| Simulation | `ECU_SENSOR_HAL=SIM` | `test_cases_simulation.json` | SET_TEMP, SET_CURRENT, SET_RPM, SET_SENSOR_VALID, USE_ADC_INPUT |
| Real Hardware | `ECU_SENSOR_HAL=REAL` | `test_cases_real.json` | SET_TEMP (override), SET_CALIB_OFFSET, INA219_DIAG |

**Important**: Simulation tests (`TC_*`) can only run on SIM mode firmware. Real hardware tests (`TC_REAL_*`) run on REAL mode firmware. Commands like SET_CURRENT, SET_RPM, SET_SENSOR_VALID are **ignored** in REAL mode.

## Execution

### Real Hardware Mode (current firmware)

```bash
python3 pi_test_bench/test_runner.py --port /dev/ttyUSB0 --mode real
# Windows:
python pi_test_bench/test_runner.py --port COM8 --mode real
```

### Simulation Mode (requires SIM firmware)

```bash
# First flash SIM firmware: set ECU_SENSOR_HAL="SIM" in main/CMakeLists.txt
python3 pi_test_bench/test_runner.py --port /dev/ttyUSB0 --mode sim
```

### Custom test file

```bash
python3 pi_test_bench/test_runner.py --port COM8 --cases path/to/custom.json
```

## Setup Delay

Both test files include `setup_delay_ms: 1500` in the config. This delay is applied after setup commands to allow the fan startup boost (1200ms at 100% duty) to complete before test steps run.

---

# Simulation Test Suite (ECU_SENSOR_HAL=SIM)

## Scope

Validates firmware behavior with simulated sensor inputs via UART commands.

## Test Groups

| Group | IDs | Coverage |
|---|---|---|
| Normal operation | `TC_001` - `TC_004` | Fan OFF, LOW, MEDIUM, HIGH based on actual firmware thresholds |
| Boundary behavior | `TC_005` - `TC_012` | 20°C, 47°C, 70°C, 90°C, 100°C boundaries |
| Invalid input | `TC_013` - `TC_015` | Unknown command, malformed float, invalid boolean |
| Fault behavior | `TC_016` - `TC_018` | Sensor fault, over-temperature, over-current |
| Recovery behavior | `TC_019` - `TC_020` | Sensor and over-current recovery |
| RTOS timing | `TC_RTOS_001` - `TC_RTOS_004` | Fast response, convergence, fan-stall, recovery timing |

## Actual Firmware Thresholds (fan_control.c)

| Temperature | Fan Mode | Duty Cycle |
|---|---|---|
| < 20°C | FAN_OFF | 0% |
| 20°C | FAN_LOW | 35% |
| 30°C | FAN_MEDIUM | 48% |
| 47°C | FAN_HIGH | 70% |
| ≥ 70°C | FAN_HIGH | 100% |
| ≥ 100°C | OVER_TEMPERATURE fault | 100% (forced) |

Startup boost: First 1200ms after fan starts → 100% duty regardless of calculated value.

---

# Real Hardware Test Suite (ECU_SENSOR_HAL=REAL)

## Scope

Validates firmware with actual LM35 (ADC) and INA219 (I2C) sensors. Uses SET_TEMP override to test control logic with known values on real hardware.

## Available Commands in REAL Mode

| Command | Behavior |
|---|---|
| `SET_TEMP:value` | Override temperature (positive=set, ≤0=clear back to real LM35) |
| `SET_CALIB_OFFSET:value` | Adjust LM35 calibration offset |
| `INA219_DIAG` | Dump INA219 I2C registers |
| `GET_STATUS` | Return current system status |
| `CLEAR_FAULT` | Clear latched fault |
| `SET_CURRENT`, `SET_RPM`, `SET_SENSOR_VALID`, `USE_ADC_INPUT` | **Ignored** |

## Test Groups

| Group | IDs | Coverage |
|---|---|---|
| Boot validation | `TC_REAL_001` | LM35 + INA219 init, system NORMAL |
| Temperature override | `TC_REAL_002`, `TC_REAL_003` | Control logic + over-temperature fault |
| Override lifecycle | `TC_REAL_004` | Clear override returns to real sensor |
| Calibration | `TC_REAL_005` | SET_CALIB_OFFSET acceptance |
| I2C diagnostics | `TC_REAL_006` | INA219 register dump |
| Fault recovery | `TC_REAL_007` | Over-temperature → clear → recovery |

## Prerequisites

- ESP32 firmware compiled in REAL mode: `ECU_SENSOR_HAL=REAL`
- LM35 temperature sensor connected to GPIO34
- INA219 current sensor connected to GPIO21 (SDA) / GPIO22 (SCL)
- DC fan with MOSFET driver connected to GPIO26
- Room temperature below 20°C (fan OFF without override)

## Known Behavior

- **SENSOR_FAULT may appear intermittently**: Caused by LM35 ADC read errors or INA219 I2C communication failures. Tests that don't specifically validate fault behavior omit FAULT/STATE expectations to avoid false failures.
- **RPM always -1**: 2-wire fan has no tachometer. Fan stall detection (TC_RTOS_003) is simulation-only.
- **INA219 current near 0A**: Low-current fan may read as noise. Over-current tests (SET_CURRENT) are simulation-only.

## Latest Result

Real hardware mode verified on `COM8`:

```text
Summary: 11/11 steps passed
Report: pi_test_bench/reports/test_report_real_20260508_173438.csv
```
