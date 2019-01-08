#include <efi.h>
#include <efilib.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdarg.h>

#include "../../devices/serial/init.c"
#include "../shared/boot.h"
#include "../shared/paging.c"

void* memcpy(void* dest, const void* src, size_t count) {
	char* dst8 = (char*)dest;
	char* src8 = (char*)src;

	while (count--) {
			*dst8++ = *src8++;
	}

	return dest;
}

extern char _binary__mnt_r_tofita_loader_kernel_img_start;
extern char _binary__mnt_r_tofita_loader_kernel_img_end;

void fillMemoryMap(EfiMemoryMap *efiMemoryMap) {
	efiMemoryMap->memoryMap = (EFI_MEMORY_DESCRIPTOR *) memoryMapBuffer;
	efiMemoryMap->memoryMapSize = MEMORY_MAP_BUFFER_SIZE;

	EFI_STATUS status = ST->BootServices->GetMemoryMap(
		&efiMemoryMap->memoryMapSize,
		efiMemoryMap->memoryMap,
		&efiMemoryMap->mapKey,
		&efiMemoryMap->descriptorSize,
		&efiMemoryMap->descriptorVersion);

	if (status != EFI_SUCCESS) {
		serialPrint("[[[efi_main]]] fillMemoryMap: failed\r\n");
	}
}

void initializeFramebuffer(Framebuffer *fb) {
	EFI_GUID gopGuid = EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID;
	EFI_GRAPHICS_OUTPUT_PROTOCOL *gop;

	ST->BootServices->LocateProtocol(&gopGuid, NULL, (void **) &gop);

	fb->base = (void *) gop->Mode->FrameBufferBase;
	fb->size = gop->Mode->FrameBufferSize;

	fb->width = gop->Mode->Info->HorizontalResolution;
	fb->height = gop->Mode->Info->VerticalResolution;
	fb->pixelsPerScanline = gop->Mode->Info->PixelsPerScanLine;

	gop->SetMode(gop, gop->Mode->Mode);
}

EFI_STATUS efi_main(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable) {
	initSerial();
	serialPrint("\r\n[[[efi_main]]] This is Tofita UEFI bootloader. Welcome!\r\n");

	serialPrintln("[[[efi_main]]] begin: InitializeLib");
	InitializeLib(ImageHandle, SystemTable);
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

	EFI_STATUS status = EFI_NOT_READY;

	serialPrintln("[[[efi_main]]] begin: ExitBootServices");
	while (status != EFI_SUCCESS) {
		serialPrintln("[[[efi_main]]] try: ExitBootServices");
		status = ST->BootServices->ExitBootServices(ImageHandle,
			initParameters.efiMemoryMap.mapKey);
	}

	if (status != EFI_SUCCESS) {
		serialPrintln("[[[efi_main]]] ExitBootServices: EFI_LOAD_ERROR");
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
