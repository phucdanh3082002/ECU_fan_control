# ECU-Like Fan Control & Diagnostics Test Bench

## 1. Executive Summary

Dự án **ECU-Like Fan Control & Diagnostics Test Bench** phát triển một hệ thống điều khiển quạt làm mát kiểu ECU sử dụng **ESP32 DevKit V1** làm embedded controller và **Raspberry Pi 4** làm Python-based test bench.

**Hai giai đoạn phát triển**:
1. **Phase 1-2 (Simulation)**: Dùng Potentiometer, Button, LED để mô phỏng - Foundation & Learning
2. **Phase 3+ (Real Hardware)**: Dùng LM35 (nhiệt độ), INA219 (dòng điện), DC Fan với MOSFET driver - Production-ready

Firmware trên ESP32 được phát triển bằng **ESP-IDF + FreeRTOS** trong VS Code, tập trung vào:
- Fan control logic (temperature-based)
- Diagnostics & fault detection (sensor fault, over-temp, over-current, fan stall)
- Safe mode & fault recovery (3-cycle recovery)
- RTOS task timing & shared data protection
- Hardware abstraction layer (HAL) để hỗ trợ cả simulation và real hardware
---

## 2. Project Objectives

### Main Objectives

#### Phase 1-2: Simulation & Foundation
- Develop an ESP32-based ECU-like fan control module with simulation mode.
- Implement FreeRTOS-based firmware architecture with proper task timing.
- Simulate temperature-based fan control using potentiometer input.
- Simulate fan output using PWM-controlled LED.
- Simulate sensor fault using button input.
- Use Raspberry Pi 4 as an automated Python test bench.
- Validate ESP32 behavior through UART test scenarios.
- Design comprehensive test cases (normal, boundary, invalid input, fault, recovery, RTOS timing).
- Apply Cppcheck for static analysis.
- Prepare host-based unit testing and GCOV/LCOV code coverage.

#### Phase 3+: Real Hardware Integration
- Replace simulation with real temperature sensor (LM35).
- Implement I2C-based current measurement (INA219).
- Control real DC fan with MOSFET driver and tachometer feedback.
- Test real sensor error handling and I2C communication failures.
- Validate system behavior with production-like hardware.
- Maintain backward compatibility with simulation mode through HAL.

---

## 3. System Architecture

### Phase 1-2: Simulation Mode
```text
+---------------------------------------------------+
|                 Raspberry Pi 4                    |
|              Raspberry Pi OS Bookworm             |
|                                                   |
|  Python Test Bench                                |
|  - Send UART test commands                        |
|  - Receive ESP32 status logs                      |
|  - Compare expected vs actual outputs             |
|  - Generate test reports                          |
+-------------------------+-------------------------+
                          |
                          | UART over USB
                          |
+-------------------------v-------------------------+
|                 ESP32 DevKit V1                   |
|              ESP-IDF + FreeRTOS                   |
|                                                   |
|  FreeRTOS Tasks:                                  |
|  - AnalogInputTask (Potentiometer)                |
|  - ButtonInputTask (Sensor fault simulation)      |
|  - UartCommandTask (UART command parsing)         |
|  - FanControlTask (Control logic)                 |
|  - DiagnosticsTask (Fault detection)              |
|  - PwmOutputTask (LED PWM output)                 |
|  - StatusReportTask (Status reporting)            |
|                                                   |
|  Hardware Simulation:                             |
|  - Potentiometer: simulated temperature input     |
|  - Button: simulated sensor fault                 |
|  - LED PWM: simulated fan speed output            |
+---------------------------------------------------+
```

### Phase 3+: Real Hardware
```text
+---------------------------------------------------+
|                 Raspberry Pi 4                    |
|              Raspberry Pi OS Bookworm             |
|                                                   |
|  Python Test Bench                                |
|  - Send UART test commands                        |
|  - Monitor real sensor data                       |
|  - Generate test & validation reports             |
+-------------------------+-------------------------+
                          |
                          | UART over USB
                          |
+-------------------------v-------------------------+
|                 ESP32 DevKit V1                   |
|              ESP-IDF + FreeRTOS + HAL             |
|                                                   |
|  FreeRTOS Tasks:                                  |
|  - SensorReadTask (LM35 + INA219 via ADC/I2C)     |
|  - TachometerTask (RPM feedback via GPIO27)       |
|  - UartCommandTask (UART command parsing)         |
|  - FanControlTask (Control logic)                 |
|  - DiagnosticsTask (Fault detection)              |
|  - FanDriverTask (MOSFET PWM control)             |
|  - StatusReportTask (Status reporting)            |
|                                                   |
|  Real Hardware Sensors:                           |
|  - LM35: Temperature input via GPIO34 (ADC)       |
|  - INA219: Current via GPIO21/22 (I2C)            |
|  - Tachometer: RPM feedback via GPIO27            |
|  - MOSFET Driver: Fan control via GPIO26          |
+---------------------------------------------------+
```

---

## 4. Bill of Materials & Tools

### Phase 1-2: Simulation Mode

| STT | Tên linh kiện/Công cụ | Số lượng | Mục đích/Vai trò |
|---:|---|---:|---|
| 1 | ESP32 DevKit V1 | 1 | Embedded controller, đóng vai ECU-like controller |
| 2 | Raspberry Pi 4 | 1 | Python-based test bench, gửi test scenario và nhận log |
| 3 | Raspberry Pi OS Bookworm | 1 | Hệ điều hành chạy trên Raspberry Pi 4 |
| 4 | USB cable | 1 | UART over USB giữa ESP32 và Raspberry Pi 4, đồng thời dùng để flash firmware |
| 5 | LED | 1-3 | Mô phỏng fan output bằng PWM |
| 6 | Điện trở 220 ohm | 1-3 | Hạn dòng cho LED |
| 7 | Button | 1 | Mô phỏng sensor fault |
| 8 | Potentiometer (10kΩ) | 1 | Mô phỏng temperature input dạng analog |
| 9 | Breadboard | 1 | Lắp mạch thử nghiệm |
| 10 | Jumper wires | 1 bộ | Kết nối ESP32 với LED, button, potentiometer |

### Phase 3+: Real Hardware (Additional Components)

| STT | Tên linh kiện | Số lượng | Mục đích |
|---:|---|---:|---|
| 11 | LM35 Temperature Sensor | 1 | Đo nhiệt độ thực tế (GPIO34 - ADC) |
| 12 | INA219 Current Sensor Module | 1 | Đo dòng điện DC fan (GPIO21/22 - I2C) |
| 13 | DC Fan 12V/24V | 1 | Fan thực tế cần điều khiển |
| 14 | MOSFET Driver (2N7000 / IRF540N) | 1 | Điều khiển gate MOSFET từ GPIO26 |
| 15 | Logic-level MOSFET (AMS1117 / IRF540N) | 1 | Chuyển mạch 12V/24V fan |
| 16 | Flywheel Diode (1N4007) | 1 | Bảo vệ MOSFET khỏi back-EMF của fan |
| 17 | Tachometer Sensor (optional) | 1 | Đo RPM fan feedback (GPIO27) |
| 18 | Pull-up Resistor 10kΩ (I2C) | 2 | I2C SDA/SCL pull-up (nếu cần) |
| 19 | Capacitor 0.1µF | 2 | Decoupling for LM35 + INA219 |
| 20 | Power supply 12V/24V | 1 | Cung cấp nguồn cho DC fan |

### Development Tools (Both Phases)

| STT | Công cụ | Số lượng | Vai trò |
|---:|---|---:|---|
| 21 | VS Code | 1 | IDE phát triển firmware |
| 22 | ESP-IDF 5.x | 1 | Framework chính thức để phát triển ESP32 firmware |
| 23 | FreeRTOS | 1 component | RTOS dùng để chia firmware thành các task |
| 24 | C/C++ | 1 language stack | Viết firmware, control logic, HAL, diagnostics |
| 25 | Python 3.8+ | 1 language stack | Viết test bench trên Raspberry Pi 4 |
| 26 | PySerial | 1 library | Giao tiếp serial giữa Raspberry Pi 4 và ESP32 |
| 27 | Git | 1 tool | Quản lý source code |
| 28 | GitHub | 1 platform | Lưu source code, tài liệu, test reports |
| 29 | GitHub Actions | 1 CI tool | Tự động hóa build, unit test, static analysis |
| 30 | Cppcheck | 1 tool | Static analysis cho source code C/C++ |
| 31 | Unity Test Framework | 1 framework | Unit testing cho ESP-IDF/component logic |
| 32 | GCOV/LCOV | 1 toolset | Đo code coverage cho host-based unit tests |

---

## 5. Hardware Design

### 5.1 Hardware Role

#### Phase 1-2: Simulation Mode

| Component | Vai trò |
|---|---|
| ESP32 DevKit V1 | Xử lý logic điều khiển fan, diagnostics, RTOS tasks |
| Raspberry Pi 4 | Chạy Python test bench để kiểm thử ESP32 |
| Potentiometer | Giả lập nhiệt độ đầu vào (ADC) |
| Button | Giả lập lỗi sensor |
| LED | Giả lập tốc độ quạt bằng PWM |
| USB cable | Giao tiếp UART over USB |

#### Phase 3+: Real Hardware

| Component | Vai trò |
|---|---|
| ESP32 DevKit V1 | Điều khiển fan, đọc sensor LM35 + INA219 qua ADC/I2C |
| Raspberry Pi 4 | Test bench, thu thập dữ liệu thực tế, validate hành vi |
| LM35 Temperature Sensor | Đo nhiệt độ thực (10mV/°C) |
| INA219 Current Sensor | Đo dòng điện DC fan via I2C |
| DC Fan 12V/24V | Fan thực tế cần điều khiển |
| MOSFET Driver + Logic MOSFET | Chuyển mạch 12V/24V fan từ GPIO26 PWM |
| Tachometer Sensor | Đo RPM fan (optional) |
| USB cable | Giao tiếp UART over USB |

---

## 6. GPIO Mapping

### Phase 1-2: Simulation Mode (ESP32-WROOM-32)

| ESP32 Pin | Kết nối | Vai trò |
|---|---|---|
| GPIO34 (ADC1_CH6) | Potentiometer signal | ADC input để mô phỏng temperature |
| GPIO4 (TOUCH0) | Button | Sensor fault input, dùng internal pull-up |
| GPIO26 (DAC_2) | LED qua điện trở 220 ohm | PWM output mô phỏng fan speed |
| USB (GPIO1/GPIO3) | Raspberry Pi 4 | UART over USB (baud: 115200) |

#### Potentiometer Wiring (Phase 1-2)

```text
Potentiometer:
- VCC    -> 3.3V ESP32
- GND    -> GND ESP32
- Signal -> GPIO34 (ADC1_CH6)
```

#### Button Wiring (Phase 1-2)

```text
Button:
- Một chân -> GND
- Một chân -> GPIO4 (TOUCH0)
- GPIO4 dùng internal pull-up (không cần pull-up resistor ngoài)
```

#### LED Wiring (Phase 1-2)

```text
LED:
GPIO26 (DAC_2) -> Resistor 220 ohm -> LED anode
LED cathode -> GND
```

---

### Phase 3+: Real Hardware (ESP32-WROOM-32)

| ESP32 Pin | Kết nối | Vai trò | Protocol |
|---|---|---|---|
| GPIO34 (ADC1_CH6) | LM35 output | Temperature input | ADC (3.3V analog) |
| GPIO21 (WIRE_SDA) | INA219 SDA | I2C data | I2C Master (0x40) |
| GPIO22 (WIRE_SCL) | INA219 SCL | I2C clock | I2C Master (0x40) |
| GPIO26 (DAC_2) | MOSFET gate | Fan PWM control | PWM (3.3V logic) |
| GPIO27 (TOUCH7) | Tachometer input | RPM feedback | GPIO input (optional) |
| USB (GPIO1/GPIO3) | Raspberry Pi 4 | UART over USB | UART (baud: 115200) |

#### LM35 Temperature Sensor Wiring (Phase 3+)

```text
LM35 Pinout (TO-92):
Pin 1 (Vcc)     -> +5V (hoặc +3.3V regulated)
Pin 2 (Vout)    -> GPIO34 (ADC1_CH6)
Pin 3 (GND)     -> GND

Decoupling Capacitor (optional but recommended):
Vcc -> 0.1µF capacitor -> GND (keep leads short)

ADC Configuration (ESP-IDF):
- ADC1_CHANNEL_6 = GPIO34
- Resolution: 12-bit (0-4095)
- Attenuation: ADC_ATTEN_DB_11 (full 0-3.3V range)

ADC Conversion Formula:
ADC_voltage_V = (ADC_raw / 4095.0) * 3.3
temperature_C = ADC_voltage_V / 0.01
```

#### INA219 Current Sensor Wiring (Phase 3+)

```text
INA219 I2C Connections (ESP32-WROOM-32):
SDA -> GPIO21 (WIRE_SDA, pre-labeled with internal pull-up)
SCL -> GPIO22 (WIRE_SCL, pre-labeled with internal pull-up)
VCC -> 3.3V
GND -> GND
A0, A1, A2, A3 -> GND (I2C address: 0x40)

Power Supply Path (for fan monitoring):
+12V/24V -> IN+ (INA219)
Fan motor -> IN- (INA219)
Shunt resistor: 0.1Ω (typically on module)

I2C Communication (ESP-IDF):
- Port: I2C_NUM_0
- Speed: 100kHz or 400kHz
- Address: 0x40 (default, changeable via A0-A3 pins)
- Register 0x01: Current measurement
- Register 0x02: Bus voltage
- LSB (Current): 0.4mA per bit (with 0.1Ω shunt)

I2C Data Read Example:
uint8_t data[2];
i2c_master_read_from_device(I2C_NUM_0, 0x40, data, 2, pdMS_TO_TICKS(10));
int16_t raw_current = (data[0] << 8) | data[1];
float current_mA = (raw_current >> 3) * 0.4;  // LSB = 0.4mA
float current_A = current_mA / 1000.0;
```

#### MOSFET Driver & DC Fan Wiring (Phase 3+)

```text
GPIO26 PWM to MOSFET Driver Circuit:
GPIO26 (3.3V PWM, freq: 1kHz-25kHz) -> Buffer/Driver -> MOSFET Gate

Logic-level MOSFET (N-channel, e.g., IRF540N or similar):
Gate       -> GPIO26 PWM (via small 100Ω resistor for EMI protection)
Drain      -> +12V/24V fan supply
Source     -> Fan motor (common return)
Source/GND -> GND (through flywheel diode)

Flywheel Diode (1N4007 or similar):
Cathode  -> +12V/24V (fan supply positive)
Anode    -> Source (fan motor return to GND)
Purpose: Suppress back-EMF transients when fan stops

Fan Motor Connection:
+12V/24V -> Fan positive (via MOSFET drain)
GND      -> Fan negative (MOSFET source via flywheel diode)

ESP-IDF PWM Configuration:
- Timer: LEDC_TIMER_0
- Channel: LEDC_CHANNEL_0
- GPIO: GPIO26
- Frequency: 5kHz (typical for MOSFET/fan)
- Duty: 0-8191 (max 100% at 8191/8191)
- Resolution: LEDC_TIMER_13_BIT (13-bit = 8191)
```

#### Tachometer Feedback Wiring (Phase 3+, Optional)

```text
Fan Tachometer Sensor:
Tachometer output -> GPIO27 (TOUCH7, with 10kΩ pull-up to 3.3V)
GND              -> GND

Typical tachometer: 1 pulse per revolution
RPM Calculation: RPM = (pulse_count / pulses_per_rev) * 60 / time_seconds

ESP-IDF GPIO Input Configuration:
- GPIO_NUM_27
- GPIO mode: GPIO_MODE_INPUT
- Pull-up: GPIO_PULLUP_ENABLE (internal pull-up recommended)
- Edge interrupt: GPIO_INTR_NEGEDGE or GPIO_INTR_POSEDGE
- Debouncing: Software debouncing with 5-10ms filter

Tachometer Signal Characteristics:
- Frequency: 0 Hz (stalled) to ~100 Hz (for typical fans)
- Voltage: 3.3V logic level
- Duty cycle: Typically 50% square wave
```

#### System Decoupling & Power (Phase 3+)

```text
LM35 Supply Decoupling:
Vcc -> 0.1µF ceramic capacitor -> GND (keep leads short, near sensor pins)

INA219 Supply Decoupling:
VCC -> 0.1µF ceramic capacitor -> GND (keep leads short, near module pins)

I2C Pull-up Resistors (usually on module, verify):
GPIO21 (SDA) -> 10kΩ -> 3.3V (if not on module)
GPIO22 (SCL) -> 10kΩ -> 3.3V (if not on module)

Power Supply Notes:
- 3.3V for ESP32, LM35, INA219: Regulated power supply (LP2950 or similar)
- 12V/24V for DC Fan: Separate power supply with adequate current capacity
- GND: Common ground between all components
```

#### Complete Wiring Summary for Phase 3+ (Real Hardware)

```text
+-------------------+
|   ESP32-WROOM-32  |
|                   |
| 3.3V -----+       |
| GND ------+-------+--- GND (common)
|                   |
| GPIO34 (ADC) ---- LM35 Vout
| GPIO21 (SDA) ---- INA219 SDA
| GPIO22 (SCL) ---- INA219 SCL
| GPIO26 (PWM) ---- MOSFET Gate (via 100Ω)
| GPIO27 (GPIO) --- Tachometer (optional)
|                   |
| GPIO1 (TX) ---+   |
| GPIO3 (RX) ---+-- UART to Raspberry Pi 4
+-------------------+

+-------------------+
|      LM35         |
| Vcc ---- +5V      |
| Vout --- GPIO34   |
| GND ---- GND      |
+-------------------+

+-------------------+
|      INA219       |
| VCC ---- 3.3V     |
| SDA ---- GPIO21   |
| SCL ---- GPIO22   |
| IN+ ---- +12/24V  |
| IN- ---- Fan(-)   |
| GND ---- GND      |
+-------------------+

+-------------------+
|    MOSFET Driver  |
| Gate ---- GPIO26  |
| Drain --- +12/24V |
| Source -- Fan(+)  |
|    & Flywheel     |
+-------------------+

+-------------------+
|     DC Fan Motor  |
| (+) ---- MOSFET   |
| (-) ---- GND      |
+-------------------+
```

#### Button Wiring (Phase 1-2)

```text
Button:
- Một chân -> GND
- Một chân -> GPIO25
- GPIO25 dùng internal pull-up
```

#### LED Wiring (Phase 1-2)

```text
LED:
GPIO26 -> Resistor 220 ohm -> LED anode
LED cathode -> GND
```

---

### Phase 3+: Real Hardware

| ESP32 Pin | Kết nối | Vai trò | Protocol |
|---|---|---|---|
| GPIO34 | LM35 output | Temperature input | ADC (3.3V analog) |
| GPIO21 | INA219 SDA | I2C data | I2C Master (0x40) |
| GPIO22 | INA219 SCL | I2C clock | I2C Master (0x40) |
| GPIO26 | MOSFET gate | Fan PWM control | PWM (3.3V logic) |
| GPIO27 | Tachometer input | RPM feedback | GPIO input (optional) |
| USB | Raspberry Pi 4 | UART over USB | UART (baud: 115200) |

#### LM35 Temperature Sensor Wiring (Phase 3+)

```text
LM35 Pinout (TO-92):
Pin 1 (Vcc)     -> +5V (hoặc +3.3V regulated)
Pin 2 (Vout)    -> GPIO34 (ADC1_CH6)
Pin 3 (GND)     -> GND

Decoupling Capacitor (optional but recommended):
Vcc -> 0.1µF capacitor -> GND (keep leads short)

ADC Conversion:
- ADC voltage range: 0 - 3.3V (if using 3.3V reference)
- LM35 output: 10mV per °C (linear)
- Formula: temperature_C = ADC_voltage_in_volts / 0.01
- Calibration: Configure ADC attenuation for 0-3.3V range
```

#### INA219 Current Sensor Wiring (Phase 3+)

```text
INA219 I2C Connections:
SDA -> GPIO21 (with 10kΩ pull-up to 3.3V if needed)
SCL -> GPIO22 (with 10kΩ pull-up to 3.3V if needed)
VCC -> 3.3V
GND -> GND
A0, A1, A2, A3 -> GND (I2C address: 0x40)

Power Supply Path (for fan monitoring):
+12V/24V -> IN+ (INA219)
Fan motor -> IN- (INA219)
Shunt resistor: 0.1Ω (typically on module)

I2C Communication:
- Address: 0x40 (default, changeable via A0-A3 pins)
- Register 0x01: Current measurement
- LSB: 0.4mA per bit (depends on shunt resistor)
- Bus voltage reading: Available from register 0x02
```

#### MOSFET Driver & DC Fan Wiring (Phase 3+)

```text
MOSFET Driver Circuit:
GPIO26 (3.3V PWM) -> Buffer/Driver -> MOSFET Gate

Logic-level MOSFET (N-channel, e.g., IRF540N):
Gate       -> GPIO26 PWM (via small resistor ~100Ω for EMI protection)
Drain      -> +12V/24V fan supply
Source     -> Fan motor (common return)
Source/GND -> GND (through flywheel diode)

Flywheel Diode (1N4007 or similar):
Cathode  -> +12V/24V (fan supply positive)
Anode    -> Source (fan motor return to GND)
Purpose: Suppress back-EMF transients when fan stops

Fan Motor:
+12V/24V -> Fan positive (via MOSFET drain)
GND      -> Fan negative (MOSFET source)

Tachometer Feedback (optional):
Fan tachometer output -> GPIO27 (with 10kΩ pull-up to 3.3V)
Typically: Hall sensor pulse output (frequency = RPM/60 * pulses_per_rev)
```

#### System Decoupling (Phase 3+)

```text
LM35 Supply Decoupling:
Vcc -> 0.1µF ceramic capacitor -> GND (keep leads short, near sensor pins)

INA219 Supply Decoupling:
VCC -> 0.1µF ceramic capacitor -> GND (keep leads short, near module pins)

I2C Pull-up Resistors (if not on module):
GPIO21 (SDA) -> 10kΩ -> 3.3V
GPIO22 (SCL) -> 10kΩ -> 3.3V
```

---

## 7. Functional Requirements

### 7.1 Temperature Input

#### Phase 1-2: Simulation
- Source: Potentiometer ADC (GPIO34)
- Range: 0°C - 100°C (via UART command or ADC mapping)
- Mode: Can be switched via `USE_ADC_INPUT:1` (potentiometer) or `USE_ADC_INPUT:0` (UART)

#### Phase 3+: Real Hardware
- Source: LM35 Temperature Sensor (GPIO34 ADC)
- Range: -40°C to +150°C (typical LM35 range)
- Accuracy: ±0.5°C (typical)
- Output: Linear 10mV per °C
- Formula: `temp_C = ADC_voltage_volts / 0.01`
- ADC Configuration: 12-bit resolution, 3.3V reference, attenuation set to 11dB for full range
- I2C Dependency: None (independent sensor)

---

### 7.2 Current Input

#### Phase 1-2: Simulation
- Source: UART command `SET_CURRENT:1.2`
- Range: 0A - 3.2A
- Mode: Mô phỏng, không có sensor thật

#### Phase 3+: Real Hardware
- Source: INA219 Current Sensor (I2C, GPIO21/22)
- Range: -3.2A to +3.2A (depends on shunt resistor, typically 0.1Ω)
- Accuracy: ±0.8% (typical)
- Resolution: LSB = 0.4mA (with 0.1Ω shunt)
- I2C Address: 0x40 (default)
- Register: 0x01 (current measurement)
- Bus Voltage: Available on register 0x02 for monitoring
- I2C Communication: 100kHz or 400kHz, write/read 2 bytes per transaction

---

### 7.3 RPM Feedback

#### Phase 1-2: Simulation
- Source: UART command `SET_RPM:1200`
- Range: 0 - 5000 RPM (simulated)
- Mode: Mô phỏng, không có feedback thật

#### Phase 3+: Real Hardware
- Source: Tachometer sensor (GPIO27, optional)
- Range: 0 - 10000 RPM (depends on fan)
- Type: Hall effect sensor or optical encoder
- Signal: Pulse frequency (typically 1 pulse per revolution for simple fans)
- Debouncing: Hardware or software debouncing required
- I2C Dependency: None (GPIO-based input capture)
- Timeout: 1000ms with DUTY > 0 and RPM = 0 → FAN_STALL fault

---

### 7.4 Fan Control Logic

The system shall calculate fan mode and duty cycle based on temperature input.

| Temperature Range | Fan Mode | Duty Cycle |
|---|---|---:|
| Temperature < 40°C | FAN_OFF | 0% |
| 40°C - 69°C | FAN_LOW | 40% |
| 70°C - 89°C | FAN_MEDIUM | 70% |
| 90°C - 99°C | FAN_HIGH | 100% |
| >= 100°C | FAN_HIGH + OVER_TEMPERATURE | 100% |

**Implementation Note**: Logic must be independent of input source (ADC/UART in Phase 1-2, LM35/INA219 in Phase 3+) via HAL abstraction.

---

### 7.5 Sensor Fault Detection

#### Phase 1-2: Simulation
- Button press → `SET_SENSOR_VALID:0` command

#### Phase 3+: Real Hardware
Sensor fault shall be triggered when:

```text
temperature < -40°C (LM35 out of range)
temperature > 150°C (LM35 out of range)
sensorValid = false (from UART override)
I2C error reading INA219 (bus failure, timeout)
I2C error reading GPIO27 tachometer (debouncing failure)
```

Expected behavior:

```text
FAULT = SENSOR_FAULT
STATE = SAFE_MODE
FAN = FAN_HIGH
DUTY = 100%
```

---

### 7.6 Over-Temperature Detection

Over-temperature shall be triggered when:

```text
temperature >= 100°C
```

Expected behavior:

```text
FAULT = OVER_TEMPERATURE
STATE = FAULT_MODE
FAN = FAN_HIGH
DUTY = 100%
```

---

### 7.7 Over-Current Detection

#### Phase 1-2: Simulation
- Source: UART command `SET_CURRENT:2.1`

#### Phase 3+: Real Hardware
- Source: INA219 sensor (I2C)

Over-current shall be triggered when:

```text
current > 2.0A (from INA219 measurement or UART)
```

Expected behavior:

```text
FAULT = OVER_CURRENT
STATE = FAULT_MODE
FAN = FAN_OFF
DUTY = 0%
```

---

### 7.8 Fan Stall Detection

#### Phase 1-2: Simulation
- Condition: `DUTY > 0 AND RPM = 0 for 1000 ms` (from UART simulation)

#### Phase 3+: Real Hardware
- Condition: `DUTY > 0 AND RPM = 0 for 1000 ms` (from tachometer GPIO27)
- Requires tachometer sensor connected and enabled

Fan stall shall be triggered when:

```text
dutyCycle > 0
rpm = 0
condition persists for 1000 ms
```

Expected behavior:

```text
FAULT = FAN_STALL
STATE = FAULT_MODE
FAN = FAN_OFF
DUTY = 0%
```

---

### 7.9 Fault Recovery

Fault recovery condition:

```text
Fault condition must be cleared for 3 consecutive control cycles.
```

Control cycle:

```text
100 ms
```

Recovery duration:

```text
3 cycles = 300 ms
```

Expected behavior after recovery:

```text
FAULT = NONE
STATE = NORMAL
Fan control returns to temperature-based logic
```

---

## 8. System States

| State | Ý nghĩa |
|---|---|
| NORMAL | Hệ thống hoạt động bình thường |
| SAFE_MODE | Hệ thống vào trạng thái an toàn do sensor fault |
| FAULT_MODE | Hệ thống có lỗi nghiêm trọng như over-current, fan stall, over-temperature |

---

## 9. Fault Codes

| Fault Code | Ý nghĩa |
|---|---|
| FAULT_NONE | Không có lỗi |
| FAULT_SENSOR | Lỗi cảm biến hoặc input không hợp lệ |
| FAULT_OVER_TEMP | Nhiệt độ vượt ngưỡng cho phép |
| FAULT_OVER_CURRENT | Dòng điện mô phỏng vượt ngưỡng |
| FAULT_FAN_STALL | Có lệnh chạy fan nhưng RPM bằng 0 trong thời gian timeout |

---

## 10. FreeRTOS Firmware Design

### 10.1 Task List

#### Phase 1-2: Simulation Tasks

| Task | Chu kỳ | Priority | Vai trò |
|---|---:|---:|---|
| UartCommandTask | Event-driven | 5 | Nhận command từ Raspberry Pi 4 |
| DiagnosticsTask | 100 ms | 4 | Kiểm tra fault conditions |
| FanControlTask | 100 ms | 4 | Tính fan mode và duty cycle |
| AnalogInputTask | 100 ms | 3 | Đọc potentiometer qua ADC |
| ButtonInputTask | 50 ms | 3 | Đọc button sensor fault |
| PwmOutputTask | 100 ms | 2 | Xuất PWM ra LED |
| StatusReportTask | Event-based / 500 ms | 2 | Gửi status về Raspberry Pi 4 |

#### Phase 3+: Real Hardware Tasks

| Task | Chu kỳ | Priority | Vai trò |
|---|---:|---:|---|
| UartCommandTask | Event-driven | 5 | Nhận command từ Raspberry Pi 4 |
| SensorReadTask | 100 ms | 4 | Đọc LM35 (ADC) + INA219 (I2C) |
| TachometerTask | 100 ms | 4 | Đọc RPM từ GPIO27 (tachometer) |
| DiagnosticsTask | 100 ms | 4 | Kiểm tra fault conditions |
| FanControlTask | 100 ms | 4 | Tính fan mode và duty cycle |
| FanDriverTask | 100 ms | 3 | Xuất PWM ra MOSFET (GPIO26) |
| StatusReportTask | Event-based / 500 ms | 2 | Gửi status về Raspberry Pi 4 |

**Task Timing Note**: Critical tasks (DiagnosticsTask, FanControlTask) must run every 100ms ± 5ms for consistent fault detection and control response.

---

### 10.2 Shared Data Structures

```c
typedef enum {
    FAN_OFF,
    FAN_LOW,
    FAN_MEDIUM,
    FAN_HIGH
} FanMode;

typedef enum {
    FAULT_NONE,
    FAULT_SENSOR,
    FAULT_OVER_TEMP,
    FAULT_OVER_CURRENT,
    FAULT_FAN_STALL
} FaultCode;

typedef enum {
    STATE_NORMAL,
    STATE_SAFE_MODE,
    STATE_FAULT_MODE
} SystemState;

typedef struct {
    float temperature;      // °C
    float current;          // A
    int rpm;                // RPM
    bool sensorValid;       // Override flag
    bool useAdcInput;       // Phase 1-2: potentiometer, Phase 3+: LM35
} SensorInput;

typedef struct {
    FanMode fanMode;
    int dutyCycle;          // 0-100%
    FaultCode fault;
    SystemState state;
} SystemStatus;
```

Shared state shall be protected using a mutex.

```c
SemaphoreHandle_t systemMutex;
SensorInput g_sensorInput;
SystemStatus g_systemStatus;
```

---

### 10.3 Hardware Abstraction Layer (HAL)

To support both simulation and real hardware, firmware must use HAL:

```c
// hal/sensor_input.h
typedef struct {
    float (*read_temperature)(void);    // Returns °C
    float (*read_current)(void);        // Returns A
    int (*read_rpm)(void);              // Returns RPM
    bool (*is_sensor_valid)(void);      // Returns true if valid
} SensorHAL;

// Phase 1-2: Simulation implementation (sensor_input_sim.c)
float sim_read_temperature(void) {
    // Return from g_sensorInput.temperature (set via UART)
    return g_sensorInput.temperature;
}

float sim_read_current(void) {
    // Return from g_sensorInput.current (set via UART)
    return g_sensorInput.current;
}

int sim_read_rpm(void) {
    // Return from g_sensorInput.rpm (set via UART)
    return g_sensorInput.rpm;
}

bool sim_is_sensor_valid(void) {
    // Use g_sensorInput.sensorValid flag
    return g_sensorInput.sensorValid;
}

static SensorHAL g_sim_hal = {
    .read_temperature = sim_read_temperature,
    .read_current = sim_read_current,
    .read_rpm = sim_read_rpm,
    .is_sensor_valid = sim_is_sensor_valid
};

// Phase 3+: Real hardware implementation (sensor_input_real.c)
float lm35_read_temperature(void) {
    // Read GPIO34 ADC, convert: temp = ADC_voltage / 0.01
    uint32_t adc_raw = adc1_get_raw(ADC1_CHANNEL_6);
    float adc_voltage = (adc_raw / 4095.0) * 3.3; // 12-bit ADC, 3.3V ref
    return adc_voltage / 0.01;  // LM35: 10mV per °C
}

float ina219_read_current(void) {
    // Read INA219 register 0x01 via I2C, convert raw to A
    uint8_t data[2];
    i2c_master_read_from_device(I2C_NUM_0, 0x40, data, 2, pdMS_TO_TICKS(10));
    int16_t raw_current = (data[0] << 8) | data[1];
    float current_mA = (raw_current >> 3) * 0.4;  // LSB = 0.4mA
    return current_mA / 1000.0;  // Convert to A
}

int gpio_read_rpm(void) {
    // Read GPIO27 tachometer with debouncing/filtering
    // Count pulse frequency over 100ms window
    // Example: 1 pulse per rev, so frequency = RPM/60
    return g_tachometer_rpm;  // Set by tachometer input task
}

bool real_is_sensor_valid(void) {
    // Check I2C errors, ADC errors, timeout conditions
    return (i2c_error_count == 0 && adc_initialized && ...);
}

static SensorHAL g_real_hal = {
    .read_temperature = lm35_read_temperature,
    .read_current = ina219_read_current,
    .read_rpm = gpio_read_rpm,
    .is_sensor_valid = real_is_sensor_valid
};

// Active HAL pointer (set at initialization)
SensorHAL *g_sensor_hal = NULL;

// Initialize based on compilation flag or runtime detection
void sensor_hal_init(void) {
    #ifdef SIMULATION_MODE
        g_sensor_hal = &g_sim_hal;
    #else
        i2c_master_init();  // Initialize I2C for INA219
        adc1_config_width(ADC_WIDTH_BIT_12);  // Configure LM35 ADC
        adc1_config_channel_atten(ADC1_CHANNEL_6, ADC_ATTEN_DB_11);  // Full range
        g_sensor_hal = &g_real_hal;
    #endif
}

// In SensorReadTask or AnalogInputTask:
// float temp = g_sensor_hal->read_temperature();
// float curr = g_sensor_hal->read_current();
// int rpm = g_sensor_hal->read_rpm();
```

---

## 11. UART Protocol

### 11.1 Communication Method

```text
UART over USB
ESP32 DevKit V1 USB port <-> Raspberry Pi 4 USB port
Baud rate: 115200
```

---

### 11.2 Commands from Raspberry Pi 4 to ESP32

| Command | Ý nghĩa |
|---|---|
| SET_TEMP:85 | Set simulated temperature to 85°C |
| SET_CURRENT:1.2 | Set simulated current to 1.2A |
| SET_RPM:1200 | Set simulated fan RPM |
| SET_SENSOR_VALID:1 | Sensor status valid |
| SET_SENSOR_VALID:0 | Simulate sensor fault |
| USE_ADC_INPUT:1 | Use potentiometer as temperature input |
| USE_ADC_INPUT:0 | Use temperature from UART command |
| CLEAR_FAULT | Clear fault if recovery condition is satisfied |
| GET_STATUS | Request current system status |

---

### 11.3 ESP32 Response Format

Normal response:

```text
STATUS,TEMP=85,CURRENT=1.2,RPM=1200,FAN=MEDIUM,DUTY=70,FAULT=NONE,STATE=NORMAL
```

Sensor fault response:

```text
STATUS,TEMP=200,CURRENT=0.5,RPM=1200,FAN=HIGH,DUTY=100,FAULT=SENSOR_FAULT,STATE=SAFE_MODE
```

Fan stall response:

```text
STATUS,TEMP=80,CURRENT=0.5,RPM=0,FAN=OFF,DUTY=0,FAULT=FAN_STALL,STATE=FAULT_MODE
```

---

## 12. Raspberry Pi 4 Test Bench Design

### 12.1 Python Modules

```text
pi_test_bench/

├── serial_client.py
├── test_runner.py
├── test_cases.json
├── report_generator.py
└── reports/
```

### 12.2 Module Responsibilities

| File | Vai trò |
|---|---|
| serial_client.py | Mở serial port, gửi command, nhận response |
| test_runner.py | Chạy toàn bộ test cases |
| test_cases.json | Lưu test scenarios và expected outputs |
| report_generator.py | Tạo CSV/HTML test report |
| reports/ | Lưu kết quả test |

---

## 13. Test Strategy

The project shall include the following testing layers:

```text
1. Unit Testing
2. Integration Testing
3. Boundary Testing
4. Invalid Input Testing
5. Fault Testing
6. Fault Recovery Testing
7. RTOS Timing Testing
8. Static Analysis
9. Host-based Code Coverage Preparation
```

---

## 14. Test Cases

### 14.1 Normal Test Cases

| Test ID | Input | Expected Output |
|---|---|---|
| TC_001 | TEMP=30 | FAN=OFF, DUTY=0, FAULT=NONE, STATE=NORMAL |
| TC_002 | TEMP=50 | FAN=LOW, DUTY=40, FAULT=NONE, STATE=NORMAL |
| TC_003 | TEMP=80 | FAN=MEDIUM, DUTY=70, FAULT=NONE, STATE=NORMAL |
| TC_004 | TEMP=95 | FAN=HIGH, DUTY=100, FAULT=NONE, STATE=NORMAL |

---

### 14.2 Boundary Test Cases

| Test ID | Input | Expected Output |
|---|---|---|
| TC_005 | TEMP=39 | FAN=OFF |
| TC_006 | TEMP=40 | FAN=LOW |
| TC_007 | TEMP=69 | FAN=LOW |
| TC_008 | TEMP=70 | FAN=MEDIUM |
| TC_009 | TEMP=89 | FAN=MEDIUM |
| TC_010 | TEMP=90 | FAN=HIGH |
| TC_011 | TEMP=99 | FAN=HIGH |
| TC_012 | TEMP=100 | FAULT=OVER_TEMPERATURE, DUTY=100 |

---

### 14.3 Invalid Input Test Cases

| Test ID | Input | Expected Output |
|---|---|---|
| TC_013 | TEMP=-50 | FAULT=SENSOR_FAULT, STATE=SAFE_MODE, DUTY=100 |
| TC_014 | TEMP=151 | FAULT=SENSOR_FAULT, STATE=SAFE_MODE, DUTY=100 |
| TC_015 | SET_SENSOR_VALID:0 | FAULT=SENSOR_FAULT, STATE=SAFE_MODE, DUTY=100 |

---

### 14.4 Fault Test Cases

| Test ID | Input | Expected Output |
|---|---|---|
| TC_016 | TEMP=80, CURRENT=2.1 | FAULT=OVER_CURRENT, STATE=FAULT_MODE, DUTY=0 |
| TC_017 | TEMP=80, RPM=0 for 1000 ms | FAULT=FAN_STALL, STATE=FAULT_MODE, DUTY=0 |
| TC_018 | TEMP=100 | FAULT=OVER_TEMPERATURE, DUTY=100 |

---

### 14.5 Recovery Test Cases

| Test ID | Steps | Expected Output |
|---|---|---|
| TC_019 | SET_SENSOR_VALID:0 -> SET_SENSOR_VALID:1 -> SET_TEMP:60 | After 3 stable cycles: FAULT=NONE, STATE=NORMAL, FAN=LOW |
| TC_020 | SET_CURRENT:2.1 -> SET_CURRENT:1.0 | After 3 stable cycles: FAULT=NONE, STATE=NORMAL |

---

### 14.6 RTOS Timing Test Cases

| Test ID | Input | Expected Output |
|---|---|---|
| TC_RTOS_001 | SET_TEMP:80 | FAN=MEDIUM within 200 ms |
| TC_RTOS_002 | GET_STATUS | Response received within 500 ms |
| TC_RTOS_003 | SET_TEMP:30 then SET_TEMP:95 quickly | Latest value is reflected correctly |
| TC_RTOS_004 | SET_SENSOR_VALID:0 then SET_SENSOR_VALID:1 | Recovery after 3 stable cycles |

---

## 15. Static Analysis

### Tool

```text
Cppcheck
```

### Target Source

```text
esp32_firmware/src
```

### Example Command

```bash
cppcheck --enable=all --inconclusive --std=c++11 esp32_firmware/src
```

### Export Report

```bash
cppcheck --enable=all --inconclusive --std=c++11 esp32_firmware/src 2> static_analysis/cppcheck_report.txt
```

### Expected Outcome

- Detect potential C/C++ issues.
- Review uninitialized variables, unused functions, invalid conditions and maintainability warnings.
- Store report in `static_analysis/cppcheck_report.txt`.

---

## 16. Unit Testing & Code Coverage Plan

### Unit Test Framework

```text
Unity Test Framework
```

### Test Target

```text
fan_control.c / fan_control.cpp
diagnostics.c / diagnostics.cpp
```

### Host-Based Test Plan

Core logic should be separated from ESP32 hardware-specific code to support host-based unit testing.

Testable modules:

```text
fan_control
diagnostics
fault_recovery
```

### GCOV/LCOV Direction

GCOV/LCOV will be prepared for host-based unit tests only.

Correct project statement:

```text
Preparing GCOV/LCOV coverage measurement for host-based unit tests of fan control and diagnostics logic.
```

Avoid claiming:

```text
Full ESP32 firmware coverage using GCOV/LCOV
```

---

## 17. Suggested Repository Structure

```text
ecu-like-fan-control-diagnostics-test-bench/

├── README.md
├── docs/
│   ├── system_architecture.md
│   ├── hardware_wiring.md
│   ├── rtos_design.md
│   ├── uart_protocol.md
│   ├── test_plan.md
│   └── test_report_sample.md
│
├── esp32_firmware/
│   ├── main/
│   │   ├── main.cpp
│   │   ├── fan_control.cpp
│   │   ├── fan_control.h
│   │   ├── diagnostics.cpp
│   │   ├── diagnostics.h
│   │   ├── uart_protocol.cpp
│   │   ├── uart_protocol.h
│   │   ├── app_tasks.cpp
│   │   └── app_tasks.h
│   │
│   ├── CMakeLists.txt
│   └── sdkconfig
│
├── unit_tests/
│   ├── test_fan_control.cpp
│   └── test_diagnostics.cpp
│
├── pi_test_bench/
│   ├── serial_client.py
│   ├── test_runner.py
│   ├── test_cases.json
│   ├── report_generator.py
│   └── reports/
│
├── static_analysis/
│   └── cppcheck_report.txt
│
├── coverage/
│   └── README.md
│
└── .github/
    └── workflows/
        └── ci.yml
```

---

## 18. Development Roadmap

## Phase 1: Environment Setup

### Tasks

- Install VS Code.
- Install ESP-IDF extension.
- Setup ESP-IDF 5.x toolchain.
- Create ESP-IDF project for ESP32 DevKit V1.
- Install Python 3.8+ and PySerial on Raspberry Pi OS Bookworm.
- Connect ESP32 to Raspberry Pi 4 via USB.
- Verify serial port:

```bash
ls /dev/ttyUSB*
ls /dev/ttyACM*
```

- Flash a basic ESP32 hello-world or serial echo firmware.

### Outcome

- ESP32 can be built and flashed from VS Code.
- Raspberry Pi 4 can communicate with ESP32 through UART over USB.

---

## Phase 2: Basic Hardware Demo (Simulation)

### Tasks

- Connect LED to GPIO26 through 220 ohm resistor.
- Connect button to GPIO25 using internal pull-up.
- Connect potentiometer signal to GPIO34 (ADC).
- Write ESP32 firmware to:
  - Read ADC value from potentiometer.
  - Read button state.
  - Generate PWM output to LED.
  - Print ADC/button/PWM status through UART.

### Outcome

- ESP32 can read physical input and control LED PWM output.
- Hardware simulation is ready.

---

## Phase 3: Core Control Logic

### Tasks

- Implement `fan_control` module (independent of hardware).
- Implement fan mode thresholds.
- Implement duty cycle output.
- Implement `diagnostics` module.
- Implement:
  - sensor fault detection,
  - over-temperature detection,
  - over-current detection,
  - fan stall detection,
  - safe mode behavior,
  - fault mode behavior,
  - fault recovery after 3 stable cycles.

### Outcome

- Fan control and diagnostics logic work correctly through manual testing.
- Logic is independent of input source (potentiometer vs LM35, UART simulation vs INA219).

---

## Phase 4: FreeRTOS Task Architecture

### Tasks

- Create FreeRTOS tasks (Phase 1-2 version):
  - AnalogInputTask (potentiometer)
  - ButtonInputTask (button)
  - UartCommandTask (UART)
  - FanControlTask
  - DiagnosticsTask
  - PwmOutputTask (LED)
  - StatusReportTask
- Implement shared system state (SensorInput, SystemStatus).
- Protect shared data using mutex (systemMutex).
- Use `vTaskDelayUntil()` for periodic tasks.
- Verify task timing and system stability (100ms cycles).

### Outcome

- Firmware has clear RTOS-based architecture.
- Periodic control loop and diagnostics loop are working.
- Phase 1-2 simulation complete and tested.

---

## Phase 5: UART Protocol Implementation

### Tasks

- Implement command parser for:
  - SET_TEMP
  - SET_CURRENT
  - SET_RPM
  - SET_SENSOR_VALID
  - USE_ADC_INPUT
  - CLEAR_FAULT
  - GET_STATUS
- Implement response formatter (comma-separated key=value).
- Validate UART commands manually from Raspberry Pi.

### Outcome

- Raspberry Pi can control test scenarios using UART commands.
- ESP32 returns structured status response.

---

## Phase 6: Python Test Bench

### Tasks

- Implement `serial_client.py` (send commands, receive responses).
- Implement `test_cases.json` (test scenarios and expected outputs).
- Implement `test_runner.py` (execute test cases, collect results).
- Implement response parser.
- Compare actual output with expected output.
- Generate CSV test report.

### Outcome

- Raspberry Pi automatically runs integration tests against ESP32.
- Test report is generated and stored.
- Phase 1-2 integration test complete.

---

## Phase 7: Test Case Completion (Simulation)

### Tasks

- Add normal test cases (TC_001-004).
- Add boundary test cases (TC_005-012).
- Add invalid input test cases (TC_013-015).
- Add fault test cases (TC_016-018).
- Add recovery test cases (TC_019-020).
- Add RTOS timing test cases (TC_RTOS_001-004).
- Run full regression test suite.

### Outcome

- Project has a complete test strategy for simulation mode.
- PASS/FAIL reports are available.
- All Phase 1-2 tests pass.

---

## Phase 8: Static Analysis & Code Quality

### Tasks

- Run Cppcheck on ESP32 firmware source code:
  ```bash
  cppcheck --enable=all --inconclusive --std=c++11 esp32_firmware/src
  ```
- Store report in `static_analysis/cppcheck_report.txt`.
- Review and fix critical warnings (null pointers, memory leaks).
- Document static analysis results.

### Outcome

- Static analysis evidence is available.
- Code quality improved.
- No critical warnings remain.

---

## Phase 9: Unit Testing & Code Coverage (Host-based)

### Tasks

- Setup Unity test framework on host (Linux/macOS/Windows).
- Separate core logic from hardware-specific code:
  - `fan_control.c` (pure logic, no hardware)
  - `diagnostics.c` (pure logic, no hardware)
  - `fault_recovery.c` (pure logic, no hardware)
  - `hal/sensor_input.h` (HAL interface)
- Write unit tests for:
  - fan control thresholds (40°C, 70°C, 90°C, 100°C boundaries).
  - diagnostics fault detection (sensor fault, over-temp, etc.).
  - recovery logic (3-cycle counting).
- Prepare host-based test build (mock HAL implementation).
- Prepare GCOV/LCOV coverage measurement for core modules.

### Outcome

- Core logic can be tested without flashing ESP32.
- Coverage measurement is prepared.
- Host-based unit tests pass with >80% coverage.

---

## Phase 10: Hardware Abstraction Layer (HAL) - Simulation

### Tasks

- Create `hal/sensor_input.h` (abstract interface).
- Implement simulation HAL (`hal/sensor_input_sim.c`):
  - `sim_read_temperature()` → returns from UART command
  - `sim_read_current()` → returns from UART command
  - `sim_read_rpm()` → returns from UART command
  - `sim_is_sensor_valid()` → returns flag from UART
- Create CMakeLists.txt conditional compilation for simulation mode.
- Verify Phase 1-2 firmware still works with HAL abstraction.

### Outcome

- Firmware can switch between simulation and real hardware via compilation flag.
- No code duplication; single codebase supports both.
- Phase 1-2 functionality unchanged.

---

## Phase 11: Real Hardware Preparation (Design & Integration)

### Tasks

- Design real hardware wiring diagram (LM35, INA219, MOSFET, fan, tachometer).
- Procure and test individual components:
  - LM35 temperature sensor (verify ADC read at various temps).
  - INA219 current sensor module (verify I2C communication, calibration).
  - DC fan + MOSFET driver (verify PWM control, fan spin).
  - Tachometer sensor (verify pulse detection on GPIO27).
- Create I2C driver wrapper for INA219 (register read/write).
- Create ADC driver wrapper for LM35 (voltage to temperature conversion).
- Create tachometer input task (GPIO27 pulse counting).
- Implement error handling for I2C failures and sensor timeouts.
- Document real hardware wiring in `docs/hardware_phase3_real.md`.

### Outcome

- Hardware is procured and tested individually.
- Driver code for LM35 (ADC) and INA219 (I2C) is ready.
- Tachometer input task is designed.
- I2C error handling is implemented.

---

## Phase 12: Real Hardware Integration - Firmware Update

### Tasks

- Implement real hardware HAL (`hal/sensor_input_real.c`):
  - `lm35_read_temperature()` → ADC read, convert via 10mV/°C formula
  - `ina219_read_current()` → I2C read from register 0x01, convert via LSB
  - `gpio_read_rpm()` → debounced tachometer pulse counting
  - `real_is_sensor_valid()` → check I2C errors, ADC errors, timeout conditions
- Replace simulation tasks with real hardware tasks:
  - Remove: AnalogInputTask (potentiometer), ButtonInputTask
  - Add: SensorReadTask (LM35 + INA219), TachometerTask (GPIO27)
  - Update: PwmOutputTask → FanDriverTask (MOSFET control)
- Implement I2C error recovery (retry logic, I2C bus reset).
- Implement ADC calibration for LM35 (if needed for accuracy).
- Test real hardware in controlled environment (lab setup).

### Outcome

- Real hardware sensors are integrated and working.
- All sensors (LM35, INA219, tachometer) read correctly.
- Error handling for I2C/ADC failures is working.
- Phase 3+ firmware is production-ready.

---

## Phase 13: Real Hardware Test Suite

### Tasks

- Update test cases for real hardware:
  - TC_REAL_001: Temperature range validation (-40°C to +150°C).
  - TC_REAL_002: Current measurement accuracy (0A to 3.2A).
  - TC_REAL_003: Fan RPM feedback (0 to 5000 RPM).
  - TC_REAL_004: Thermal stress test (stable operation at 100°C for 1 hour).
  - TC_REAL_005: I2C error recovery (INA219 temporary failure).
  - TC_REAL_006: Fan stall detection with real fan.
  - TC_REAL_007: Fault recovery with real sensors.
- Run full integration test suite on Raspberry Pi.
- Validate test results against expected outputs.
- Generate test report with real sensor data.

### Outcome

- Real hardware test suite is complete.
- All tests pass on Phase 3+ hardware.
- Integration test report is generated.

---

## Phase 14: GitHub Actions CI/CD

### Tasks

- Create `.github/workflows/ci.yml`:
  - Build host-based unit tests.
  - Run unit tests with Unity framework.
  - Measure code coverage (GCOV/LCOV).
  - Run Cppcheck static analysis.
  - Build ESP32 firmware (simulation mode).
  - (Optional) Automated UART integration tests on CI runner.
- Store logs and reports as GitHub artifacts.
- Add CI badge to README.

### Outcome

- Repository has automated CI workflow.
- Code quality is continuously monitored.
- Pull requests are automatically tested.

---

## Phase 15: Documentation & Finalization

### Tasks

Complete documentation:

```text
README.md (main project overview)
docs/system_architecture.md (system design, Phase 1-2 & 3+)
docs/hardware_wiring.md (wiring diagrams for both phases)
docs/hardware_phase1_simulation.md (potentiometer, button, LED details)
docs/hardware_phase3_real.md (LM35, INA219, fan, MOSFET details)
docs/rtos_design.md (task architecture, shared data, mutex)
docs/uart_protocol.md (command format, response format)
docs/hal_abstraction.md (HAL design, simulation vs real)
docs/test_plan.md (test strategy, test cases, expected results)
docs/test_report_sample.md (sample test report format)
docs/ci_cd.md (GitHub Actions workflow explanation)
```

README should include:

```text
- Overview (Phase 1-2 & 3+)
- Objectives
- Hardware (both phases)
- GPIO Mapping (both phases)
- System architecture diagrams
- RTOS task design
- UART protocol
- HAL abstraction
- Test strategy & results
- Static analysis summary
- Code coverage summary
- How to build (Phase 1-2)
- How to build (Phase 3+)
- Known limitations
- Future improvements
```

### Outcome

- GitHub repo is clear and professional.
- Documentation covers both simulation and real hardware.
- Project is ready for CV, GitHub portfolio, and interviews.

---

## Phase 16: Final Review & Portfolio Integration

### Tasks

- Run final integration test on both Phase 1-2 (simulation) and Phase 3+ (real hardware).
- Run static analysis, verify coverage.
- Update README with test results and benchmarks.
- Add demo photos or videos (if available).
- Update CV project description.
- Prepare interview explanation (30-60 seconds).

### Suggested CV Description

```text
ECU-Like Fan Control & Diagnostics Test Bench
Completed | ESP32 DevKit V1, Raspberry Pi 4, ESP-IDF, FreeRTOS, C/C++, Python, I2C/ADC, Unit/Integration Testing

Phase 1-2 (Completed - Simulation Mode):
- Implemented FreeRTOS-based ESP32 firmware with 7 concurrent tasks for fan control, diagnostics, and safe mode handling
- Simulated temperature input via potentiometer ADC, sensor faults via button, and fan output via LED PWM
- Designed and tested 20+ test cases covering normal operation, boundary conditions, fault injection, and recovery scenarios

Phase 3+ (Completed - Real Hardware Integration):
- Integrated real temperature sensor (LM35) via ADC and current sensor (INA219) via I2C for production-like monitoring
- Implemented tachometer feedback (GPIO27) for fan RPM detection and fan stall diagnosis
- Designed hardware abstraction layer (HAL) to support both simulation and real hardware from single codebase
- Implemented I2C error handling, ADC calibration, and sensor timeout detection for robustness

Testing & Quality:
- Created Python test bench on Raspberry Pi 4 with 20+ automated UART test cases and CSV reporting
- Applied Cppcheck static analysis and prepared host-based unit tests with Unity framework and GCOV/LCOV coverage
- All test suites pass on both simulation (Phase 1-2) and real hardware (Phase 3+) configurations

Key Skills Demonstrated:
Embedded systems (ESP32/FreeRTOS), real sensor integration (LM35/INA219), hardware abstraction, I2C/ADC communication, 
RTOS task synchronization, fault detection & recovery, comprehensive testing strategy, CI/CD automation (GitHub Actions)
```

### Interview Explanation (60 seconds)

```text
I developed an ECU-like fan control system in two phases to demonstrate embedded software testing for automotive applications.

Phase 1-2 focuses on the foundation: I simulated temperature, faults, and fan output using a potentiometer, button, and LED 
on an ESP32 with FreeRTOS. The system includes 7 concurrent tasks that implement fan control logic (40°C-100°C temperature 
thresholds), three types of fault detection (sensor, over-temp, over-current, fan stall), and automatic recovery after 300ms.

Phase 3+ takes it to production: I integrated real sensors—LM35 for temperature via ADC and INA219 for current via I2C. 
I designed a hardware abstraction layer (HAL) so the same firmware works with both simulation and real hardware, just 
by changing a compilation flag. This demonstrates clean architecture and code reusability.

For testing, I built a Python test bench on Raspberry Pi that sends UART commands and validates ESP32 responses. I designed 
20+ test cases covering normal operation, boundaries, fault injection, and recovery. I also applied Cppcheck for static 
analysis and prepared host-based unit tests with code coverage measurement.

The biggest learning: designing systems that are testable from day one. The core logic (fan control, diagnostics) is 
completely independent of hardware input sources, making it easy to test on a host computer. This is critical for 
automotive-grade software where reliability is non-negotiable.
```

### Outcome

- Project is production-quality and ready for portfolio/interviews.
- Candidate can clearly articulate Phase 1-2 vs Phase 3+ approach.
- Code demonstrates real-world embedded system design principles.

---

## 19. Known Limitations

### Phase 1-2: Simulation Mode
- Hardware simulation uses potentiometer, button, LED (not real sensors/actuators)
- Temperature and RPM inputs are simulated via UART commands (not real measurements)
- No I2C communication involved (simplified testing environment)
- Cannot validate real sensor error handling (I2C failures, timeout, calibration errors)

### Phase 3+: Real Hardware
- LM35 sensor has ±0.5°C accuracy (not industrial-grade ±0.1°C)
- INA219 module shunt resistor (0.1Ω) limits current range to 3.2A (can be modified with shunt resistor change)
- Tachometer feedback is optional (not critical for operation, but improves diagnostics)
- GCOV/LCOV coverage is for host-based core logic only, not full ESP32 firmware
- Project is an educational prototype, not an automotive-grade ECU (no ASIL certification, no functional safety)

---

## 20. Future Improvements

### Phase 1-2 → Phase 3+
- ✅ Add real temperature sensor (LM35)
- ✅ Add real current sensor (INA219)
- ✅ Add real DC fan with MOSFET driver
- ✅ Add tachometer feedback for RPM measurement
- ✅ Implement hardware abstraction layer (HAL)
- ✅ Create real hardware wiring diagram with detailed specs
- ✅ Design I2C error recovery and sensor validation

### Phase 3+ Enhancements
- Upgrade to higher-accuracy temperature sensor (DS18B20 1-Wire, ±0.5°C)
- Add PWM frequency tuning for specific fan models
- Implement adaptive fan control (PID controller instead of step-based)
- Add humidity sensor (DHT22) for combined climate monitoring
- Implement thermal stress testing (run at max temp for extended period)
- Add web dashboard (WebSocket + React) for real-time monitoring
- Implement data logging to SD card (JSON format)
- Add OTA (Over-The-Air) firmware update capability

### Testing & CI/CD Enhancements
- Automated integration tests on GitHub Actions runner (if possible with hardware)
- Code coverage badges in README
- Performance benchmarking (task execution time, UART latency)
- Thermal imaging analysis (validate LM35 readings vs IR camera)
- Long-term stability tests (24+ hour continuous operation)

### Documentation Enhancements
- Add PCB schematic (KiCAD or similar)
- Add PCB layout design
- Add Gerber files for PCB manufacturing
- Add 3D printed enclosure design (STL files)
- Add video walkthrough (wiring, firmware compilation, testing)
- Add troubleshooting guide (common issues and solutions)

---

## 21. Interview Explanation - Complete Project Story

### 30-Second Elevator Pitch

```text
I built an ECU-like fan control system with two deployment phases. Phase 1-2 uses simulation 
(potentiometer, button, LED) to demonstrate RTOS design and test strategy. Phase 3+ integrates 
real sensors (LM35, INA219) and fan control via a MOSFET driver. The system handles fault 
detection, safe mode, and automatic recovery. All cores are tested via a Python test bench 
and supported by a hardware abstraction layer that lets the same firmware work in both modes.
```

### Full Interview Explanation (3-5 Minutes)

```text
I developed this ECU-like fan control system specifically to practice embedded software testing 
for automotive applications. Let me break it down into two phases:

**Phase 1-2: Simulation & Foundation (Learning)**

I started with simulation to focus on software design without hardware complexity. I used a 
potentiometer to simulate temperature input (0-100°C), a button to simulate sensor faults, 
and an LED to simulate the fan output via PWM. This is intentional—I wanted to validate the 
control logic and test strategy before adding real hardware.

The ESP32 runs FreeRTOS with 7 concurrent tasks:
- AnalogInputTask reads the potentiometer every 100ms
- UartCommandTask receives test commands from Raspberry Pi
- FanControlTask calculates fan speed based on temperature (40°C = 40%, 70°C = 70%, 90°C = 100%)
- DiagnosticsTask detects faults: sensor errors (temp < -40°C or > 150°C), over-temp (≥100°C), 
  over-current (>2A), and fan stall (duty > 0 but RPM = 0 for 1 second)
- When a fault is detected, the system enters SAFE_MODE or FAULT_MODE
- After 300ms of stable conditions, the system recovers to NORMAL state

The test bench on Raspberry Pi sends 20+ test cases via UART and validates ESP32 responses. 
This automated approach lets me test boundary conditions, fault injection, and recovery 
scenarios reliably.

**Phase 3+: Real Hardware Integration (Production)**

Once Phase 1-2 was working, I started Phase 3 with real sensors. But here's the key: 
I didn't rewrite the firmware. Instead, I designed a Hardware Abstraction Layer (HAL).

The HAL is simple—a struct with function pointers for read_temperature(), read_current(), 
and read_rpm(). In Phase 1-2, these functions return values from UART commands. In Phase 3+, 
they read from real sensors.

For Phase 3+, I integrated:
1. **LM35 Temperature Sensor** (GPIO34 - ADC)
   - Output: 10mV per °C (linear from -40°C to +150°C)
   - ADC configuration: 12-bit, 3.3V reference, proper attenuation
   - Formula: temp_C = ADC_voltage / 0.01

2. **INA219 Current Sensor** (GPIO21/22 - I2C at address 0x40)
   - Measures DC fan current via internal shunt resistor (0.1Ω)
   - Range: 0 to 3.2A with 0.4mA resolution
   - I implemented error handling for I2C failures (timeout, bus errors)
   - If I2C read fails, it triggers FAULT_SENSOR (same as invalid temperature)

3. **DC Fan with MOSFET Driver** (GPIO26 - PWM)
   - GPIO26 PWM signal controls a logic-level MOSFET gate
   - MOSFET switches 12V/24V power to the fan motor
   - Flywheel diode (1N4007) protects MOSFET from back-EMF

4. **Tachometer Feedback** (GPIO27 - optional)
   - Hall effect sensor pulse input
   - Used for RPM measurement and fan stall detection

The beautiful part: the same firmware binary runs in both Phase 1-2 (simulation) and Phase 3+ 
(real hardware) by just changing a compilation flag. No code duplication, no maintenance nightmare.

**Testing & Quality**

For testing, I followed a comprehensive strategy:
1. **Unit tests** (host-based with Unity framework): Test fan control logic in isolation
2. **Integration tests** (Python + UART): Test full system behavior with simulated/real inputs
3. **Test cases**: Boundary testing (39°C vs 40°C thresholds), fault injection, recovery
4. **Static analysis** (Cppcheck): Detect potential C/C++ issues before runtime
5. **Code coverage** (GCOV/LCOV): Measure test coverage for core modules

The test suite is comprehensive—I test not just happy paths, but edge cases:
- What happens if temperature jumps from 30°C to 100°C instantly?
- What if current sensor fails temporarily (I2C error)?
- What if fan starts spinning but then stalls mid-operation?

**Key Lessons**

1. **Hardware Abstraction Is Critical**: Designing testable code from day one saves months 
   of debugging later. The HAL pattern let me validate Phase 1-2 before touching real hardware.

2. **Fault Recovery Must Be Deterministic**: The 3-cycle (300ms) recovery time isn't arbitrary. 
   It's long enough to distinguish transient noise from real failures, but fast enough to 
   respond to actual issues.

3. **RTOS Task Timing Is Non-Negotiable**: Every task is timed precisely. If DiagnosticsTask 
   misses its 100ms deadline, the system might miss a fan stall. This is where FreeRTOS 
   `vTaskDelayUntil()` matters—it ensures deadline compliance.

4. **Mutex Protection Must Be Everywhere**: Shared state (sensor readings, fan mode) is 
   protected by mutex. Without it, task A could read temperature while task B is updating it, 
   leading to race conditions.

5. **Test Automation Scales**: Running 20 tests manually is error-prone. Writing a Python 
   test bench that runs them all, compares outputs, and generates reports is better. 
   As the system grows, automated tests become mandatory.

**Project Impact**

This project demonstrates:
- Real embedded systems design (RTOS, I2C, ADC, PWM, GPIO)
- Automotive-like thinking (fault detection, safe mode, recovery)
- Clean architecture (HAL, separation of concerns)
- Comprehensive testing (unit, integration, static analysis, coverage)
- Automation (Python test bench, GitHub Actions CI/CD)

It's not a production ECU (no ASIL certification, no redundancy), but it shows the 
mindset and practices needed to build one.
```

---

## 22. Project Status & Roadmap

```text
Project Name:
ECU-Like Fan Control & Diagnostics Test Bench

Current Status:
Phase 1-2 (Simulation): Completed ✅
Phase 3+ (Real Hardware): In Progress (design complete, ready for implementation)

Main Technologies:
ESP32 DevKit V1, Raspberry Pi 4, ESP-IDF, FreeRTOS, C/C++
ADC (LM35), I2C (INA219), GPIO (tachometer, MOSFET)
Python 3, PySerial, Cppcheck, Unity Test Framework, GCOV/LCOV
GitHub Actions CI/CD

Core Competencies Demonstrated:
✅ Embedded systems design (RTOS multi-tasking, real-time constraints)
✅ Hardware sensors integration (ADC, I2C protocols)
✅ Fault detection & recovery (state machines, defensive programming)
✅ Comprehensive testing (unit, integration, boundary, stress testing)
✅ Code quality (static analysis, coverage measurement)
✅ Clean architecture (hardware abstraction layer, separation of concerns)
✅ Automation (Python test bench, CI/CD pipelines)

Next Steps:
→ Implement Phase 3+ real hardware integration (LM35 ADC driver, INA219 I2C driver)
→ Validate Phase 3+ firmware on real hardware
→ Complete test suite for Phase 3+ and validate all test cases pass
→ Document real hardware wiring and specs
→ Prepare demo video and PCB design
```
