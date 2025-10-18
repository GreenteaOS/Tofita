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

set QEMU="C:\Program Files\qemu\qemu-system-riscv64.exe"
set OVMF="C:\Program Files\qemu\share\opensbi-riscv64-generic-fw_dynamic.bin"
set DEVICETREE="C:\Program Files\qemu\share\petalogix-s3adsp1800.dtb"

%QEMU% -accel tcg -bios %OVMF% -m 2048 ^
    -device virtio-gpu-device -machine virt ^
    -cpu sifive-u54 ^
    -serial file:!DRIVE!:\Tea\qemu-riscv64.log ^
    -smp 1 ^
    -cdrom !ISO! ^
    -dtb %DEVICETREE% ^
    -name "Greentea QEMU" -monitor stdio
::    -drive format=raw,file=fat:rw:%DRIVE%:\Tea\spin-off ^
::
pause

:: -cpu sifive-u54

::qemu-system-riscv64 ^
::  -machine virt ^
::  -m 1024 ^
::  -bios none ^
::  -drive if=pflash,format=raw,readonly=on,file=edk2-riscv64-code.fd ^
::  -drive if=pflash,format=raw,file=edk2-riscv64-vars.fd ^
::  -drive file=fat:rw:ESP,format=raw ^
::  -serial stdio
