# ECU-Like Fan Control & Diagnostics Test Bench

**Quick Reference for OpenCode Agents** | For detailed specifications, wiring diagrams, and functional requirements, see **README.md**

## Project Overview
Embedded systems project combining ESP32 firmware (FreeRTOS-based) with Python-based automated testing on Raspberry Pi 4. The system demonstrates ECU-like fan control, diagnostics, safe mode, and fault recovery mechanisms.

- **Phase 1-2**: Simulation mode (Potentiometer, Button, LED) - Foundation & Learning
- **Phase 3+**: Real hardware (LM35, INA219, DC Fan with Tachometer) - Production-ready

## Project Structure
```
ecu-like-fan-control-diagnostics-test-bench/
├── esp32_firmware/          # ESP32 firmware (ESP-IDF + FreeRTOS)
│   ├── main/                # Application code
│   ├── CMakeLists.txt
│   └── sdkconfig
├── pi_test_bench/           # Python test automation
│   ├── serial_client.py
│   ├── test_runner.py
│   ├── test_cases.json
│   └── reports/
├── unit_tests/              # Host-based unit tests (Unity Framework)
├── static_analysis/         # Cppcheck results
├── docs/                    # Documentation
└── coverage/                # Code coverage reports
```

## Build & Test Commands

### ESP32 Firmware
- **Build**: `idf.py build` (from esp32_firmware directory)
- **Flash**: `idf.py flash` (requires ESP32 connected via USB)
- **Monitor**: `idf.py monitor`
- **Clean**: `idf.py fullclean`

### Python Test Bench
- **Run tests**: `python3 pi_test_bench/test_runner.py` (on Raspberry Pi 4)
- **Check serial port**: `ls /dev/ttyUSB* /dev/ttyACM*` (ESP32 will appear here)
- **Baud rate**: 115200

### Static Analysis
- **Cppcheck**: `cppcheck --enable=all --inconclusive --std=c++11 esp32_firmware/src`
- **Report output**: `static_analysis/cppcheck_report.txt`

### Unit Tests
- **Build & run**: Host-based tests using Unity Framework (prepares for GCOV/LCOV coverage)

## Architecture & Key Points

### Hardware Mapping (See README §4-6 for wiring diagrams)

**Phase 1-2 Simulation**: GPIO34 (potentiometer), GPIO4 (button), GPIO26 (LED), USB (UART@115200)
**Phase 3+ Real Hardware**: GPIO34 (LM35), GPIO21/22 (INA219 I2C), GPIO26 (MOSFET), GPIO27 (tachometer), USB (UART@115200)

**GPIO Safety Notes for ESP32-WROOM-32**:
- **Avoid GPIO6-11**: Flash memory pins (will cause system crash)
- **Avoid GPIO12-15 during JTAG debugging**: JTAG pins, can cause issues
- **GPIO21/22**: Pre-labeled as WIRE_SDA/WIRE_SCL, perfect for I2C
- **GPIO4**: Has internal pull-up, ideal for button/input
- **GPIO34**: ADC input only (no output capability)
- **GPIO26**: Can be used as regular GPIO despite DAC_2 label

### FreeRTOS Task Structure

#### Phase 1-2: Simulation Mode
- **UartCommandTask** (P5): Event-driven, handles incoming commands
- **DiagnosticsTask** (P4): 100ms cycle, fault detection
- **FanControlTask** (P4): 100ms cycle, fan mode calculation
- **AnalogInputTask** (P3): 100ms cycle, ADC reading (GPIO34: potentiometer)
- **ButtonInputTask** (P3): 50ms cycle, button state (GPIO4: TOUCH0)
- **PwmOutputTask** (P2): 100ms cycle, LED PWM output (GPIO26: DAC_2)
- **StatusReportTask** (P2): Event/500ms, status reporting

#### Phase 3+: Real Hardware Mode
- **UartCommandTask** (P5): Event-driven, handles incoming commands
- **SensorReadTask** (P4): 100ms cycle, LM35 (ADC) + INA219 (I2C) reading
- **DiagnosticsTask** (P4): 100ms cycle, fault detection (with I2C error handling)
- **FanControlTask** (P4): 100ms cycle, fan mode calculation
- **TachometerTask** (P3): 100ms cycle, tachometer RPM counting (GPIO27: TOUCH7)
- **FanDriverTask** (P2): 100ms cycle, MOSFET PWM output (GPIO26: DAC_2)
- **StatusReportTask** (P2): Event/500ms, status reporting

### Shared Data Protection & Fan Logic
- Use `systemMutex` (FreeRTOS semaphore) to protect `SensorInput` and `SystemStatus` structures
- **Fan Control**: OFF (<40°C) → LOW (40-69°C, 40%) → MEDIUM (70-89°C, 70%) → HIGH (≥90°C, 100%)
- See README §7 for complete functional requirements

## UART Protocol

### Command Format (Raspberry Pi → ESP32)
```
SET_TEMP:85          # Set temperature (°C)
SET_CURRENT:1.2      # Set current (A)
SET_RPM:1200         # Set fan RPM
SET_SENSOR_VALID:1   # Sensor valid (1) or invalid (0)
USE_ADC_INPUT:1      # Use ADC (1) or UART commands (0)
CLEAR_FAULT          # Clear fault if recovery conditions met
GET_STATUS           # Request current status
```

### Response Format (ESP32 → Raspberry Pi)
```
STATUS,TEMP=85,CURRENT=1.2,RPM=1200,FAN=MEDIUM,DUTY=70,FAULT=NONE,STATE=NORMAL
```

## Development Notes

### Setup & Hardware (See README §9 for detailed setup; §4-6 for hardware specs)

**Phase 1-2 Essentials**:
- ESP32 firmware: ESP-IDF 5.x + FreeRTOS timing critical (DiagnosticsTask, FanControlTask every 100ms)
- Mutex protection required for SensorInput and SystemStatus access
- UART over USB at /dev/ttyUSB0 or /dev/ttyACM0 (baud: 115200)

**Phase 3+ Hardware**:
- LM35: GPIO34 (ADC), ~10mV/°C, calibration required
- INA219: GPIO21/22 (I2C 0x40), current & bus voltage measurement
- DC Fan: GPIO26 (PWM via MOSFET), GPIO27 (tachometer optional)

### Hardware Abstraction Layer (HAL)
Support both simulation and real hardware via abstraction:
```c
typedef struct {
    float (*read_temperature)(void);
    float (*read_current)(void);
    int (*read_rpm)(void);
} SensorHAL;
```
Simulation returns g_sensorInput values; real hardware reads LM35 (ADC) and INA219 (I2C).

### Common Patterns
- Use `xSemaphoreTake(systemMutex, portMAX_DELAY)` before reading/writing shared state
- Use `vTaskDelayUntil()` for precise periodic task timing
- Command parsing on UartCommandTask should be event-driven (UART interrupt + queue)
- Status reports should be triggered on fault state changes or periodic (500ms)
- In Phase 3+, sensor I2C errors should trigger FAULT_SENSOR (same as invalid values)

## Testing Strategy
1. **Unit Tests**: Core fan_control and diagnostics modules (host-based, Unity Framework)
2. **Integration Tests**: Full firmware behavior via UART commands (Python test bench)
3. **Test Coverage**: Boundary, invalid input, fault, recovery, and RTOS timing scenarios
4. **Static Analysis**: Cppcheck on esp32_firmware/src
5. **Code Coverage**: GCOV/LCOV for host-based unit tests only (not full ESP32 firmware)

## File Organization Rules
- ESP32 code: C/C++ with header/source separation
- Python: One responsibility per module (serial_client, test_runner, report_generator)
- Test cases: JSON format in test_cases.json
- Reports: CSV format in pi_test_bench/reports/
