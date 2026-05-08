---
name: hal-switch-simulation-to-real
description: "Automate Hardware Abstraction Layer (HAL) switching between simulation (potentiometer/button/LED) and real hardware (LM35/INA219/Fan). Updates include files, rebuild system, and maintains documentation consistency."
license: MIT
compatibility: ESP-IDF 5.x+, CMake, C99+
metadata:
  audience: "firmware developers testing across phases"
  workflow_type: "configuration and environment setup"
  target_phase: "Phase 1-3+ (all phases)"
  hal_targets: "simulation, real-hardware"
  file_updates: "CMakeLists.txt, app_tasks.c, AGENTS.md"
  gpio_management: "automatic"
---

# HAL Switch: Simulation ↔ Real Hardware Skill

## Overview

This skill automates switching between two Hardware Abstraction Layer (HAL) implementations:

### Simulation HAL (Phase 1-2)
- **Temperature Input**: Potentiometer on GPIO34 (ADC)
- **Control Input**: Button on GPIO4 (TOUCH0)
- **Output**: LED PWM on GPIO26 (DAC_2)
- **Sensor Values**: Controlled via UART commands from test bench
- **Use Case**: Development, testing without hardware, simulation-based debugging

### Real Hardware HAL (Phase 3+)
- **Temperature Input**: LM35 sensor on GPIO34 (ADC) with I2C calibration
- **Current Input**: INA219 current sensor on GPIO21/22 (I2C 0x40)
- **Output**: MOSFET PWM on GPIO26 (DAC_2) driving DC fan
- **Tachometer**: RPM feedback on GPIO27 (TOUCH7, optional)
- **Use Case**: Production testing, real-world behavior validation, integration tests

## Use Cases

- **Phase Transition**: Switch from simulation to real hardware after validation
- **Regression Testing**: Run identical test suite on both HAL implementations
- **Hardware Commissioning**: Activate real hardware features progressively
- **Debugging**: Simulate behavior before deploying to real hardware
- **Cross-Validation**: Compare simulation vs. real measurements

## Trigger Phrases

- "Switch to real hardware"
- "Enable real hardware HAL"
- "Activate simulation mode"
- "Switch to simulation"
- "hal-switch-simulation-to-real"
- "hal-switch to real"
- "Configure for Phase 3+"
- "Use LM35 and INA219 sensors"

## Step-by-Step Workflow

### Prerequisites
- Working directory: project root
- `esp32_firmware/` directory present with existing HAL files
- CMake configuration up-to-date
- Git repository initialized (for backup/rollback)
- AGENTS.md and README.md accessible

### Workflow Steps

#### 1. Validate Current State
- Read current HAL configuration from `esp32_firmware/main/app_tasks.c`
- Detect active HAL mode (simulation vs. real)
- Check if switch is already active (no redundant changes)
- Backup existing config to `.opencode/backups/hal_config_<timestamp>.json`

#### 2. Select Target HAL
- Display current mode and target mode confirmation
- Allow user to abort if wrong target selected
- Provide summary of changes that will occur

#### 3. Update CMakeLists.txt
**Current Example** (Simulation):
```cmake
set(SENSOR_HAL_SOURCE "sensor_input_sim.c")
set(SENSOR_INCLUDES "esp32_firmware/main/hal")
```

**After Switch to Real**:
```cmake
set(SENSOR_HAL_SOURCE "sensor_input_real.c")
set(SENSOR_INCLUDES "esp32_firmware/main/hal")
```

#### 4. Update app_tasks.c Includes
**Current** (Simulation):
```c
// Simulation HAL
#include "hal/sensor_input_sim.c"
#define TEMP_SENSOR_PIN GPIO_NUM_34    // ADC Potentiometer
#define BUTTON_PIN GPIO_NUM_4          // TOUCH0 Button
#define FAN_OUTPUT_PIN GPIO_NUM_26     // DAC_2 / LED PWM
```

**After Switch to Real**:
```c
// Real Hardware HAL
#include "hal/sensor_input_real.c"
#define TEMP_SENSOR_PIN GPIO_NUM_34    // LM35 ADC Input
#define CURRENT_SENSOR_I2C 0x40        // INA219 address
#define I2C_SDA_PIN GPIO_NUM_21        // I2C Data
#define I2C_SCL_PIN GPIO_NUM_22        // I2C Clock
#define FAN_OUTPUT_PIN GPIO_NUM_26     // PWM MOSFET Gate
#define TACHOMETER_PIN GPIO_NUM_27     // TOUCH7 Tachometer
```

#### 5. Update Task Priorities (if needed)
**Simulation Tasks**:
- AnalogInputTask, ButtonInputTask, PwmOutputTask
- **Real Hardware Tasks**:
- SensorReadTask, I2CTask (if I2C is needed), TachometerTask, FanDriverTask

#### 6. Update AGENTS.md
**Simulation Section**:
```markdown
## Phase 1-2: Simulation Mode
- **GPIO34**: Potentiometer (ADC input, simulates temperature)
- **GPIO4**: Button (TOUCH0, simulates control input)
- **GPIO26**: LED PWM (output, simulates fan control)
- **UART**: Commands SET_TEMP, SET_CURRENT, USE_ADC_INPUT
```

**Real Hardware Section**:
```markdown
## Phase 3+: Real Hardware Mode
- **GPIO34**: LM35 (ADC input, ~10mV/°C, -40 to +125°C range)
- **GPIO21/22**: INA219 (I2C 0x40, measures current & bus voltage)
- **GPIO26**: MOSFET (PWM gate drive for DC fan, 0-100% duty)
- **GPIO27**: Tachometer (TOUCH7, RPM feedback, optional)
```

#### 7. Rebuild Configuration
- Execute: `idf.py reconfigure` to apply CMake changes
- Verify `sdkconfig` is updated correctly
- Report any configuration conflicts or warnings

#### 8. Generate Summary Report
- Display file changes: which files modified, line count changes
- List GPIO mappings: before/after comparison
- Task structure: which tasks added/removed
- Git diff: show unified diff of all changes (optional)

### Configuration Options

```bash
# Interactive mode (prompt for confirmation)
hal-switch-simulation-to-real --interactive

# Direct switch without prompts
hal-switch-simulation-to-real --target real --yes

# Switch back to simulation (reverse operation)
hal-switch-simulation-to-real --target simulation --yes

# Show current HAL mode without making changes
hal-switch-simulation-to-real --status

# Generate detailed change report as JSON
hal-switch-simulation-to-real --target real --report-json

# Enable rollback option (create backup and restore point)
hal-switch-simulation-to-real --target real --with-backup
```

## File Changes Reference

### Modified Files

| File | Changes | Impact |
|------|---------|--------|
| `esp32_firmware/CMakeLists.txt` | Update `SENSOR_HAL_SOURCE` variable | Controls which HAL is linked |
| `esp32_firmware/main/app_tasks.c` | Update includes, GPIO defines, I2C init | Switches sensor interface |
| `esp32_firmware/sdkconfig` | Enable/disable I2C, ADC components | Firmware feature flags |
| `AGENTS.md` | Update hardware mapping docs | Keeps documentation synced |
| `README.md` (optional) | Update GPIO wiring section | Reference documentation |

### Preserved Files (NO CHANGES)

- `esp32_firmware/main/fan_control.c` - Logic independent of HAL
- `esp32_firmware/main/diagnostics.c` - Fault detection independent of HAL
- `esp32_firmware/main/uart_protocol.c` - Command protocol independent of HAL
- `esp32_firmware/main/hal/sensor_input.h` - Interface definition (shared)
- Both `sensor_input_sim.c` and `sensor_input_real.c` - Remain as-is, just selected

## Expected Output

### Console Output Example
```
═══════════════════════════════════════════════════════════════
  HAL Switch: Simulation → Real Hardware
═══════════════════════════════════════════════════════════════

🔍 CURRENT STATE
  Active HAL:        Simulation
  GPIO Configuration:
    - Temp Input:    GPIO34 (Potentiometer, ADC)
    - Control Input: GPIO4 (Button, TOUCH0)
    - Output:        GPIO26 (LED PWM)

🎯 TARGET STATE
  Target HAL:        Real Hardware
  GPIO Configuration:
    - Temp Input:    GPIO34 (LM35, ADC)
    - Current Input: GPIO21/22 (INA219, I2C 0x40)
    - Output:        GPIO26 (MOSFET PWM)
    - Tachometer:    GPIO27 (TOUCH7, RPM feedback)

📝 FILES TO BE MODIFIED
  ✓ esp32_firmware/CMakeLists.txt
    └─ Line 25: SENSOR_HAL_SOURCE = "sensor_input_real.c"
    └─ Line 26: ADD I2C_DRIVER component
  
  ✓ esp32_firmware/main/app_tasks.c
    └─ Line 14: #include "hal/sensor_input_real.c" (was sim)
    └─ Line 22: #define CURRENT_SENSOR_I2C 0x40 (NEW)
    └─ Lines 98-120: I2C initialization task (NEW)
  
  ✓ AGENTS.md
    └─ Section 6: Update Hardware Mapping (documentation)

⚙️  APPLYING CHANGES
  [████████████░░░░░] 60% - Updating CMakeLists.txt...
  [██████████████░░░] 85% - Updating app_tasks.c...
  [██████████████████] 100% - Updating AGENTS.md...

🔨 RECONFIGURING ESP-IDF
  idf.py reconfigure...
  
  ✓ Configuration updated successfully
  ✓ I2C driver enabled in sdkconfig
  ✓ No conflicts detected

📊 CHANGE SUMMARY
  Files Modified:    3
  Lines Added:       28
  Lines Removed:     12
  New Tasks:         1 (I2CTask for INA219)
  Removed Tasks:     0
  GPIO Changes:      2 new pins (I2C) + 1 new (Tachometer)

📋 GIT STATUS
  Unstaged Changes:  3 files
  Modified:
    - esp32_firmware/CMakeLists.txt
    - esp32_firmware/main/app_tasks.c
    - AGENTS.md
  
  Backup Created:    .opencode/backups/hal_config_2026-05-08_143022.json

✅ HAL SWITCH COMPLETE
  Ready to: idf.py build && idf.py flash
  
  Next Steps:
  1. Run unit tests: run-unit-tests-with-coverage
  2. Build firmware: esp32-firmware-build
  3. Test on real hardware (Phase 3+)

═══════════════════════════════════════════════════════════════
```

## GPIO Safety & Validation

### Pre-Switch Validation
- ✅ Check GPIO34 is not in use elsewhere (ADC-only pin)
- ✅ Verify GPIO21/22 availability for I2C (when switching to real)
- ✅ Confirm GPIO26 not reserved for other peripherals
- ✅ Validate GPIO27 (optional for tachometer)
- ⚠️ Warn about GPIO6-11 (flash memory, never use)
- ⚠️ Warn about GPIO12-15 during JTAG debugging

### I2C Configuration (Real Hardware Only)
- I2C Bus 0: GPIO21 (SDA), GPIO22 (SCL)
- Default Address: 0x40 (INA219)
- Frequency: 100 kHz (standard mode)
- Pullup Resistors: 4.7kΩ (typically required)

## Integration with Project

### HAL Interface (Unchanged by Switch)
```c
// Both simulation and real HAL implement this interface
typedef struct {
    float (*read_temperature)(void);
    float (*read_current)(void);
    int (*read_rpm)(void);
    bool (*is_sensor_valid)(void);
} SensorHAL;
```

### Task Structure After Switch to Real
```
FreeRTOS Tasks (Phase 3+):
- UartCommandTask (P5)       [unchanged]
- SensorReadTask (P4)         [reads LM35 + INA219]
- I2CTask (P4, NEW)           [handles I2C errors]
- DiagnosticsTask (P4)        [unchanged]
- FanControlTask (P4)         [unchanged]
- TachometerTask (P3, NEW)    [GPIO27 RPM counting]
- FanDriverTask (P2, NEW)     [MOSFET PWM output]
- StatusReportTask (P2)       [unchanged]
```

## Rollback & Recovery

### Automatic Backup
- On every switch, create JSON backup: `.opencode/backups/hal_config_<timestamp>.json`
- Contains: file paths, original content, git commit hash
- Allows one-click rollback if issues detected

### Manual Rollback
```bash
# View available backups
ls -la .opencode/backups/

# Restore from backup (interactive prompt shows diffs)
hal-switch-simulation-to-real --restore .opencode/backups/hal_config_2026-05-08_143022.json

# Or use git to revert changes
git checkout esp32_firmware/CMakeLists.txt esp32_firmware/main/app_tasks.c AGENTS.md
```

## Related Skills

- `esp32-firmware-build` - Compile and flash after HAL switch
- `run-unit-tests-with-coverage` - Test HAL changes with unit tests
- `ina219-sensor-diagnostics` - Validate INA219 after switching to real hardware

## References

- ESP32 GPIO Reference: https://docs.espressif.com/projects/esp32-hal/en/latest/api_reference/gpio.html
- I2C Protocol: https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/peripherals/i2c.html
- LM35 Datasheet: https://www.ti.com/lit/ds/symlink/lm35.pdf
- INA219 Datasheet & I2C Guide: https://www.ti.com/lit/ds/symlink/ina219.pdf
- Project HAL Architecture: `README.md` Section 5 (Hardware Abstraction Layer)
- Phase Documentation: `docs/firmware_setup.md`

## Troubleshooting

### Common Issues

| Issue | Cause | Solution |
|-------|-------|----------|
| "I2C initialization fails after switch" | Missing pullup resistors | Add 4.7kΩ pullups on GPIO21/22 (real hardware) |
| "Build fails: 'sensor_input_real.h' not found" | Incorrect includes path | Check CMakeLists.txt `SENSOR_INCLUDES` variable |
| "GPIO conflict detected" | GPIO already in use | Switch back and check for custom modifications |
| "INA219 not responding at 0x40" | Wrong I2C address or device not connected | Use `i2cdetect -y 0` (Raspberry Pi) or `idf.py monitor` (ESP32) |
| "Tachometer task causes crashes" | GPIO27 conflicts or interrupt priority | Check GPIO usage: verify GPIO27 is free |

### Validation Commands

```bash
# Test I2C communication after switch (on Raspberry Pi connected to ESP32)
i2cdetect -y 0

# Monitor ESP32 output to verify HAL switch
idf.py monitor

# Check GPIO usage in CMakeLists.txt
grep -n "GPIO_NUM" esp32_firmware/CMakeLists.txt

# Verify git diff before rebuilding
git diff esp32_firmware/CMakeLists.txt esp32_firmware/main/app_tasks.c
```

## Future Enhancements

- **Multi-HAL Support**: Support additional HAL implementations (e.g., MockHAL for testing)
- **Hardware Profiling**: Auto-detect available sensors and suggest optimal HAL
- **Gradual Migration**: Activate real hardware features one at a time (e.g., LM35 first, then INA219)
- **HAL Comparison Mode**: Run both HALs in parallel and compare readings

---

**Last Updated**: May 2026
**Tested On**: ESP-IDF 5.0+, Phase 1-2 (simulation) and Phase 3+ (real hardware)
