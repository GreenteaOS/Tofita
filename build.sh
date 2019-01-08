# Configure

# sudo apt-get install --reinstall make
# sudo apt install xorriso
# sudo apt-get install clang-6.0

# /mnt/r/ is a temp folder

# Clean

rm -rf /mnt/r/tofita/
mkdir -p /mnt/r/tofita/iso/EFI/BOOT/

# Bootloader
as -o /mnt/r/tofita/loader.s.o public-prototype/boot/loader/loader.s

clang-6.0 -O0 -xc -std=gnu11 -fno-stack-protector -fshort-wchar -w -mno-red-zone -Wall -Wextra \
	-Wimplicit-function-declaration -Werror \
	-DGNU_EFI_USE_MS_ABI -DGNU_EFI_USE_EXTERNAL_STDARG -fPIC \
	-Iexternal/inc -Iexternal/inc/x86_64 -Iexternal/inc/protocol -DEFI_FUNCTION_WRAPPER \
	-c -o /mnt/r/tofita/boot.o public-prototype/boot/uefi/boot.c
clang-6.0 -O0 -xc -std=gnu11 -fno-stack-protector -fshort-wchar -w -mno-red-zone -Wall -Wextra \
	-Wimplicit-function-declaration -Werror \
	-DGNU_EFI_USE_MS_ABI -DGNU_EFI_USE_EXTERNAL_STDARG -fPIC -mcmodel=large \
	-Iexternal/inc -Iexternal/inc/x86_64 -Iexternal/inc/protocol -DEFI_FUNCTION_WRAPPER \
	-c -o /mnt/r/tofita/loader.o public-prototype/boot/loader/loader.c

# Kernel
as -o /mnt/r/tofita/tofita.s.o public-prototype/kernel/tofita.s
as -o /mnt/r/tofita/cpu.o public-prototype/devices/cpu/cpu.s

clang-6.0 -O0 -xc -std=gnu11 -fno-stack-protector -fshort-wchar -w -mno-red-zone -Wall -Wextra \
	-Wimplicit-function-declaration -Werror \
	-DGNU_EFI_USE_MS_ABI -DGNU_EFI_USE_EXTERNAL_STDARG -fPIC \
	-Iexternal/inc -Iexternal/inc/x86_64 -Iexternal/inc/protocol -DEFI_FUNCTION_WRAPPER \
	-c -o /mnt/r/tofita/tofita.o public-prototype/kernel/tofita.c

# Link
ld -T public-prototype/kernel/kernel.ld -o /mnt/r/tofita/tofita_kernel.elf.img /mnt/r/tofita/tofita.s.o /mnt/r/tofita/tofita.o /mnt/r/tofita/cpu.o
objcopy -O binary /mnt/r/tofita/tofita_kernel.elf.img /mnt/r/tofita/tofita.img
objcopy -I binary -O elf64-x86-64 -B i386 /mnt/r/tofita/tofita.img /mnt/r/tofita/tofitaimg.o

ld -T public-prototype/boot/loader/loader.ld -o /mnt/r/tofita/loader_kernel.elf.img /mnt/r/tofita/loader.s.o /mnt/r/tofita/loader.o /mnt/r/tofita/tofitaimg.o
objcopy -O binary /mnt/r/tofita/loader_kernel.elf.img /mnt/r/tofita/loader_kernel.img
objcopy -I binary -O elf64-x86-64 -B i386 /mnt/r/tofita/loader_kernel.img /mnt/r/tofita/loader_kernelimg.o

ld -nostdlib -znocombreloc -T external/gnuefi/elf_x86_64_efi.lds -shared -Bsymbolic -L external/gnuefi \
	-L /usr/lib \
	external/gnuefi/crt0-efi-x86_64.o \
	/mnt/r/tofita/boot.o \
	/mnt/r/tofita/loader_kernelimg.o \
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
