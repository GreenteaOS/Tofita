@echo off
color 0A

:: You must fix R:\ paths and copy OVMF.fd to QEMU folder to run this

set QEMU="C:\Program Files\qemu\"
cd /d %QEMU%

:: -accel tcg
:: -accel whpx

qemu-system-x86_64.exe -accel tcg -bios OVMF.fd -m 2048 -vga std -machine pc-q35-2.10 -serial file:R:\tcg.txt -cpu max,x2apic=off -smp 2 -drive format=raw,file=fat:rw:R:\tofita\iso -name "Greentea OS" -monitor stdio

::pause
