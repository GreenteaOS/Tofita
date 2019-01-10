#include <efi.h>
#include <efilib.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdarg.h>

#include "../../devices/serial/init.c"
#include "../shared/boot.h"
#include "../shared/paging.c"
#include "memory.c"
#include "ramdisk.c"

void* memcpy(void* dest, const void* src, size_t count) {
	uint8_t* dst8 = (uint8_t*)dest;
	uint8_t* src8 = (uint8_t*)src;

	while (count--) {
			*dst8++ = *src8++;
	}

	return dest;
}

extern uint8_t _binary__mnt_r_tofita_loader_kernel_img_start;
extern uint8_t _binary__mnt_r_tofita_loader_kernel_img_end;

// Entry point
EFI_STATUS efi_main(EFI_HANDLE imageHandle, EFI_SYSTEM_TABLE *systemTable) {
	initSerial();
	serialPrint("\r\n[[[efi_main]]] This is Tofita UEFI bootloader. Welcome!\r\n");

	serialPrintln("[[[efi_main]]] begin: InitializeLib");
	InitializeLib(imageHandle, systemTable);
	serialPrintln("[[[efi_main]]] done: InitializeLib");

	KernelParams initParameters;
	initParameters.efiRuntimeServices = RT;

	serialPrintln("[[[efi_main]]] begin: initializeFramebuffer");
	initializeFramebuffer(&initParameters.framebuffer);
	// TODO: render something to show that loader is ok, because initial start form USB may take a while
	serialPrintln("[[[efi_main]]] done: initializeFramebuffer");

	serialPrintln("[[[efi_main]]] begin: fillMemoryMap");
	fillMemoryMap(&initParameters.efiMemoryMap);
	serialPrintln("[[[efi_main]]] done: fillMemoryMap");

	// Initial RAM disk
	findAndLoadRamDisk(systemTable->BootServices);

	EFI_STATUS status = EFI_NOT_READY;

	serialPrintln("[[[efi_main]]] begin: ExitBootServices");
	uint8_t oops = 0;
	while (status != EFI_SUCCESS) {
		if (oops < 10) serialPrintln("[[[efi_main]]] try: ExitBootServices");
		if (oops == 100) {
			serialPrintln("[[[efi_main]]] <ERROR?> probably infinite loop on ExitBootServices");
			serialPrintln("[[[efi_main]]] <ERROR?> system may or may not start");
			oops = 200;
		}
		if (oops < 100) {
			oops++;
		}
		status = ST->BootServices->ExitBootServices(imageHandle,
			initParameters.efiMemoryMap.mapKey);
	}

	if (status != EFI_SUCCESS) {
		serialPrintln("[[[efi_main]]] <ERROR> ExitBootServices: EFI_LOAD_ERROR");
		return EFI_LOAD_ERROR;
	}
	serialPrintln("[[[efi_main]]] done: ExitBootServices");

	void *kernelBase = (void *) KERNEL_START;
	const void *kernelImage = (const void *) &_binary__mnt_r_tofita_loader_kernel_img_start;
	size_t kernelImgSize = ((size_t) &_binary__mnt_r_tofita_loader_kernel_img_end) - ((size_t) kernelImage);

	serialPrintln("[[[efi_main]]] begin: preparing kernel loader");

	memcpy(kernelBase, kernelImage, kernelImgSize);
	InitKernel startFunction = (InitKernel) kernelBase;

	serialPrintln("[[[efi_main]]] done: all done, entering kernel loader");

	startFunction(&initParameters);

	return EFI_SUCCESS;
}
