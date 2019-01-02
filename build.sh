# Configure

# sudo apt-get install --reinstall make
# sudo apt install xorriso
# sudo apt-get install clang-6.0

# /mnt/r/ temp folder

# Clean

rm -f /mnt/r/boot.o
rm -f /mnt/r/loader.efi
rm -f /mnt/r/loader.so

# Bootloader

clang-6.0 -O0 -xc -std=gnu11 -fno-stack-protector -fshort-wchar -w -mno-red-zone -Wall -Wextra \
	-Wimplicit-function-declaration -Werror \
	-DGNU_EFI_USE_MS_ABI -DGNU_EFI_USE_EXTERNAL_STDARG -fPIC \
	-Iexternal/inc -Iexternal/inc/x86_64 -Iexternal/inc/protocol -DEFI_FUNCTION_WRAPPER \
	-c -o /mnt/r/boot.o public-prototype/kernel/boot/boot.c
# Link

ld -nostdlib -znocombreloc -T external/gnuefi/elf_x86_64_efi.lds -shared -Bsymbolic -L external/gnuefi \
	-L /usr/lib \
	external/gnuefi/crt0-efi-x86_64.o \
	/mnt/r/boot.o \
	-o /mnt/r/loader.so -lefi -lgnuefi

objcopy \
	-j .text -j .sdata -j .data -j .dynamic \
	-j .dynsym  -j .rel -j .rela -j .reloc \
	--target=efi-app-x86_64 /mnt/r/loader.so /mnt/r/loader.efi

# Create floppy image
dd if=/dev/zero of=/mnt/r/disk.img bs=1k count=2880
mformat -i /mnt/r/disk.img -f 2880 ::
mmd -i /mnt/r/disk.img ::/EFI
mmd -i /mnt/r/disk.img ::/EFI/BOOT
mcopy -i /mnt/r/disk.img /mnt/r/loader.efi ::/EFI/BOOT/BOOTx64.EFI

# Create disk image
mkdir -p /mnt/r/iso/EFI/BOOT/
rm -f /mnt/r/uefi.iso
rm -f /mnt/r/iso/EFI/BOOT/BOOTX64.EFI
cp -f /mnt/r/loader.efi /mnt/r/iso/EFI/BOOT/BOOTX64.EFI
xorriso -as mkisofs -o /mnt/r/uefi.iso -iso-level 3 -V UEFI /mnt/r/iso /mnt/r/disk.img -e /disk.img -no-emul-boot

echo [Build complete]
