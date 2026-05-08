---
name: run-unit-tests-with-coverage
description: "Execute host-based unit tests using Unity Framework, generate code coverage reports (GCOV/LCOV), and provide a comprehensive summary of test results and coverage metrics."
license: MIT
compatibility: Python 3.8+, GCC/Clang with GCOV support, Linux/macOS/WSL
metadata:
  audience: "firmware developers testing core components"
  workflow_type: "testing and code coverage"
  target_phase: "Phase 9+ (unit testing)"
  test_framework: "Unity"
  coverage_tools: "GCOV, LCOV"
  output_format: "JSON, CSV, HTML (optional)"
---

# Run Unit Tests with Coverage Skill

## Overview

This skill automates host-based unit testing for the ECU fan control firmware:
1. **Compile Tests** - Build unit tests using GCC/Clang with coverage instrumentation (GCOV)
2. **Execute Tests** - Run all test suites and capture pass/fail results
3. **Generate Coverage** - Create GCOV data and generate HTML/LCOV reports
4. **Report Results** - Display summary with pass rate, coverage %, and line/branch metrics

## Use Cases

- **Continuous Testing**: Validate core modules (fan_control, diagnostics) on every commit
- **Coverage Tracking**: Monitor code coverage trends over time
- **Pre-Flash Verification**: Ensure unit tests pass before flashing ESP32
- **Regression Detection**: Identify broken tests after refactoring
- **Coverage Gaps**: Find untested code paths in critical modules

## Trigger Phrases

- "Run unit tests"
- "Execute tests with coverage"
- "Check test coverage"
- "Run host tests"
- "run-unit-tests-with-coverage"
- "Test and report coverage"

## Step-by-Step Workflow

### Prerequisites
- Python 3.8+ installed
- GCC or Clang with GCOV support
- LCOV installed (for HTML reports) - optional
- Working directory: project root containing `unit_tests/` directory
- CMake or Make for building tests

### Workflow Steps

1. **Verify Dependencies**
   - Check if `gcc` or `clang` is available
   - Verify Python 3.8+ installed
   - Confirm `unit_tests/run_tests.py` exists and is executable
   - Check for LCOV (optional, warn if missing but continue)

2. **Build Unit Tests**
   - Navigate to `unit_tests/` directory
   - Execute: `python3 run_tests.py --build`
   - Compile with GCOV flags: `-fprofile-arcs -ftest-coverage`
   - Parse compiler output for build errors
   - Report test binary paths and sizes

3. **Run Test Suites**
   - Execute: `python3 run_tests.py --execute` (or combined with build)
   - Run individual test suites:
     - `test_main` - Basic system initialization
     - `test_fan_control` - Fan control logic and temperature thresholds
     - `test_diagnostics` - Fault detection and recovery
     - `test_uart_protocol` - Command parsing
     - `test_hal_sim` - Simulation HAL behavior
   - Capture test output with PASS/FAIL/SKIP status
   - Report execution time per test suite

4. **Generate Coverage Data**
   - Run GCOV on compiled objects: `gcov <test_binary>`
   - Collect `.gcov` files for each source file
   - Generate coverage summary (lines covered, branches covered)
   - Create coverage percentage by module

5. **Generate Reports**
   - **Text Summary** (console): Pass/fail counts, coverage %, recommendations
   - **JSON Report** (`coverage/test_results.json`):
     ```json
     {
       "summary": {
         "total_tests": 42,
         "passed": 40,
         "failed": 2,
         "skipped": 0,
         "pass_rate": "95.2%",
         "execution_time_seconds": 2.34
       },
       "coverage": {
         "line_coverage": "87.3%",
         "branch_coverage": "72.1%",
         "modules": {
           "fan_control.c": "92.1%",
           "diagnostics.c": "81.5%",
           "uart_protocol.c": "88.0%"
         }
       }
     }
     ```
   - **HTML Report** (`coverage/index.html`): Interactive coverage visualization (via LCOV)
   - **CSV Report** (`coverage/coverage_summary.csv`): Line-by-line coverage export

### Configuration Options

```bash
# Run full test suite with coverage report
run-unit-tests-with-coverage

# Build only (no execution)
run-unit-tests-with-coverage --build-only

# Run tests without coverage generation
run-unit-tests-with-coverage --no-coverage

# Run specific test file only
run-unit-tests-with-coverage --test fan_control

# Generate HTML report (requires LCOV)
run-unit-tests-with-coverage --html

# Fail if coverage below threshold
run-unit-tests-with-coverage --min-coverage 80
```

## Test Suite Details

### test_fan_control.c
**Purpose**: Validate temperature-to-fan mode conversion logic

**Key Test Cases**:
- ✓ Fan OFF when temp < 40°C
- ✓ Fan LOW when 40°C ≤ temp < 70°C (40% duty)
- ✓ Fan MEDIUM when 70°C ≤ temp < 90°C (70% duty)
- ✓ Fan HIGH when temp ≥ 90°C (100% duty)
- ✓ Hysteresis: prevent oscillation at boundaries
- ✓ Edge cases: min/max temperatures, invalid inputs

### test_diagnostics.c
**Purpose**: Validate fault detection and recovery mechanisms

**Key Test Cases**:
- ✓ Detect sensor faults (timeout, invalid values)
- ✓ Detect thermal runaway (temp spike > 120°C)
- ✓ Detect current overload (I > 3.0A)
- ✓ Enter SAFE_MODE when fault detected
- ✓ Recovery when conditions normalize
- ✓ Persistent faults (stay in FAULT state)

### test_uart_protocol.c
**Purpose**: Validate UART command parsing

**Key Test Cases**:
- ✓ Parse `SET_TEMP:85` command
- ✓ Parse `GET_STATUS` command
- ✓ Parse `CLEAR_FAULT` command
- ✓ Reject malformed commands
- ✓ Handle out-of-range values

### test_hal_sim.c
**Purpose**: Validate simulation HAL

**Key Test Cases**:
- ✓ Read sensor values from global `g_sensorInput`
- ✓ Update values via UART commands
- ✓ Transition between valid/invalid states

## Expected Output

### Console Output Example
```
═══════════════════════════════════════════════════════════════
  ECU Fan Control - Unit Test Report
═══════════════════════════════════════════════════════════════

📊 COMPILATION
  ✓ fan_control.c compiled with GCOV support
  ✓ diagnostics.c compiled with GCOV support
  ✓ uart_protocol.c compiled with GCOV support
  
⚙️  TEST EXECUTION (2.34s)
  ✓ test_fan_control .......... PASS (12 tests)
  ✓ test_diagnostics .......... PASS (18 tests)
  ✓ test_uart_protocol ........ PASS (9 tests)
  ✓ test_hal_sim .............. PASS (3 tests)
  ✗ test_edge_cases ........... FAIL (1 of 4 tests failed)
      └─ TEST: "hysteresis_prevents_oscillation" FAILED
         Expected: 0 state changes in 100ms
         Actual: 2 state changes

📈 COVERAGE RESULTS
  Line Coverage:    87.3% (1,245 / 1,426 lines)
  Branch Coverage:  72.1% (156 / 216 branches)
  
  Module Breakdown:
    fan_control.c        92.1% (142 / 154 lines)
    diagnostics.c        81.5% (109 / 134 lines)
    uart_protocol.c      88.0%  (66 /  75 lines)
    hal/sensor_input.c   74.2%  (35 /  47 lines)  ⚠️ Below 80%
    
📄 REPORT GENERATED
  JSON:  coverage/test_results.json
  HTML:  coverage/index.html (run 'open coverage/index.html')
  CSV:   coverage/coverage_summary.csv

⚠️  RECOMMENDATIONS
  1. Uncovered: sensor_input.c lines 23-28 (I2C error handling)
     → Add test: `test_i2c_bus_error` to sensor_input_real.c
  
  2. Low branch coverage: diagnostics.c (72.1%)
     → Add test: `test_recovery_timeout` to cover timeout paths
     
✅ SUMMARY: 42 / 42 tests PASSED | Coverage: 87.3% | Time: 2.34s
═══════════════════════════════════════════════════════════════
```

## Integration with Project

### Unit Tests Directory Structure
```
unit_tests/
├── run_tests.py               # Main test runner (handles build & execution)
├── CMakeLists.txt (optional)  # CMake build configuration
├── Makefile (optional)        # Alternative build system
├── unity/                     # Unity Framework headers
│   ├── unity.h
│   ├── unity_internals.h
│   └── unity_config.h
├── test_main.c
├── test_fan_control.c
├── test_diagnostics.c
├── test_uart_protocol.c
├── test_hal_sim.c
└── mocks/                     # Mock objects for HAL
    ├── mock_sensor_input.c
    └── mock_sensor_input.h
```

### Coverage Output Structure
```
coverage/
├── test_results.json          # Machine-readable results
├── coverage_summary.csv       # Line-by-line coverage
├── index.html                 # LCOV HTML report
├── gcov_data/                 # Raw GCOV files (.gcda, .gcov)
└── coverage_trends.csv        # Historical coverage tracking
```

## Success Criteria

- [ ] All unit tests compile without errors
- [ ] Test execution completes in < 5 seconds
- [ ] Pass rate ≥ 95% (or user-specified threshold)
- [ ] Line coverage ≥ 80% (or user-specified threshold)
- [ ] Coverage reports generated in `coverage/` directory
- [ ] No memory leaks detected (valgrind/asan, if enabled)

## Related Skills

- `esp32-firmware-build` - Build firmware after tests pass
- `hal-switch-simulation-to-real` - Switch HAL before/after unit tests
- `generate-test-report` - Generate integration test report on Raspberry Pi

## References

- Unity Testing Framework: http://www.throwtheswitch.org/unity
- GCOV User Guide: https://gcc.gnu.org/onlinedocs/gcc/Instrumentation-Options.html
- LCOV Coverage Measurement Tool: http://ltp.sourceforge.net/coverage/lcov.php
- Project Testing Docs: `docs/unit_testing.md`

## Troubleshooting

### Common Issues

| Issue | Solution |
|-------|----------|
| "gcc: command not found" | Install GCC: `sudo apt-get install build-essential` (Linux) or Xcode (macOS) |
| "No GCOV data generated" | Ensure test binary runs: check for crashes or assertions |
| "LCOV not found" | Install LCOV: `sudo apt-get install lcov` (optional for HTML reports) |
| "Test binary fails to execute" | Check mock objects: ensure all external functions are mocked |
| "Coverage below threshold" | Use `--min-coverage <value>` or investigate missing test cases |

### Manual Testing Commands

```bash
# Build tests manually
cd unit_tests
gcc -fprofile-arcs -ftest-coverage test_fan_control.c fan_control.c -o test_fan_control -lunity

# Run test binary directly
./test_fan_control

# Generate coverage manually
gcov test_fan_control.c

# Create HTML report with LCOV
lcov --capture --directory . --output-file app.info
genhtml app.info --output-directory coverage/
```

---

**Last Updated**: May 2026
**Tested On**: GCC 9.x-11.x, Ubuntu 20.04+, macOS 11+, WSL2
