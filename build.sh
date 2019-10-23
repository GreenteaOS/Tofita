#!/bin/bash

clang-6.0 -O0 -xc -std=gnu11 -fno-stack-protector -fshort-wchar -w -mno-red-zone -Wall -Wextra \
	-Wimplicit-function-declaration -Werror \
	-DGNU_EFI_USE_MS_ABI -DGNU_EFI_USE_EXTERNAL_STDARG -fPIC \
	-Iexternal/inc -Iexternal/inc/x86_64 -Iexternal/inc/protocol -DEFI_FUNCTION_WRAPPER \
	-c -o /mnt/r/tofita/boot.o boot/uefi/boot.c

clang-6.0 -O0 -xc -std=gnu11 -fno-stack-protector -fshort-wchar -w -mno-red-zone -Wall -Wextra \
	-Wimplicit-function-declaration -Werror \
	-DGNU_EFI_USE_MS_ABI -DGNU_EFI_USE_EXTERNAL_STDARG -fPIC -mcmodel=large \
	-Iexternal/inc -Iexternal/inc/x86_64 -Iexternal/inc/protocol -DEFI_FUNCTION_WRAPPER \
	-c -o /mnt/r/tofita/loader.o boot/loader/loader.c

as -o /mnt/r/tofita/cpu.s.o devices/cpu/cpu.s

clang-6.0 -O2 -xc -std=gnu11 -fno-stack-protector -fshort-wchar -w -mno-red-zone -Wall -Wextra \
	-Wimplicit-function-declaration -Werror \
	-DTOFITA -mtune=nocona \
	-DGNU_EFI_USE_MS_ABI -DGNU_EFI_USE_EXTERNAL_STDARG -fPIC \
	-Iexternal/inc -Iexternal/inc/x86_64 -Iexternal/inc/protocol -DEFI_FUNCTION_WRAPPER \
	-c -o /mnt/r/tofita/tofita.o kernel/tofita.c

ld -T kernel/kernel.ld -o /mnt/r/tofita/tofita_kernel.elf.img \
	/mnt/r/tofita/tofita.asm.o \
	/mnt/r/tofita/cpu.s.o \
	/mnt/r/tofita/tofita.o

../Teapot/LLVM-9.0.0/bin/llvm-objcopy.exe -O binary r:/tofita/tofita_kernel.elf.img r:/tofita/tofita.img
cwd=$(pwd)
cd /mnt/r/tofita/
$cwd/../Teapot/LLVM-9.0.0/bin/llvm-objcopy.exe -I binary -O elf64-x86-64 -B i386 tofita.img tofitaimg.o
cd - > /dev/null

ld -T boot/loader/loader.ld -o /mnt/r/tofita/loader_kernel.elf.img /mnt/r/tofita/loader.s.o /mnt/r/tofita/loader.o /mnt/r/tofita/tofitaimg.o
../Teapot/LLVM-9.0.0/bin/llvm-objcopy.exe -O binary r:/tofita/loader_kernel.elf.img r:/tofita/loader_kernel.img

ld -nostdlib -znocombreloc -T external/gnuefi/elf_x86_64_efi.lds -shared -Bsymbolic -L external/gnuefi \
	-L /usr/lib \
	external/gnuefi/crt0-efi-x86_64.o \
	/mnt/r/tofita/boot.o \
	-o /mnt/r/tofita/loader.so -lgnuefi

../Teapot/x86_64-w64-mingw32/objcopy.exe \
	-j .text -j .sdata -j .data -j .dynamic \
	-j .dynsym  -j .rel -j .rela -j .reloc \
	--target=efi-app-x86_64 R:/tofita/loader.so R:/tofita/loader.efi
