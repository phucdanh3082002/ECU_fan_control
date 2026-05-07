from __future__ import annotations

import argparse
import json
import sys
import time
from datetime import datetime
from pathlib import Path
from typing import Any

from report_generator import summarize_results, write_csv_report
from serial_client import ProtocolResponse, SerialClient


DEFAULT_CASES = Path(__file__).with_name("test_cases.json")
DEFAULT_REPORT_DIR = Path(__file__).with_name("reports")


def load_test_plan(path: Path) -> dict[str, Any]:
    with path.open("r", encoding="utf-8") as file:
        return json.load(file)


def values_match(actual: str | None, expected: Any) -> bool:
    if actual is None:
        return False

    if isinstance(expected, bool):
        return actual in {"1", "true", "TRUE"} if expected else actual in {"0", "false", "FALSE"}

    if isinstance(expected, int):
        try:
            return int(float(actual)) == expected
        except ValueError:
            return False

    if isinstance(expected, float):
        try:
            return abs(float(actual) - expected) < 0.05
        except ValueError:
            return False

    return actual == str(expected)


def compare_response(response: ProtocolResponse, expected: dict[str, Any]) -> tuple[bool, str]:
    mismatches: list[str] = []

    for key, expected_value in expected.items():
        actual_value = response.fields.get(key)
        if not values_match(actual_value, expected_value):
            mismatches.append(f"{key}: expected {expected_value!r}, got {actual_value!r}")

    if mismatches:
        return False, "; ".join(mismatches)

    return True, "OK"


def compare_timing(elapsed_ms: float, step: dict[str, Any]) -> tuple[bool, str]:
    min_elapsed_ms = step.get("min_elapsed_ms")
    max_elapsed_ms = step.get("max_elapsed_ms")

    if min_elapsed_ms is not None and elapsed_ms < float(min_elapsed_ms):
        return False, f"elapsed_ms: expected >= {float(min_elapsed_ms):.1f}, got {elapsed_ms:.1f}"

    if max_elapsed_ms is not None and elapsed_ms > float(max_elapsed_ms):
        return False, f"elapsed_ms: expected <= {float(max_elapsed_ms):.1f}, got {elapsed_ms:.1f}"

    return True, "OK"


def format_expected(step: dict[str, Any]) -> str:
    expected: dict[str, Any] = {"fields": step.get("expect", {})}

    if "min_elapsed_ms" in step:
        expected["min_elapsed_ms"] = step["min_elapsed_ms"]

    if "max_elapsed_ms" in step:
        expected["max_elapsed_ms"] = step["max_elapsed_ms"]

    return json.dumps(expected, sort_keys=True)


def send_setup_commands(client: SerialClient, commands: list[str], timeout_s: float) -> None:
    for command in commands:
        client.send_command(command, timeout_s)


def run_step(
    client: SerialClient,
    test_case: dict[str, Any],
    step: dict[str, Any],
    step_index: int,
    response_timeout_s: float,
) -> dict[str, Any]:
    command = step.get("command", "")
    expected = step.get("expect", {})

    if "wait_ms" in step:
        wait_ms = int(step["wait_ms"])
        time.sleep(wait_ms / 1000.0)
        return {
            "test_id": test_case["id"],
            "description": test_case.get("description", ""),
            "step_index": step_index,
            "command": f"WAIT:{wait_ms}ms",
            "expected": "",
            "actual": "",
            "elapsed_ms": wait_ms,
            "passed": True,
            "message": "OK",
        }

    elapsed_ms = 0.0

    try:
        start = time.monotonic()
        response = client.send_command(command, response_timeout_s)
        elapsed_ms = (time.monotonic() - start) * 1000.0
        fields_passed, fields_message = compare_response(response, expected)
        timing_passed, timing_message = compare_timing(elapsed_ms, step)
        passed = fields_passed and timing_passed
        message = "; ".join(message for message in (fields_message, timing_message) if message != "OK") or "OK"
        actual = response.line
    except Exception as exc:  # noqa: BLE001 - report serial/test failures in CSV
        passed = False
        message = str(exc)
        actual = ""

    return {
        "test_id": test_case["id"],
        "description": test_case.get("description", ""),
        "step_index": step_index,
        "command": command,
        "expected": format_expected(step),
        "actual": actual,
        "elapsed_ms": f"{elapsed_ms:.1f}",
        "passed": passed,
        "message": message,
    }


def run_tests(args: argparse.Namespace) -> int:
    test_plan = load_test_plan(args.cases)
    config = test_plan.get("config", {})
    baud_rate = args.baud_rate or int(config.get("baud_rate", 115200))
    response_timeout_s = float(config.get("response_timeout_s", 5.0))
    startup_delay_s = float(config.get("startup_delay_s", 2.0))

    results: list[dict[str, Any]] = []

    with SerialClient(
        port=args.port,
        baud_rate=baud_rate,
        read_timeout_s=0.25,
        startup_delay_s=startup_delay_s,
    ) as client:
        for test_case in test_plan["test_cases"]:
            if not args.no_setup:
                send_setup_commands(client, test_plan.get("setup_commands", []), response_timeout_s)

            for index, step in enumerate(test_case["steps"], start=1):
                result = run_step(client, test_case, step, index, response_timeout_s)
                results.append(result)
                status = "PASS" if result["passed"] else "FAIL"
                print(f"[{status}] {result['test_id']} step {index}: {result['command']}")
                if not result["passed"]:
                    print(f"       {result['message']}")

        if not args.no_cleanup:
            send_setup_commands(client, test_plan.get("cleanup_commands", []), response_timeout_s)

    report_path = args.report or DEFAULT_REPORT_DIR / f"test_report_{datetime.now():%Y%m%d_%H%M%S}.csv"
    write_csv_report(report_path, results)

    passed, total = summarize_results(results)
    print(f"\nSummary: {passed}/{total} steps passed")
    print(f"Report: {report_path}")

    return 0 if passed == total else 1


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description="Run ECU fan-control UART integration tests")
    parser.add_argument("--port", required=True, help="Serial port, e.g. COM8 or /dev/ttyUSB0")
    parser.add_argument("--baud-rate", type=int, default=None, help="Serial baud rate override")
    parser.add_argument("--cases", type=Path, default=DEFAULT_CASES, help="Path to test_cases.json")
    parser.add_argument("--report", type=Path, default=None, help="Output CSV report path")
    parser.add_argument("--no-setup", action="store_true", help="Skip setup commands before each test")
    parser.add_argument("--no-cleanup", action="store_true", help="Skip cleanup commands after all tests")
    return parser.parse_args()


if __name__ == "__main__":
    sys.exit(run_tests(parse_args()))
