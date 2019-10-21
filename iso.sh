#!/bin/bash

# Create floppy image
dd if=/dev/zero of=/mnt/r/tofita/disk.img bs=1k count=6272
mkfs.vfat /mnt/r/tofita/disk.img
mmd -i /mnt/r/tofita/disk.img ::/EFI
mmd -i /mnt/r/tofita/disk.img ::/EFI/BOOT
mcopy -i /mnt/r/tofita/disk.img /mnt/r/tofita/loader.efi ::/EFI/BOOT/BOOTx64.EFI
mcopy -i /mnt/r/tofita/disk.img /mnt/r/TOFITA.DAT ::/TOFITA.DAT

# Create disk image
cp -f /mnt/r/TOFITA.DAT /mnt/r/tofita/iso/TOFITA.DAT
xorriso -as mkisofs -o /mnt/r/tofita/greenteaos-uefi.iso -iso-level 3 -V UEFI /mnt/r/tofita/iso \
	-append_partition 2 0xef /mnt/r/tofita/disk.img \
	-e --interval:appended_partition_2:all:: -no-emul-boot

echo [Build .iso complete]
exit
