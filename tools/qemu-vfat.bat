@echo off
color 0A

:: You must set proper paths to run this

set DRIVE=C
set QEMU="C:\Program Files\qemu\qemu-system-x86_64.exe"
set OVMF="..\Teapot\OVMF.fd"

%QEMU% -accel tcg -bios %OVMF% -m 2048 ^
    -vga std -machine pc-q35-2.10 ^
    -serial file:%DRIVE%:\Tea\qemu.log -cpu max,x2apic=off ^
    -smp 4 ^
    -cdrom %DRIVE%:\Tea\greenteaos-uefi64.iso ^
    -name "Greentea QEMU" -monitor stdio
::    -drive format=raw,file=fat:rw:%DRIVE%:\Tea\spin-off ^
::pause

:: Possible settings
:: -accel tcg
:: -accel whpx
:: -accel whpx,kernel-irqchip=off
