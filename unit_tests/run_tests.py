from __future__ import annotations

import shutil
import subprocess
import sys
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]
UNIT_TEST_DIR = ROOT / "unit_tests"
BUILD_DIR = UNIT_TEST_DIR / "build"
COVERAGE_DIR = ROOT / "coverage"
FIRMWARE_DIR = ROOT / "esp32_firmware" / "main"
HAL_DIR = FIRMWARE_DIR / "hal"

CORE_SOURCES = [
    FIRMWARE_DIR / "fan_control.c",
    FIRMWARE_DIR / "diagnostics.c",
    FIRMWARE_DIR / "system_types.c",
    FIRMWARE_DIR / "uart_protocol.c",
    HAL_DIR / "sensor_input_sim.c",
]

TEST_SOURCES = [
    UNIT_TEST_DIR / "unity" / "unity.c",
    UNIT_TEST_DIR / "test_main.c",
    UNIT_TEST_DIR / "test_fan_control.c",
    UNIT_TEST_DIR / "test_diagnostics.c",
    UNIT_TEST_DIR / "test_uart_protocol.c",
    UNIT_TEST_DIR / "test_system_types.c",
    UNIT_TEST_DIR / "test_sensor_input_sim.c",
]

CFLAGS = [
    "-std=c99",
    "-Wall",
    "-Wextra",
    "-O0",
    "-g",
    "-fprofile-arcs",
    "-ftest-coverage",
]


def run(command: list[str], cwd: Path = ROOT) -> None:
    print(" ".join(command))
    subprocess.run(command, cwd=cwd, check=True)


def clean_outputs() -> None:
    if BUILD_DIR.exists():
        shutil.rmtree(BUILD_DIR)
    BUILD_DIR.mkdir(parents=True)
    COVERAGE_DIR.mkdir(parents=True, exist_ok=True)

    for pattern in ("*.gcov", "coverage_summary.txt", "*.info"):
        for path in COVERAGE_DIR.glob(pattern):
            path.unlink()


def compile_sources() -> list[Path]:
    objects: list[Path] = []
    include_flags = ["-I", str(FIRMWARE_DIR), "-I", str(HAL_DIR), "-I", str(UNIT_TEST_DIR / "unity")]

    for source in [*CORE_SOURCES, *TEST_SOURCES]:
        object_path = BUILD_DIR / f"{source.stem}.o"
        run(["gcc", *CFLAGS, *include_flags, "-c", str(source), "-o", str(object_path)])
        objects.append(object_path)

    return objects


def link_test_binary(objects: list[Path]) -> Path:
    executable = BUILD_DIR / ("unit_tests.exe" if sys.platform.startswith("win") else "unit_tests")
    run(["gcc", *[str(obj) for obj in objects], "-fprofile-arcs", "-ftest-coverage", "-o", str(executable)])
    return executable


def run_unit_tests(executable: Path) -> None:
    run([str(executable)])


def run_gcov() -> None:
    for source in CORE_SOURCES:
        run(["gcov", "-b", "-c", "-o", str(BUILD_DIR), str(source)], cwd=COVERAGE_DIR)


def parse_gcov_file(path: Path) -> tuple[int, int]:
    covered = 0
    executable = 0

    with path.open("r", encoding="utf-8", errors="replace") as gcov_file:
        for line in gcov_file:
            parts = line.split(":", 2)
            if len(parts) < 3:
                continue

            count = parts[0].strip()
            if count == "-":
                continue

            executable += 1
            if count not in {"#####", "====="}:
                covered += 1

    return covered, executable


def write_coverage_summary() -> None:
    lines = ["Host unit-test coverage summary", ""]
    total_covered = 0
    total_executable = 0

    for source in CORE_SOURCES:
        gcov_path = COVERAGE_DIR / f"{source.name}.gcov"
        if not gcov_path.exists():
            lines.append(f"{source.name}: missing gcov output")
            continue

        covered, executable = parse_gcov_file(gcov_path)
        total_covered += covered
        total_executable += executable
        percent = (covered / executable * 100.0) if executable else 100.0
        lines.append(f"{source.name}: {covered}/{executable} lines covered ({percent:.1f}%)")

    total_percent = (total_covered / total_executable * 100.0) if total_executable else 100.0
    lines.extend([
        "",
        f"TOTAL: {total_covered}/{total_executable} lines covered ({total_percent:.1f}%)",
        "Coverage target: >80%",
        "Result: PASS" if total_percent >= 80.0 else "Result: FAIL",
        "",
        "Note: LCOV/genhtml can consume the generated gcda/gcno data on hosts where lcov is installed.",
    ])

    summary_path = COVERAGE_DIR / "coverage_summary.txt"
    summary_path.write_text("\n".join(lines) + "\n", encoding="utf-8")
    print(summary_path.read_text(encoding="utf-8"))

    if total_percent < 80.0:
        raise SystemExit(1)


def main() -> int:
    clean_outputs()
    objects = compile_sources()
    executable = link_test_binary(objects)
    run_unit_tests(executable)
    run_gcov()
    write_coverage_summary()
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
