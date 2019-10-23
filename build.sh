#!/bin/bash

../Teapot/LLVM-9.0.0/bin/clang.exe -target x86_64-pc-linux-gnu \
	-O0 -xc -std=gnu11 -fno-stack-protector -fshort-wchar -w -mno-red-zone -Wall -Wextra \
	-Wimplicit-function-declaration -Werror \
	-DGNU_EFI_USE_MS_ABI -DGNU_EFI_USE_EXTERNAL_STDARG -fPIC \
	-Iexternal/inc -Iexternal/inc/x86_64 -Iexternal/inc/protocol -DEFI_FUNCTION_WRAPPER \
	-c -o r:/tofita/boot.o boot/uefi/boot.c

../Teapot/LLVM-9.0.0/bin/clang.exe -target x86_64-pc-linux-gnu \
	-O0 -xc -std=gnu11 -fno-stack-protector -fshort-wchar -w -mno-red-zone -Wall -Wextra \
	-Wimplicit-function-declaration -Werror \
	-DGNU_EFI_USE_MS_ABI -DGNU_EFI_USE_EXTERNAL_STDARG -fPIC -mcmodel=large \
	-Iexternal/inc -Iexternal/inc/x86_64 -Iexternal/inc/protocol -DEFI_FUNCTION_WRAPPER \
	-c -o r:/tofita/loader.o boot/loader/loader.c

../Teapot/LLVM-9.0.0/bin/clang.exe -target x86_64-pc-linux-gnu \
	-O2 -xc -std=gnu11 -fno-stack-protector -fshort-wchar -w -mno-red-zone -Wall -Wextra \
	-Wimplicit-function-declaration -Werror \
	-DTOFITA -mtune=nocona \
	-DGNU_EFI_USE_MS_ABI -DGNU_EFI_USE_EXTERNAL_STDARG -fPIC \
	-Iexternal/inc -Iexternal/inc/x86_64 -Iexternal/inc/protocol -DEFI_FUNCTION_WRAPPER \
	-c -o r:/tofita/tofita.o kernel/tofita.c

../Teapot/linux-ubuntu-natty_x86_64-bin_x86_64-mingw_20101218_vityan/bin/x86_64-vityan-linux-gnu-ld.exe -m elf_x86_64 \
	-T kernel/kernel.ld -o r:/tofita/tofita_kernel.elf.img \
	r:/tofita/tofita.asm.o \
	r:/tofita/cpu.asm.o \
	r:/tofita/tofita.o

../Teapot/LLVM-9.0.0/bin/llvm-objcopy.exe -O binary r:/tofita/tofita_kernel.elf.img r:/tofita/tofita.img
cwd=$(pwd)
cd /mnt/r/tofita/
$cwd/../Teapot/LLVM-9.0.0/bin/llvm-objcopy.exe -I binary -O elf64-x86-64 -B i386 tofita.img tofitaimg.o
cd - > /dev/null

../Teapot/linux-ubuntu-natty_x86_64-bin_x86_64-mingw_20101218_vityan/bin/x86_64-vityan-linux-gnu-ld.exe -m elf_x86_64 \
	-T boot/loader/loader.ld \
	-o r:/tofita/loader_kernel.elf.img \
	r:/tofita/loader.asm.o \
	r:/tofita/loader.o \
	r:/tofita/tofitaimg.o
../Teapot/LLVM-9.0.0/bin/llvm-objcopy.exe -O binary r:/tofita/loader_kernel.elf.img r:/tofita/loader_kernel.img

../Teapot/linux-ubuntu-natty_x86_64-bin_x86_64-mingw_20101218_vityan/bin/x86_64-vityan-linux-gnu-ld.exe -m elf_x86_64 \
	-nostdlib -znocombreloc -T external/gnuefi/elf_x86_64_efi.lds -shared -Bsymbolic -L external/gnuefi \
	-L /usr/lib \
	external/gnuefi/crt0-efi-x86_64.o \
	r:/tofita/boot.o \
	-o r:/tofita/loader.so -lgnuefi

../Teapot/x86_64-w64-mingw32/objcopy.exe \
	-j .text -j .sdata -j .data -j .dynamic \
	-j .dynsym  -j .rel -j .rela -j .reloc \
	--target=efi-app-x86_64 R:/tofita/loader.so R:/tofita/loader.efi
