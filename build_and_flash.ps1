& "C:\Espressif\tools\Microsoft.v6.0.PowerShell_profile.ps1"
Set-Location "C:\Users\danhs\Downloads\ECU_fan_control\esp32_firmware"
idf.py build
if (Test-Path "build\ecu_fan_control.bin") {
    Write-Host "BUILD SUCCESS"
    idf.py -p COM8 flash
    Write-Host "FLASH SUCCESS"
} else {
    Write-Host "BUILD FAILED"
}
