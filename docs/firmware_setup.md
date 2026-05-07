# Firmware Setup

This document captures the current firmware baseline for the ESP32 side of the ECU-like fan control project.

## Project Location

```text
esp32_firmware/
```

## Current Baseline

- Language: C
- Framework: ESP-IDF v6.0 installed on the local machine
- Target: ESP32 DevKit V1
- Firmware behavior: Phase 5 UART command protocol running on the Phase 4 FreeRTOS task architecture, verified by the Phase 7 Python UART regression suite

## Phase 2 Hardware POC

GPIO mapping for ESP32-WROOM-32:

| Function | GPIO | ESP-IDF Peripheral |
|---|---:|---|
| Potentiometer signal | GPIO34 | ADC1 Channel 6 |
| Button | GPIO4 | GPIO input with internal pull-up |
| LED PWM | GPIO26 | LEDC PWM output |

Runtime behavior:

- The potentiometer raw ADC value is sampled every 500 ms.
- ADC raw value is converted to a `0-100%` potentiometer percentage.
- ADC raw value is also mapped to a simulated `0-120C` temperature range for early validation.
- LED duty follows the potentiometer percentage when the button is released.
- Pressing the button forces LED duty to `100%` and logs `BUTTON=PRESSED`.

## Phase 3 Core Logic POC

The firmware now includes hardware-independent core modules:

| Module | Responsibility |
|---|---|
| `system_types` | Shared `SensorInput`, `SystemStatus`, enums and string conversion helpers |
| `fan_control` | Temperature-to-fan-mode and duty-cycle thresholds |
| `diagnostics` | Fault detection, safe/fault mode behavior and 3-cycle recovery |

Current runtime behavior:

- Control cycle runs every `100 ms` with `vTaskDelayUntil()`.
- Status is logged every `500 ms`.
- Potentiometer maps `GPIO34` ADC raw value to simulated `0-100C` temperature.
- Button press maps to `sensorValid=false`, triggering `SENSOR_FAULT` and `SAFE_MODE`.
- Simulated current is fixed at `0.5A` until UART command support is added in Phase 5.
- Simulated RPM is fixed at `1200` until UART command support is added in Phase 5.

Fan thresholds implemented:

| Temperature | Fan | Duty |
|---|---|---:|
| `< 40C` | `OFF` | `0%` |
| `40C - 69C` | `LOW` | `40%` |
| `70C - 89C` | `MEDIUM` | `70%` |
| `>= 90C` | `HIGH` | `100%` |

Diagnostics implemented:

| Condition | Fault | State | Fan/Duty |
|---|---|---|---|
| `sensorValid=false` or temp out of `-40C..150C` | `SENSOR_FAULT` | `SAFE_MODE` | `HIGH / 100%` |
| `temperature >= 100C` | `OVER_TEMPERATURE` | `FAULT_MODE` | `HIGH / 100%` |
| `current > 2.0A` | `OVER_CURRENT` | `FAULT_MODE` | `OFF / 0%` |
| `duty > 0` and `rpm = 0` for `1000 ms` | `FAN_STALL` | `FAULT_MODE` | `OFF / 0%` |

Fault recovery requires the fault condition to be cleared for `3` consecutive `100 ms` control cycles.

## Phase 4 FreeRTOS Task Architecture

The single-loop Phase 3 firmware has been split into FreeRTOS tasks. Each task has one primary responsibility, and shared data is protected by `systemMutex`.

| Task | Period | Priority | Responsibility |
|---|---:|---:|---|
| `UartCommandTask` | Event/poll driven | 5 | Reads USB UART commands and returns protocol responses |
| `DiagnosticsTask` | 100 ms | 4 | Evaluates fault conditions and updates `SystemStatus` |
| `FanControlTask` | 100 ms | 4 | Converts temperature to normal fan command |
| `AnalogInputTask` | 100 ms | 3 | Reads `GPIO34` ADC and updates simulated temperature |
| `ButtonInputTask` | 50 ms | 3 | Reads `GPIO4`; pressed means `sensorValid=false` |
| `PwmOutputTask` | 100 ms | 2 | Writes `SystemStatus.dutyCycle` to `GPIO26` PWM |
| `StatusReportTask` | 500 ms | 2 | Prints current status over USB serial monitor |

Shared state:

| State | Producer | Consumer |
|---|---|---|
| `SensorInput` | `AnalogInputTask`, `ButtonInputTask` | `FanControlTask`, `DiagnosticsTask`, `StatusReportTask` |
| normal fan command | `FanControlTask` | `DiagnosticsTask` |
| `SystemStatus` | `DiagnosticsTask` | `PwmOutputTask`, `StatusReportTask` |
| debug ADC/button fields | `AnalogInputTask`, `ButtonInputTask` | `StatusReportTask` |

Why this structure matters:

- Hardware input is isolated from control logic.
- Diagnostics can override normal fan output when a fault is active.
- PWM output only consumes the final system status, so it does not need to know why a duty cycle was chosen.
- The mutex prevents partial reads/writes when multiple tasks access shared state.
- `vTaskDelayUntil()` keeps periodic tasks aligned to their intended cycle time instead of drifting over time.

## Phase 5 UART Protocol

UART protocol is handled by `UartCommandTask` on `UART0` through the ESP32 USB serial bridge.

Serial settings:

| Setting | Value |
|---|---|
| Port | `COM8` on this Windows machine |
| Baud rate | `115200` |
| Data bits | `8` |
| Parity | None |
| Stop bits | `1` |
| Line ending | `\r\n` or `\n` |

Supported commands:

| Command | Behavior |
|---|---|
| `SET_TEMP:85` | Sets simulated temperature to `85C` and switches to UART temperature mode |
| `SET_CURRENT:1.2` | Sets simulated current in amps |
| `SET_RPM:1200` | Sets simulated RPM |
| `SET_SENSOR_VALID:1` | Marks UART sensor override as valid |
| `SET_SENSOR_VALID:0` | Triggers sensor fault via UART override |
| `USE_ADC_INPUT:1` | Uses potentiometer/ADC temperature input |
| `USE_ADC_INPUT:0` | Uses last UART temperature value |
| `CLEAR_FAULT` | Requests diagnostics context reset; active faults will immediately re-latch |
| `GET_STATUS` | Returns current status without changing state |

Protocol response format:

```text
STATUS,TEMP=80.0,CURRENT=0.5,RPM=1200,FAN=MEDIUM,DUTY=70,FAULT=NONE,STATE=NORMAL
```

Invalid command response:

```text
ERROR,REASON=UNKNOWN_OR_INVALID_COMMAND
```

Implementation notes:

- `SET_TEMP` intentionally switches `useAdcInput=false` so UART tests can control temperature directly.
- `USE_ADC_INPUT:1` returns control to the potentiometer input.
- Command responses are delayed by `350 ms` after state-changing commands so `AnalogInputTask`, `FanControlTask`, and `DiagnosticsTask` can converge before the status line is emitted.
- `UartCommandTask` accepts real `CR/LF`, literal text `\n` or `\r`, and commands without line ending after a short idle timeout. This makes the protocol tolerant of serial tools such as Hercules.
- `StatusReportTask` still emits periodic ESP-IDF log lines for human debugging; automated tools should parse lines that start exactly with `STATUS,` or `ERROR,`.

Hercules-specific notes:

- If Hercules shows text like `SET_CURRENT:2.5I (...) app_tasks: STATUS...`, it means the command was displayed in the receive window without a real line ending.
- Firmware now parses this case after `300 ms` of UART receive idle time.
- If you type `SET_CURRENT:2.5\n`, Hercules sends two literal characters (`\` and `n`), not a newline byte; firmware also accepts that case.
- Periodic ESP-IDF logs still appear every `500 ms`, so protocol responses may be mixed with human-readable logs. Look for lines beginning exactly with `STATUS,` or `ERROR,`.

## Phase 6 Python Test Bench

The Python test bench automates UART integration testing against the running ESP32 firmware. It sends protocol commands, parses `STATUS,` and `ERROR,` response lines, compares expected fields, and writes a CSV report.

Files:

| File | Responsibility |
|---|---|
| `pi_test_bench/serial_client.py` | Opens the serial port, sends commands, and filters protocol responses |
| `pi_test_bench/test_cases.json` | Defines setup commands, cleanup commands, and Phase 6 test cases |
| `pi_test_bench/test_runner.py` | CLI runner that executes JSON test steps and prints pass/fail results |
| `pi_test_bench/report_generator.py` | Writes CSV reports under `pi_test_bench/reports/` |

Install dependencies:

```bash
python -m pip install -r pi_test_bench/requirements.txt
```

Run on this Windows machine:

```bash
python pi_test_bench/test_runner.py --port COM8
```

Run on Raspberry Pi:

```bash
python3 pi_test_bench/test_runner.py --port /dev/ttyUSB0
```

If the ESP32 appears as a CDC ACM device instead, use:

```bash
python3 pi_test_bench/test_runner.py --port /dev/ttyACM0
```

Runner behavior:

- Setup commands reset simulated inputs before each test case.
- Cleanup commands restore current, RPM, sensor validity, and `USE_ADC_INPUT:1` after all tests.
- Expected fields are subset matches, so logs can include additional fields without breaking tests.
- `wait_ms` steps support time-dependent checks such as fan-stall detection.
- Generated CSV reports are local test artifacts and are ignored by git except for `pi_test_bench/reports/.gitkeep`.

Important serial-port rule:

- Only one program can own `COM8` at a time. Close `idf.py monitor`, Hercules, or any other serial terminal before running the Python test bench.

## Phase 7 Test Case Completion

Phase 7 expands the Phase 6 smoke tests into a full simulation-mode regression suite. The suite is still driven over UART, so it validates the same system path a Raspberry Pi test bench uses: command parser, shared state update, fan-control task, diagnostics task, and status formatting.

Test coverage groups:

| Group | Test IDs | Purpose |
|---|---|---|
| Normal operation | `TC_001` - `TC_004` | Fan OFF/LOW/MEDIUM/HIGH behavior in non-fault conditions |
| Boundary behavior | `TC_005` - `TC_012` | Exact fan thresholds and `2.0A` over-current boundary |
| Invalid input | `TC_013` - `TC_015` | Unknown command, malformed numeric value, invalid boolean value |
| Fault behavior | `TC_016` - `TC_018` | Sensor fault, over-temperature, over-current |
| Recovery behavior | `TC_019` - `TC_020` | Recovery after sensor and over-current faults clear |
| RTOS timing | `TC_RTOS_001` - `TC_RTOS_004` | Response timing, command convergence, fan-stall timeout, recovery-cycle timing |

Runner updates for Phase 7:

- CSV reports now include `elapsed_ms` for each step.
- Test steps can define `max_elapsed_ms` and `min_elapsed_ms` for timing assertions.
- The serial parser trims protocol responses at the final protocol field, preventing ESP-IDF logs from contaminating CSV output.
- No firmware changes were required for Phase 7; the extra logic is host-side test automation only.

## Phase 8 Static Analysis

Phase 8 applies Cppcheck to the ESP32 application firmware. Cppcheck is installed on this Windows machine at:

```text
C:\Program Files\Cppcheck\cppcheck.exe
```

The analysis uses the ESP-IDF generated `compile_commands.json` so Cppcheck sees the same include paths and defines as the firmware build. The project source directory is `esp32_firmware/main`, so the analysis filters to `*/main/*.c` instead of the roadmap placeholder `esp32_firmware/src`.

Run from the repository root:

```powershell
& "C:\Program Files\Cppcheck\cppcheck.exe" --enable=all --inconclusive --check-level=exhaustive --force --inline-suppr --error-exitcode=1 --suppress=missingInclude --suppress=missingIncludeSystem --suppress=checkersReport --suppress="*:C:\esp\v6.0\esp-idf\*" --suppress="unusedFunction:esp32_firmware\main\main.c" --template=gcc --quiet --project="esp32_firmware/build/compile_commands.json" --file-filter="*/main/*.c" --output-file="static_analysis/cppcheck_report.txt"
```

Suppressions used:

- `missingInclude` and `missingIncludeSystem`: Cppcheck environment noise for ESP-IDF and standard headers, not application defects.
- `*:C:\esp\v6.0\esp-idf\*`: excludes findings inside ESP-IDF framework headers from the project report.
- `unusedFunction:esp32_firmware\main\main.c`: `app_main` is called by ESP-IDF startup code, so standalone static analysis sees it as unused.
- `checkersReport`: Cppcheck metadata, not a code issue.

Current result:

- Cppcheck exit code: `0` with `--error-exitcode=1`.
- `static_analysis/cppcheck_report.txt` generated.
- No unsuppressed warnings in application firmware.
- No critical findings such as null-pointer use, memory leaks, invalid frees, or uninitialized application data.

## Phase 9 Host Unit Tests And Coverage

Phase 9 adds host-based unit tests for pure firmware logic. These tests run on the development machine with host `gcc`, so core logic can be validated without flashing the ESP32.

Tested modules:

| Module | Reason |
|---|---|
| `fan_control.c` | Pure temperature-to-fan command logic |
| `diagnostics.c` | Fault detection, latch behavior, and recovery-cycle counting |
| `system_types.c` | Enum-to-protocol string conversion |
| `uart_protocol.c` | UART command parsing and response formatting |
| `hal/sensor_input_sim.c` | Simulation HAL source selection and sensor-valid composition |

Excluded module:

- `app_tasks.c` is intentionally excluded from host unit tests because it depends on ESP-IDF drivers, FreeRTOS tasks, UART, GPIO, ADC, and PWM hardware APIs.

Run from the repository root:

```bash
python unit_tests/run_tests.py
```

The script:

- Compiles core firmware modules and test files with host `gcc`.
- Runs a small Unity-style test harness under `unit_tests/unity/`.
- Runs `gcov` for core modules.
- Writes `coverage/coverage_summary.txt`.

Current result:

```text
30 Tests 0 Failures
TOTAL: 213/220 lines covered (96.8%)
Coverage target: >80%
Result: PASS
```

LCOV/genhtml are not installed on this Windows host. The build still generates `gcda`, `gcno`, and `gcov` data, so LCOV can be run later on a host where those tools are available.

## Phase 10 HAL Simulation

Phase 10 introduces a sensor input HAL for simulation mode. The firmware now reads sensor values through a small function-pointer interface instead of letting control and diagnostics depend directly on where the sensor values came from.

Files:

| File | Responsibility |
|---|---|
| `esp32_firmware/main/hal/sensor_input.h` | Generic sensor-input HAL interface |
| `esp32_firmware/main/hal/sensor_input_sim.h` | Simulation-specific setter API used by UART, ADC, and button tasks |
| `esp32_firmware/main/hal/sensor_input_sim.c` | Simulation HAL state and read functions |

Simulation behavior:

- `SET_TEMP:<value>` stores a UART temperature and switches `useAdcInput=false`.
- `USE_ADC_INPUT:1` switches temperature back to the potentiometer-fed ADC value.
- `SET_CURRENT:<value>` updates simulated current in the HAL.
- `SET_RPM:<value>` updates simulated RPM in the HAL.
- `SET_SENSOR_VALID:<0|1>` updates UART-controlled validity.
- Button press still contributes to sensor validity, so `sensorValid = uartSensorValid && !buttonPressed`.

CMake selection:

- `ECU_SENSOR_HAL` defaults to `SIM`.
- `hal/sensor_input_sim.c` is compiled when `ECU_SENSOR_HAL=SIM`.
- `hal/sensor_input_real.c` is compiled when `ECU_SENSOR_HAL=REAL`.
- Unsupported HAL selections fail CMake early with a clear error.

Verification performed:

- Host unit tests: `30 Tests 0 Failures`.
- Host coverage: `96.8%` total line coverage including `sensor_input_sim.c` at `100.0%`.
- ESP-IDF build: `Project build complete`.
- ESP-IDF real-HAL build: `idf.py -B build_real -DECU_SENSOR_HAL=REAL build` completed.
- ESP32 flash on `COM8`: completed.
- Direct UART smoke/regression on `COM8`: passed.
- Cppcheck: `EXIT=0`, no unsuppressed application-firmware findings.

## Phase 12 Real Hardware HAL Skeleton

The real HAL is compile-selectable but does not replace the default simulation build. It reads LM35 on GPIO34, initializes INA219 on I2C0 GPIO21/GPIO22 at address `0x40`, and reports RPM as `SENSOR_RPM_UNAVAILABLE` because the current 2-wire fan has no tachometer output.

Build command:

```powershell
idf.py -B build_real -DECU_SENSOR_HAL=REAL build
```

The `build_real/` output directory is ignored by Git.

## Activate ESP-IDF On This Windows Machine

The standard `idf.py` command is not available in a plain PowerShell session until the ESP-IDF environment is activated.

For this machine, use the ESP-IDF/EIM activation script:

```powershell
& "C:\Espressif\tools\Microsoft.v6.0.PowerShell_profile.ps1"
```

After activation, verify the CLI:

```bash
idf.py --version
```

## Build

Run from `esp32_firmware/`:

```bash
idf.py set-target esp32
idf.py build
```

## Flash And Monitor

Run from `esp32_firmware/`:

```bash
idf.py flash monitor
```

If the serial port is not detected automatically, specify it explicitly:

```bash
idf.py -p <serial-port> flash monitor
```

Examples:

```bash
idf.py -p COM3 flash monitor
idf.py -p /dev/ttyUSB0 flash monitor
idf.py -p /dev/ttyACM0 flash monitor
```

## Expected Monitor Output

```text
ECU Fan Control Phase 3 core logic POC started
ADC: GPIO34, Button: GPIO4, LED PWM: GPIO26
STATUS,TEMP=18.8,CURRENT=0.5,RPM=1200,SENSOR=1,FAN=OFF,DUTY=0,FAULT=NONE,STATE=NORMAL,ADC_RAW=768,POT=19%,BUTTON=RELEASED
```

When validating hardware:

- Rotate the potentiometer and confirm `TEMP`, `FAN`, `DUTY` and LED brightness follow the fan-control thresholds.
- Press the button and confirm `SENSOR=0`, `FAULT=SENSOR_FAULT`, `STATE=SAFE_MODE`, `FAN=HIGH`, `DUTY=100`.
- Release the button and confirm recovery to `FAULT=NONE`, `STATE=NORMAL` after 3 stable cycles.

## Notes

- `sdkconfig` is generated by ESP-IDF during the first build and should be reviewed before committing if project-specific options are added.
- `sdkconfig.defaults` contains project defaults that should remain stable across machines.
- UART protocol is implemented for Phase 5 commands.
- Python test bench automation is implemented for Phase 6 in `pi_test_bench/`.
- Phase 7 simulation regression coverage is implemented in `pi_test_bench/test_cases.json` and documented in `docs/test_plan.md`.
- Phase 8 static analysis is documented in `docs/static_analysis.md`; local Cppcheck output is stored in `static_analysis/cppcheck_report.txt`.
- Phase 9 host unit tests are documented in `docs/unit_testing.md`; coverage summary is stored in `coverage/coverage_summary.txt`.
- Phase 10 HAL simulation is documented in `docs/hal_simulation.md`; firmware build, flash, unit tests, Cppcheck, and direct UART smoke/regression pass.

## Verified Baseline

The initial project skeleton has been verified with:

```bash
idf.py set-target esp32
idf.py build
```

Result:

```text
Project build complete.
```

Phase 3 has also been verified with:

```bash
idf.py build
idf.py -p COM8 flash
idf.py -p COM8 monitor
```

Observed result:

```text
ECU Fan Control Phase 3 core logic POC started
STATUS,TEMP=18.8,CURRENT=0.5,RPM=1200,SENSOR=1,FAN=OFF,DUTY=0,FAULT=NONE,STATE=NORMAL,ADC_RAW=768,POT=19%,BUTTON=RELEASED
```

Phase 7 regression suite has been verified against the ESP32 on `COM8` with:

```bash
python pi_test_bench/test_runner.py --port COM8
```

Observed result:

```text
Summary: 34/34 steps passed
Report: C:\Users\danhs\Downloads\ECU_fan_control\pi_test_bench\reports\test_report_20260504_203500.csv
```

Phase 8 Cppcheck analysis has been verified with:

```text
Cppcheck 2.20.0
EXIT=0
```

The focused application-firmware report contains no unsuppressed warnings.

Phase 9 host unit tests and GCOV coverage have been verified with:

```bash
python unit_tests/run_tests.py
```

Observed result:

```text
30 Tests 0 Failures
TOTAL: 213/220 lines covered (96.8%)
Result: PASS
```

Phase 10 HAL simulation firmware has been verified with:

```bash
idf.py build
idf.py -p COM8 flash
python unit_tests/run_tests.py
```

Observed result:

```text
Project build complete.
Flash done on COM8.
30 Tests 0 Failures
TOTAL: 213/220 lines covered (96.8%)
Cppcheck EXIT=0
Direct UART smoke/regression: PASS
```
