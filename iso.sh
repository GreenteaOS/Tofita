#!/bin/bash

# Create floppy image

# Create disk image
xorriso -as mkisofs -o /mnt/r/tofita/greenteaos-uefi.iso -iso-level 3 -V UEFI /mnt/r/tofita/iso \
	-append_partition 2 0xef /mnt/r/tofita/disk.img \
	-e --interval:appended_partition_2:all:: -no-emul-boot

echo [Build .iso complete]
exit
