#!/bin/bash

# Create floppy image
dd if=/dev/zero of=/mnt/r/tofita/disk.img bs=1k count=6272
mkfs.vfat /mnt/r/tofita/disk.img
../Teapot/mtools/mmd.exe -i R:\\tofita\\disk.img ::/EFI
../Teapot/mtools/mmd.exe -i R:\\tofita\\disk.img ::/EFI/BOOT
../Teapot/mtools/mcopy.exe -i R:\\tofita\\disk.img R:\\tofita\\loader.efi ::/EFI/BOOT/BOOTx64.EFI
../Teapot/mtools/mcopy.exe -i R:\\tofita\\disk.img R:\\tofita\\iso\\TOFITA.DAT ::/TOFITA.DAT

# Create disk image
xorriso -as mkisofs -o /mnt/r/tofita/greenteaos-uefi.iso -iso-level 3 -V UEFI /mnt/r/tofita/iso \
	-append_partition 2 0xef /mnt/r/tofita/disk.img \
	-e --interval:appended_partition_2:all:: -no-emul-boot

echo [Build .iso complete]
exit
