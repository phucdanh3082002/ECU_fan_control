# ECU-Like Fan Control & Diagnostics Test Bench

## Project Overview
Embedded systems project combining ESP32 firmware (FreeRTOS-based) with Python-based automated testing on Raspberry Pi 4. The system demonstrates ECU-like fan control, diagnostics, safe mode, and fault recovery mechanisms.

### Project Phases
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

### Hardware Mapping

#### Phase 1-2: Simulation Mode
| GPIO/Interface | Component | Role |
|--------|-----------|------|
| GPIO34 | Potentiometer | ADC input (temperature simulation) |
| GPIO25 | Button | Sensor fault input (internal pull-up) |
| GPIO26 | LED (220Ω resistor) | PWM output (fan speed simulation) |
| USB | Raspberry Pi | UART over USB (baud: 115200) |

#### Phase 3+: Real Hardware
| GPIO/Interface | Component | Role |
|--------|-----------|------|
| GPIO34 (ADC) | LM35 Sensor | Temperature input (real sensor) |
| GPIO21 (SDA) + GPIO22 (SCL) | INA219 | Current measurement via I2C |
| GPIO26 (PWM) | MOSFET Driver | Control signal for DC fan |
| GPIO23 | Tachometer input | Fan RPM feedback (optional) |
| USB | Raspberry Pi | UART over USB (baud: 115200) |

### FreeRTOS Task Structure
- **UartCommandTask** (P5): Event-driven, handles incoming commands
- **DiagnosticsTask** (P4): 100ms cycle, fault detection
- **FanControlTask** (P4): 100ms cycle, fan mode calculation
- **AnalogInputTask** (P3): 100ms cycle, ADC reading
- **ButtonInputTask** (P3): 50ms cycle, button state
- **PwmOutputTask** (P2): 100ms cycle, LED PWM output
- **StatusReportTask** (P2): Event/500ms, status reporting

### Shared Data Protection
- Use `systemMutex` (FreeRTOS semaphore) to protect:
  - `SensorInput` (temperature, current, rpm, sensorValid, useAdcInput)
  - `SystemStatus` (fanMode, dutyCycle, fault, state)

### Fan Control Logic
| Temperature | Mode | Duty |
|-------------|------|------|
| < 40°C | OFF | 0% |
| 40-69°C | LOW | 40% |
| 70-89°C | MEDIUM | 70% |
| 90-99°C | HIGH | 100% |
| ≥ 100°C | HIGH + FAULT | 100% |

### Fault Handling
- **Sensor Fault**: temp < -40°C OR temp > 150°C OR sensorValid=false → SAFE_MODE, FAN=HIGH, DUTY=100%
- **Over-Current**: current > 2.0A → FAULT_MODE, FAN=OFF, DUTY=0%
- **Fan Stall**: dutyCycle > 0 AND rpm=0 for 1000ms → FAULT_MODE, FAN=OFF, DUTY=0%
- **Recovery**: Fault condition cleared for 3 consecutive 100ms cycles (300ms total)

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

### Phase 1-2: Simulation Mode (Learning)

#### Key Setup Requirements
1. ESP32 firmware requires ESP-IDF 5.x and VS Code + ESP-IDF extension
2. Raspberry Pi must have Python 3.8+ and PySerial installed
3. FreeRTOS timing is critical: DiagnosticsTask and FanControlTask must run every 100ms
4. Mutex protection is essential when accessing shared SensorInput and SystemStatus structures
5. Test bench assumes UART over USB at /dev/ttyUSB0 or /dev/ttyACM0

### Phase 3+: Real Hardware (Production)

#### Additional Setup Requirements
1. **LM35 Temperature Sensor**:
   - Requires ADC calibration for accurate temperature reading
   - Output: ~10mV per °C (linear, accurate from -55 to 150°C)
   - Connection: GPIO34 (ADC1_CH6)
   - Formula: `temperature_C = ADC_voltage / 0.01`

2. **INA219 Current Sensor**:
   - I2C protocol: GPIO21 (SDA) + GPIO22 (SCL)
   - I2C address: 0x40 (default, configurable via A0-A3 pins)
   - Requires esp-idf driver for I2C communication
   - Shunt resistor: 0.1Ω (typical for 3.2A max range)
   - Can measure both current and bus voltage

3. **DC Fan with MOSFET Driver**:
   - GPIO26: PWM control signal (to MOSFET gate)
   - GPIO23: Tachometer feedback (optional, for RPM measurement)
   - Requires flywheel diode protection (1N4007 or similar)
   - MOSFET: AMS1117 3.3V regulator or similar logic-level MOSFET (RDS_on < 100mΩ)
   - Fan voltage: 12V or 24V (depending on motor, power supply separate)

#### I2C Communication Pattern
```c
// Initialize I2C
i2c_config_t conf;
conf.mode = I2C_MODE_MASTER;
conf.sda_io_num = GPIO_NUM_21;
conf.scl_io_num = GPIO_NUM_22;
i2c_param_config(I2C_NUM_0, &conf);
i2c_driver_install(I2C_NUM_0, I2C_MODE_MASTER, 0, 0, 0);

// Read from INA219 (register 0x01 for current)
uint8_t data[2];
i2c_master_read_from_device(I2C_NUM_0, INA219_ADDRESS, data, 2, pdMS_TO_TICKS(10));
int16_t raw_current = (data[0] << 8) | data[1];
float current_mA = (raw_current >> 3) * 0.4; // LSB = 0.4mA
```

### Important Conventions
- All temperature values in °C (simulated via potentiometer ADC or UART commands in Phase 1-2; real LM35 in Phase 3+)
- All duty cycle values as integers 0-100%
- Status responses always include comma-separated key=value pairs
- Fault recovery requires 3 consecutive stable control cycles (300ms minimum)

### Hardware Abstraction Layer (HAL)
To support both simulation and real hardware, firmware must use abstraction:
```c
// hal/sensor_input.h
typedef struct {
    float (*read_temperature)(void);
    float (*read_current)(void);
    int (*read_rpm)(void);
} SensorHAL;

// Simulation implementation
float sim_read_temperature(void) { return g_sensorInput.temperature; }
float sim_read_current(void) { return g_sensorInput.current; }

// Real hardware implementation
float lm35_read_temperature(void) { /* ADC + LM35 conversion */ }
float ina219_read_current(void) { /* I2C + INA219 read */ }
```

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
