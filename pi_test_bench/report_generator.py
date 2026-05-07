from __future__ import annotations

import csv
from pathlib import Path
from typing import Any


FIELDNAMES = [
    "test_id",
    "description",
    "step_index",
    "command",
    "expected",
    "actual",
    "elapsed_ms",
    "passed",
    "message",
]


def write_csv_report(report_path: Path, results: list[dict[str, Any]]) -> None:
    report_path.parent.mkdir(parents=True, exist_ok=True)

    with report_path.open("w", newline="", encoding="utf-8") as csv_file:
        writer = csv.DictWriter(csv_file, fieldnames=FIELDNAMES)
        writer.writeheader()
        for result in results:
            writer.writerow({key: result.get(key, "") for key in FIELDNAMES})


def summarize_results(results: list[dict[str, Any]]) -> tuple[int, int]:
    total = len(results)
    passed = sum(1 for result in results if result.get("passed") is True)
    return passed, total
