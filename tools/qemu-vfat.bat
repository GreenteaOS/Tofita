@echo off
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

set QEMU="C:\Program Files\qemu\qemu-system-x86_64.exe"
set OVMF="..\Teapot\OVMF.fd"

%QEMU% -accel tcg -bios %OVMF% -m 2048 ^
    -vga std -machine pc-q35-2.10 ^
    -serial file:!DRIVE!:\Tea\qemu.log -cpu max,x2apic=off ^
    -smp 4 ^
    -cdrom !ISO! ^
    -name "Greentea QEMU" -monitor stdio
::    -drive format=raw,file=fat:rw:%DRIVE%:\Tea\spin-off ^
::pause

:: Possible settings
:: -accel tcg
:: -accel whpx
:: -accel whpx,kernel-irqchip=off
