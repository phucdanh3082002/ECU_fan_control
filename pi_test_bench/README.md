# Python Test Bench

Phase 6 provides the Python UART integration test bench. Phase 7 expands it into a full simulation-mode regression suite for the ESP32 firmware. It sends commands over USB serial, parses protocol responses, validates expected fields, checks selected timing limits, and writes a CSV report.

## Install

```bash
python -m pip install -r pi_test_bench/requirements.txt
```

On Raspberry Pi, use `python3` if that is the configured Python executable.

## Run

Windows, using the current ESP32 port:

```bash
python pi_test_bench/test_runner.py --port COM8
```

Raspberry Pi examples:

```bash
python3 pi_test_bench/test_runner.py --port /dev/ttyUSB0
python3 pi_test_bench/test_runner.py --port /dev/ttyACM0
```

Close `idf.py monitor`, Hercules, or any other serial terminal first. Only one process can use the ESP32 serial port at a time.

## Files

| File | Purpose |
|---|---|
| `serial_client.py` | Serial transport and response parser |
| `test_cases.json` | Test plan, setup commands, cleanup commands, expected fields, timing limits |
| `test_runner.py` | CLI runner |
| `report_generator.py` | CSV report writer |
| `reports/` | Generated CSV reports |

## Test Coverage

The default Phase 7 plan checks:

- Normal operation: `TC_001` - `TC_004`.
- Boundary behavior: `TC_005` - `TC_012`.
- Invalid input: `TC_013` - `TC_015`.
- Fault behavior: `TC_016` - `TC_018`.
- Recovery behavior: `TC_019` - `TC_020`.
- RTOS timing behavior: `TC_RTOS_001` - `TC_RTOS_004`.

Timing-aware steps use `max_elapsed_ms` or `min_elapsed_ms`. The generated CSV includes an `elapsed_ms` column for every step.

## Verified Result

Latest local run against ESP32 on `COM8`:

```text
Summary: 34/34 steps passed
```

The runner writes timestamped CSV files to `pi_test_bench/reports/`.
