@echo off
set MSYSTEM=
set IDF_PATH=C:\esp\v6.0\esp-idf
set IDF_TOOLS_PATH=C:\Espressif\tools
set IDF_PYTHON_ENV_PATH=C:\Espressif\tools\python\v6.0\venv
set ESP_ROM_ELF_DIR=C:\Espressif\tools\esp-rom-elfs\20241011
set ESP_IDF_VERSION=6.0
set PATH=C:\Espressif\tools\ccache\4.12.1\ccache-4.12.1-windows-x86_64;C:\Espressif\tools\cmake\4.0.3\bin;C:\Espressif\tools\dfu-util\0.11\dfu-util-0.11-win64;C:\Espressif\tools\idf-exe\1.0.3\;C:\Espressif\tools\ninja\1.12.1\;C:\Espressif\tools\xtensa-esp-elf\esp-15.2.0_20251204\xtensa-esp-elf\bin;C:\Espressif\tools\xtensa-esp-elf\esp-15.2.0_20251204\xtensa-esp-elf\xtensa-esp-elf\bin;C:\Espressif\tools\riscv32-esp-elf\esp-15.2.0_20251204\riscv32-esp-elf\bin;C:\Espressif\tools\riscv32-esp-elf\esp-15.2.0_20251204\riscv32-esp-elf\riscv32-esp-elf\bin;C:\Espressif\tools\python\v6.0\venv\Scripts;%PATH%
C:\Espressif\tools\python\v6.0\venv\Scripts\python.exe C:\esp\v6.0\esp-idf\tools\idf.py %*
