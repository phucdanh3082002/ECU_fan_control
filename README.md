# ECU-Like Fan Control & Diagnostics Test Bench

## 1. Executive Summary

Dự án **ECU-Like Fan Control & Diagnostics Test Bench** mô phỏng một hệ thống điều khiển quạt làm mát kiểu ECU sử dụng **ESP32 DevKit V1** làm embedded controller và **Raspberry Pi 4** làm Python-based test bench. Firmware trên ESP32 được phát triển bằng **ESP-IDF + FreeRTOS** trong VS Code, tập trung vào điều khiển fan, diagnostics, safe mode, fault recovery và RTOS task timing.

Dự án hướng đến việc thực hành các kỹ năng phù hợp với vị trí **Embedded Software Testing for Automotive System**, bao gồm: **C/C++ embedded development, FreeRTOS, unit testing, integration testing, test case design, boundary testing, invalid input testing, fault recovery testing, UART communication, Python test automation, static analysis với Cppcheck và chuẩn bị code coverage với GCOV/LCOV**.

Dự án không nhằm tạo ECU automotive-grade thật, mà là một mô hình kỹ thuật có tính thực thi cao để chứng minh năng lực thiết kế, kiểm thử và debug embedded software.

---

## 2. Project Objectives

### Main Objectives

- Develop an ESP32-based ECU-like fan control module.
- Implement FreeRTOS-based firmware architecture.
- Simulate temperature-based fan control using potentiometer input.
- Simulate fan output using PWM-controlled LED.
- Simulate sensor fault using button input.
- Use Raspberry Pi 4 as an automated Python test bench.
- Validate ESP32 behavior through UART test scenarios.
- Design test cases for normal, boundary, invalid input, fault, recovery and RTOS timing scenarios.
- Apply Cppcheck for static analysis.
- Prepare host-based unit testing and GCOV/LCOV code coverage for core logic.

---

## 3. System Architecture

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
|  - AnalogInputTask                                |
|  - ButtonInputTask                                |
|  - UartCommandTask                                |
|  - FanControlTask                                 |
|  - DiagnosticsTask                                |
|  - PwmOutputTask                                  |
|  - StatusReportTask                               |
|                                                   |
|  Hardware Simulation:                             |
|  - Potentiometer: simulated temperature input     |
|  - Button: simulated sensor fault                 |
|  - LED PWM: simulated fan speed output            |
+---------------------------------------------------+
```

---

## 4. Bill of Materials & Tools

| STT | Tên linh kiện/Công cụ | Số lượng | Mục đích/Vai trò |
|---:|---|---:|---|
| 1 | ESP32 DevKit V1 | 1 | Embedded controller, đóng vai ECU-like controller |
| 2 | Raspberry Pi 4 | 1 | Python-based test bench, gửi test scenario và nhận log |
| 3 | Raspberry Pi OS Bookworm | 1 | Hệ điều hành chạy trên Raspberry Pi 4 |
| 4 | USB cable | 1 | UART over USB giữa ESP32 và Raspberry Pi 4, đồng thời dùng để flash firmware |
| 5 | LED | 1-3 | Mô phỏng fan output bằng PWM |
| 6 | Điện trở 220 ohm | 1-3 | Hạn dòng cho LED |
| 7 | Button | 1 | Mô phỏng sensor fault |
| 8 | Potentiometer | 1 | Mô phỏng temperature input dạng analog |
| 9 | Breadboard | 1 | Lắp mạch thử nghiệm |
| 10 | Jumper wires | 1 bộ | Kết nối ESP32 với LED, button, potentiometer |
| 11 | VS Code | 1 | IDE phát triển firmware |
| 12 | ESP-IDF | 1 | Framework chính thức để phát triển ESP32 firmware |
| 13 | FreeRTOS | 1 component | RTOS dùng để chia firmware thành các task |
| 14 | C/C++ | 1 language stack | Viết firmware, control logic và diagnostics logic |
| 15 | Python 3 | 1 language stack | Viết test bench trên Raspberry Pi 4 |
| 16 | PySerial | 1 library | Giao tiếp serial giữa Raspberry Pi 4 và ESP32 |
| 17 | Git | 1 tool | Quản lý source code |
| 18 | GitHub | 1 platform | Lưu source code, tài liệu, test reports |
| 19 | GitHub Actions | 1 CI tool | Tự động hóa build, unit test, static analysis |
| 20 | Cppcheck | 1 tool | Static analysis cho source code C/C++ |
| 21 | Unity Test Framework | 1 framework | Unit testing cho ESP-IDF/component logic |
| 22 | GCOV/LCOV | 1 toolset | Chuẩn bị đo code coverage cho host-based unit tests |

---

## 5. Hardware Design

### 5.1 Hardware Role

| Component | Vai trò |
|---|---|
| ESP32 DevKit V1 | Xử lý logic điều khiển fan, diagnostics, RTOS tasks |
| Raspberry Pi 4 | Chạy Python test bench để kiểm thử ESP32 |
| Potentiometer | Giả lập nhiệt độ đầu vào |
| Button | Giả lập lỗi sensor |
| LED | Giả lập tốc độ quạt bằng PWM |
| USB cable | Giao tiếp UART over USB |

---

## 6. GPIO Mapping

| ESP32 Pin | Kết nối | Vai trò |
|---|---|---|
| GPIO34 | Potentiometer signal | ADC input để mô phỏng temperature |
| GPIO25 | Button | Sensor fault input, dùng internal pull-up |
| GPIO26 | LED qua điện trở 220 ohm | PWM output mô phỏng fan speed |
| USB | Raspberry Pi 4 | UART over USB |

### Potentiometer Wiring

```text
Potentiometer:
- VCC    -> 3.3V ESP32
- GND    -> GND ESP32
- Signal -> GPIO34
```

### Button Wiring

```text
Button:
- Một chân -> GND
- Một chân -> GPIO25
- GPIO25 dùng internal pull-up
```

### LED Wiring

```text
LED:
GPIO26 -> Resistor 220 ohm -> LED anode
LED cathode -> GND
```

---

## 7. Functional Requirements

### 7.1 Fan Control Logic

The system shall calculate fan mode and duty cycle based on temperature input.

| Temperature Range | Fan Mode | Duty Cycle |
|---|---|---:|
| Temperature < 40°C | FAN_OFF | 0% |
| 40°C - 69°C | FAN_LOW | 40% |
| 70°C - 89°C | FAN_MEDIUM | 70% |
| 90°C - 99°C | FAN_HIGH | 100% |
| >= 100°C | FAN_HIGH + OVER_TEMPERATURE | 100% |

---

### 7.2 Sensor Fault Detection

Sensor fault shall be triggered when:

```text
temperature < -40°C
temperature > 150°C
sensorValid = false
```

Expected behavior:

```text
FAULT = SENSOR_FAULT
STATE = SAFE_MODE
FAN = FAN_HIGH
DUTY = 100%
```

---

### 7.3 Over-Temperature Detection

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

### 7.4 Over-Current Detection

Current is simulated by command from Raspberry Pi 4.

Over-current shall be triggered when:

```text
current > 2.0A
```

Expected behavior:

```text
FAULT = OVER_CURRENT
STATE = FAULT_MODE
FAN = FAN_OFF
DUTY = 0%
```

---

### 7.5 Fan Stall Detection

Fan stall is simulated by RPM input from Raspberry Pi 4.

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

### 7.6 Fault Recovery

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

| Task | Chu kỳ | Priority | Vai trò |
|---|---:|---:|---|
| UartCommandTask | Event-driven | 5 | Nhận command từ Raspberry Pi 4 |
| DiagnosticsTask | 100 ms | 4 | Kiểm tra fault conditions |
| FanControlTask | 100 ms | 4 | Tính fan mode và duty cycle |
| AnalogInputTask | 100 ms | 3 | Đọc potentiometer qua ADC |
| ButtonInputTask | 50 ms | 3 | Đọc button sensor fault |
| PwmOutputTask | 100 ms | 2 | Xuất PWM ra LED |
| StatusReportTask | Event-based / 500 ms | 2 | Gửi status về Raspberry Pi 4 |

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
    float temperature;
    float current;
    int rpm;
    bool sensorValid;
    bool useAdcInput;
} SensorInput;

typedef struct {
    FanMode fanMode;
    int dutyCycle;
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
- Setup ESP-IDF toolchain.
- Create ESP-IDF project for ESP32 DevKit V1.
- Install Python 3 and PySerial on Raspberry Pi OS Bookworm.
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

## Phase 2: Basic Hardware Demo

### Tasks

- Connect LED to GPIO26 through 220 ohm resistor.
- Connect button to GPIO25 using internal pull-up.
- Connect potentiometer signal to GPIO34.
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

- Implement `fan_control` module.
- Implement fan mode thresholds.
- Implement duty cycle output.
- Implement `diagnostics` module.
- Implement:
  - sensor fault,
  - over-temperature,
  - over-current,
  - fan stall,
  - safe mode,
  - fault mode,
  - fault recovery after 3 stable cycles.

### Outcome

- Fan control and diagnostics logic work correctly through manual testing.

---

## Phase 4: FreeRTOS Task Architecture

### Tasks

- Create FreeRTOS tasks:
  - AnalogInputTask
  - ButtonInputTask
  - UartCommandTask
  - FanControlTask
  - DiagnosticsTask
  - PwmOutputTask
  - StatusReportTask
- Implement shared system state.
- Protect shared data using mutex.
- Use `vTaskDelayUntil()` for periodic tasks.
- Verify task timing and system stability.

### Outcome

- Firmware has clear RTOS-based architecture.
- Periodic control loop and diagnostics loop are working.

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
- Implement response formatter.
- Validate UART commands manually from Raspberry Pi.

### Outcome

- Raspberry Pi can control test scenarios using UART commands.
- ESP32 returns structured status response.

---

## Phase 6: Python Test Bench

### Tasks

- Implement `serial_client.py`.
- Implement `test_cases.json`.
- Implement `test_runner.py`.
- Implement response parser.
- Compare actual output with expected output.
- Generate CSV test report.

### Outcome

- Raspberry Pi automatically runs integration tests against ESP32.
- Test report is generated.

---

## Phase 7: Test Case Completion

### Tasks

- Add normal test cases.
- Add boundary test cases.
- Add invalid input test cases.
- Add fault test cases.
- Add recovery test cases.
- Add RTOS timing test cases.
- Run full regression test.

### Outcome

- Project has a complete test strategy.
- PASS/FAIL reports are available.

---

## Phase 8: Static Analysis

### Tasks

- Run Cppcheck on ESP32 firmware source code.
- Store report in `static_analysis/cppcheck_report.txt`.
- Review and fix important warnings.
- Document static analysis result in README.

### Outcome

- Static analysis evidence is available.
- Code quality is improved.

---

## Phase 9: Unit Testing & Coverage Preparation

### Tasks

- Setup Unity test framework.
- Separate core logic from hardware-specific code.
- Write unit tests for:
  - fan control thresholds,
  - diagnostics fault detection,
  - recovery logic.
- Prepare host-based test build.
- Prepare GCOV/LCOV coverage flow for host-based unit tests.

### Outcome

- Core logic can be tested without flashing ESP32.
- Coverage measurement is prepared.

---

## Phase 10: GitHub Actions CI

### Tasks

- Create `.github/workflows/ci.yml`.
- Automate:
  - host-based unit test build,
  - unit test execution,
  - Cppcheck static analysis.
- Store logs/reports in repository.

### Outcome

- Repository has basic CI workflow.
- Project demonstrates automated testing workflow.

---

## Phase 11: Documentation

### Tasks

Complete the following documentation:

```text
README.md
docs/system_architecture.md
docs/hardware_wiring.md
docs/rtos_design.md
docs/uart_protocol.md
docs/test_plan.md
docs/test_report_sample.md
```

README should include:

```text
- Overview
- Objectives
- Hardware
- Wiring
- System architecture
- RTOS task design
- UART protocol
- Test strategy
- Test cases
- Sample test report
- Static analysis
- Known limitations
- Future improvements
```

### Outcome

- GitHub repo is clear and professional.
- Project is ready to be shown to recruiters/interviewers.

---

## Phase 12: Final Review & CV Integration

### Tasks

- Run final integration test.
- Run static analysis.
- Update README.
- Add sample screenshots or demo images if available.
- Update CV project description.

### Suggested CV Description

```text
ECU-Like Fan Control & Diagnostics Test Bench
In Progress | ESP32 DevKit V1, Raspberry Pi 4, ESP-IDF, FreeRTOS, C/C++, Python, Unit/Integration Testing

- Developing a FreeRTOS-based ESP32 control module for fan control, diagnostics, and safe mode handling using LED, button, and potentiometer-based hardware simulation.
- Using Raspberry Pi 4 as a Python test bench to send UART test scenarios, collect system logs, and validate expected outputs.
- Designing test cases for normal, boundary, invalid input, over-temperature, over-current, fan stall, fault recovery, and RTOS timing scenarios.
- Applying Cppcheck for static analysis and preparing Unity/GCOV/LCOV-based testing for host-based code coverage measurement.
```

### Outcome

- Project is ready for CV, GitHub and interview discussion.
- Candidate can explain system requirements, RTOS design, test strategy, diagnostics and debugging workflow.

---

## 19. Known Limitations

- Current hardware does not use real automotive sensors.
- Fan is simulated by LED PWM in the initial version.
- Current and RPM are simulated through UART commands.
- GCOV/LCOV coverage is planned for host-based core logic only, not full ESP32 firmware.
- Project is an educational ECU-like prototype, not an automotive-grade ECU.

---

## 20. Future Improvements

- Add real temperature sensor such as DS18B20 or DHT11/DHT22.
- Add real DC fan with MOSFET/transistor driver.
- Add tachometer feedback if using a fan that supports RPM output.
- Add current sensor for real over-current detection.
- Generate HTML test report.
- Add full GitHub Actions workflow.
- Add code coverage badge.
- Add demo video and wiring diagram to README.

---

## 21. Interview Explanation

A concise explanation for interview:

```text
I built this project to practice embedded software testing concepts for an ECU-like control system. ESP32 DevKit V1 is used as the embedded controller running ESP-IDF and FreeRTOS, while Raspberry Pi 4 acts as a Python-based test bench. The ESP32 handles fan control, diagnostics, safe mode, and fault recovery. The Raspberry Pi sends test scenarios over UART, collects status logs, compares actual outputs with expected results, and generates test reports. This project helps me practice unit testing, integration testing, boundary testing, invalid input testing, fault recovery testing, RTOS task timing, static analysis, and host-based coverage preparation.
```

---

## 22. Final Project Status

```text
Project Name:
ECU-Like Fan Control & Diagnostics Test Bench

Status:
In Progress

Main Technologies:
ESP32 DevKit V1, Raspberry Pi 4, ESP-IDF, FreeRTOS, C/C++, Python, UART, Cppcheck, Unity, GCOV/LCOV, GitHub Actions

Core Focus:
Embedded software testing, ECU-like diagnostics, RTOS-based firmware, Python test automation
```
