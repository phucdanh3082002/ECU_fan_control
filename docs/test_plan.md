# Simulation Test Plan

This document describes the Phase 7 simulation-mode UART regression suite.

## Scope

The test bench validates the ESP32 firmware through the public UART protocol. This covers command parsing, simulated sensor inputs, fan-control decisions, diagnostics overrides, fault recovery, status formatting, and selected RTOS timing behavior.

The suite does not use private firmware hooks. Test cases interact with the same UART interface that a Raspberry Pi test bench uses.

## Execution

Windows:

```bash
python pi_test_bench/test_runner.py --port COM8
```

Raspberry Pi:

```bash
python3 pi_test_bench/test_runner.py --port /dev/ttyUSB0
```

Use `/dev/ttyACM0` if the ESP32 enumerates as a CDC ACM serial device.

## Test Groups

| Group | IDs | Coverage |
|---|---|---|
| Normal operation | `TC_001` - `TC_004` | Fan OFF, LOW, MEDIUM, HIGH with no fault active |
| Boundary behavior | `TC_005` - `TC_012` | `40C`, `70C`, `90C`, `100C`, and `2.0A` boundaries |
| Invalid input | `TC_013` - `TC_015` | Unknown command, malformed float, invalid boolean |
| Fault behavior | `TC_016` - `TC_018` | Sensor fault, over-temperature, over-current |
| Recovery behavior | `TC_019` - `TC_020` | Sensor and over-current recovery |
| RTOS timing | `TC_RTOS_001` - `TC_RTOS_004` | Fast status response, command convergence, fan-stall timeout, recovery timing |

## Expected Behavior

| Condition | Expected Output |
|---|---|
| `TEMP < 40C` | `FAN=OFF`, `DUTY=0` |
| `40C <= TEMP < 70C` | `FAN=LOW`, `DUTY=40` |
| `70C <= TEMP < 90C` | `FAN=MEDIUM`, `DUTY=70` |
| `90C <= TEMP < 100C` | `FAN=HIGH`, `DUTY=100`, `FAULT=NONE` |
| `TEMP >= 100C` | `FAULT=OVER_TEMPERATURE`, `STATE=FAULT_MODE`, fan forced high |
| `CURRENT > 2.0A` | `FAULT=OVER_CURRENT`, `STATE=FAULT_MODE`, fan off |
| `SENSOR_VALID=0` | `FAULT=SENSOR_FAULT`, `STATE=SAFE_MODE`, fan forced high |
| Fan commanded on and `RPM=0` for timeout | `FAULT=FAN_STALL`, `STATE=FAULT_MODE`, fan off |
| Fault condition cleared for required diagnostics cycles | `FAULT=NONE`, `STATE=NORMAL` |

## Latest Result

Verified against ESP32 on `COM8`:

```text
Summary: 34/34 steps passed
Report: C:\Users\danhs\Downloads\ECU_fan_control\pi_test_bench\reports\test_report_20260504_203500.csv
```
