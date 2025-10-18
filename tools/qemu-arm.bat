@echo off
cd /d %~dp0
color 0A
setlocal enableDelayedExpansion
cls

:: Try R: first and then C:

set DRIVE=R
set WAY=\Tea\greenteaos-uefi64.iso
set ISO=%DRIVE%:%WAY%

if not exist "%ISO%" (
	echo Trying alternative path...
	set DRIVE=C
	set ISO=!DRIVE!:!WAY!
	echo !ISO!
)

set QEMU="C:\Program Files\qemu\qemu-system-aarch64.exe"
set OVMF="C:\Program Files\qemu\share\edk2-aarch64-code.fd"

%QEMU% -accel tcg -bios %OVMF% -m 2048 ^
    -device ramfb -machine virt ^
    -cpu cortex-a72 ^
    -serial file:!DRIVE!:\Tea\qemu-aarch64.log ^
    -smp 8 ^
    -cdrom !ISO! ^
    -name "Greentea QEMU" -monitor stdio
::    -drive format=raw,file=fat:rw:%DRIVE%:\Tea\spin-off ^
::pause
