#!/bin/sh
# chmod +x ./tools/qemu-vfat.sh
# bash ./tools/qemu-vfat.sh
qemu-system-x86_64 \
    -accel tcg \
    -bios ../Teapot/OVMF.fd \
    -m 2048 \
    -vga std \
    -machine pc-q35-2.10 \
    -serial stdio \
    -cpu max -smp 2 \
    -drive format=raw,file=fat:rw:/tmp/tofita/iso \
    -name "Greentea OS"
