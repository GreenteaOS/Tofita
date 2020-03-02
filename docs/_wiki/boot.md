---
layout: wiki
title: Boot sequence
---

UEFI firmware sets CPU into 64-bit paged mode and enters `EFI\BOOT\BOOTX64.EFI`
from boot storage (which is `.iso`, bootable device or FAT32 EFI-bootable GPT partition).

:car:
