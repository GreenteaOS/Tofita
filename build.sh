# Configure

# sudo apt-get install --reinstall make
# sudo apt install xorriso
# sudo apt-get install clang-6.0

# /mnt/r/ is a temp folder

# Clean

rm -rf /mnt/r/tofita/
mkdir -p /mnt/r/tofita/iso/EFI/BOOT/

# Bootloader

clang-6.0 -O0 -xc -std=gnu11 -fno-stack-protector -fshort-wchar -w -mno-red-zone -Wall -Wextra \
	-Wimplicit-function-declaration -Werror \
	-DGNU_EFI_USE_MS_ABI -DGNU_EFI_USE_EXTERNAL_STDARG -fPIC \
	-Iexternal/inc -Iexternal/inc/x86_64 -Iexternal/inc/protocol -DEFI_FUNCTION_WRAPPER \
	-c -o /mnt/r/tofita/boot.o public-prototype/boot/uefi/boot.c
# Link

ld -nostdlib -znocombreloc -T external/gnuefi/elf_x86_64_efi.lds -shared -Bsymbolic -L external/gnuefi \
	-L /usr/lib \
	external/gnuefi/crt0-efi-x86_64.o \
	-o /mnt/r/loader.so -lefi -lgnuefi
	/mnt/r/tofita/boot.o \
	-o /mnt/r/tofita/loader.so -lefi -lgnuefi

objcopy \
	-j .text -j .sdata -j .data -j .dynamic \
	-j .dynsym  -j .rel -j .rela -j .reloc \
	--target=efi-app-x86_64 /mnt/r/tofita/loader.so /mnt/r/tofita/loader.efi

# Create floppy image
dd if=/dev/zero of=/mnt/r/tofita/disk.img bs=1k count=2880
mformat -i /mnt/r/tofita/disk.img -f 2880 ::
mmd -i /mnt/r/tofita/disk.img ::/EFI
mmd -i /mnt/r/tofita/disk.img ::/EFI/BOOT
mcopy -i /mnt/r/tofita/disk.img /mnt/r/tofita/loader.efi ::/EFI/BOOT/BOOTx64.EFI

# Create disk image
cp -f /mnt/r/tofita/loader.efi /mnt/r/tofita/iso/EFI/BOOT/BOOTX64.EFI
xorriso -as mkisofs -o /mnt/r/tofita/uefi.iso -iso-level 3 -V UEFI /mnt/r/tofita/iso /mnt/r/tofita/disk.img -e /disk.img -no-emul-boot

echo [Build complete]
