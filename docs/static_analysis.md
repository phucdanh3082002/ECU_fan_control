# Static Analysis

This document records the Phase 8 Cppcheck setup and result.

## Tool

Cppcheck was installed with `winget`:

```powershell
winget install --id Cppcheck.Cppcheck -e --accept-source-agreements --accept-package-agreements --silent
```

Installed binary:

```text
C:\Program Files\Cppcheck\cppcheck.exe
```

Verified version:

```text
Cppcheck 2.20.0
```

## Scope

The analysis targets application firmware under `esp32_firmware/main`. The command uses `esp32_firmware/build/compile_commands.json` from the ESP-IDF build and filters to `*/main/*.c`.

ESP-IDF framework headers are treated as external dependencies. Findings inside `C:\esp\v6.0\esp-idf\*` are excluded from the project report because this phase is intended to check project-owned application code.

## Command

Run from the repository root:

```powershell
& "C:\Program Files\Cppcheck\cppcheck.exe" --enable=all --inconclusive --check-level=exhaustive --force --inline-suppr --error-exitcode=1 --suppress=missingInclude --suppress=missingIncludeSystem --suppress=checkersReport --suppress="*:C:\esp\v6.0\esp-idf\*" --suppress="unusedFunction:esp32_firmware\main\main.c" --template=gcc --quiet --project="esp32_firmware/build/compile_commands.json" --file-filter="*/main/*.c" --output-file="static_analysis/cppcheck_report.txt"
```

## Suppressions

| Suppression | Reason |
|---|---|
| `missingInclude` | ESP-IDF/conditional include noise when Cppcheck expands framework headers |
| `missingIncludeSystem` | Standard/system header availability noise, not application logic |
| `checkersReport` | Cppcheck metadata only |
| `*:C:\esp\v6.0\esp-idf\*` | Ignore third-party framework header findings |
| `unusedFunction:esp32_firmware\main\main.c` | `app_main` is invoked by ESP-IDF startup code outside the application source graph |

## Result

Latest run:

```text
EXIT=0
```

Because `--error-exitcode=1` was enabled, exit code `0` means there are no unsuppressed findings in the focused application-firmware analysis.

No critical warnings remain for project-owned firmware code:

- No null-pointer findings.
- No memory-leak findings.
- No invalid free findings.
- No uninitialized application data findings.

The generated local report is:

```text
static_analysis/cppcheck_report.txt
```
