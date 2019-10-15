// The Tofita Kernel
// Copyright (C) 2019  Oleg Petrenko
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, version 3 of the License.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.

// Boot loader: enters efi_main, reads all UEFI data and starts kernel loader

#include <efi.h>
#include <efilib.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdarg.h>

#include "../../devices/serial/init.c"
#include "../shared/boot.h"
#include "memory.c"
#include "ramdisk.c"
#include "../../kernel/ramdisk.c"

void* tmemcpy(void* dest, const void* src, size_t count) {
	uint8_t* dst8 = (uint8_t*)dest;
	uint8_t* src8 = (uint8_t*)src;

	while (count--) {
			*dst8++ = *src8++;
	}

	return dest;
}

// Loading animation, progress 0...2
void drawLoading(Framebuffer* framebuffer, uint8_t progress) {
	uint32_t* pixels = framebuffer->base;
	for (uint8_t y = 0; y < 24; y++)
		for (uint8_t x = 0; x < 24; x++)
			pixels[
				((y + (framebuffer->height/4) * 3) * framebuffer->width)
				+ x + framebuffer->width/2
				- 12 + progress*24*2 - 48
			] = 0xFFFFFFFF;
}

// Entry point
EFI_STATUS efi_main(EFI_HANDLE imageHandle, EFI_SYSTEM_TABLE *systemTable) {
	initSerial();
	serialPrint("\r\n[[[efi_main]]] This is Tofita " Version " UEFI bootloader. Welcome!\r\n");

	serialPrintln("[[[efi_main]]] begin: InitializeLib");
	InitializeLib(imageHandle, systemTable);
	serialPrintln("[[[efi_main]]] done: InitializeLib");

	{
		uint32_t revision = systemTable->FirmwareRevision;
		uint16_t minor = (uint16_t)revision;
		uint16_t major = (uint16_t)(revision >> 16);
		serialPrintf("[[[efi_main]]] UEFI revision %d.%d\r\n", major, minor);
	}

	EFI_BOOT_SERVICES *bootsvc = systemTable->BootServices;

	{
		serialPrintln("[[[efi_main]]] begin: ACPI");
		void *acpiTable = NULL;
		EFI_GUID acpi20 = ACPI_20_TABLE_GUID;
		EFI_GUID acpi = ACPI_TABLE_GUID;

		for (size_t i = 0; i < systemTable->NumberOfTableEntries; i++) {
			EFI_CONFIGURATION_TABLE *efiTable = &systemTable->ConfigurationTable[i];
			if (0 == CompareGuid(&efiTable->VendorGuid, &acpi20)) { // Prefer ACPI 2.0
				acpiTable = efiTable->VendorTable;
				serialPrintln("[[[efi_main]]] found: ACPI 2.0");
				break;
			} else if (0 == CompareGuid(&efiTable->VendorGuid, &acpi)) {
				acpiTable = (void *)((intptr_t)efiTable->VendorTable | 0x1); // LSB high
				serialPrintln("[[[efi_main]]] found: ACPI 1.0");
			}
		}
		serialPrintln("[[[efi_main]]] done: ACPI");
	}

	KernelParams initParameters;
	initParameters.efiRuntimeServices = RT;
	EFI_STATUS status = EFI_NOT_READY;

	serialPrintln("[[[efi_main]]] begin: initializeFramebuffer");
	initializeFramebuffer(&initParameters.framebuffer, systemTable);
	drawLoading(&initParameters.framebuffer, 0);
	// TODO: render something to show that loader is ok, because initial start form USB may take a while
	serialPrintln("[[[efi_main]]] done: initializeFramebuffer");

	// Initial RAM disk
	findAndLoadRamDisk(systemTable->BootServices, &initParameters.ramdisk);
	drawLoading(&initParameters.framebuffer, 1);

	// Simple memory buffer for in-kernel allocations
	serialPrintln("[[[efi_main]]] begin: uefiAllocate the buffer");
	initParameters.bufferSize = 128 * 1024 * 1024;
	void *address = 342352128; // arbitary physical address to fit in RAM
	size_t size = initParameters.bufferSize;
	status = uefiAllocate(
			bootsvc,
			EfiBootServicesCode,
			&size,
			&address);
	if (status != EFI_SUCCESS) {
		serialPrintf("[[[efi_main]]] <ERROR> failed uefiAllocate with status %d\r\n", status);
	}
	initParameters.buffer = address;
	serialPrintf("[[[efi_main]]] done: uefiAllocate the buffer, size %d\r\n", size);

	serialPrintln("[[[efi_main]]] begin: fillMemoryMap");
	fillMemoryMap(&initParameters.efiMemoryMap, systemTable);
	serialPrintln("[[[efi_main]]] done: fillMemoryMap");

	serialPrintln("[[[efi_main]]] begin: ExitBootServices");
	uint8_t oops = 0;
	status = EFI_NOT_READY;
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
		status = systemTable->BootServices->ExitBootServices(imageHandle,
			initParameters.efiMemoryMap.mapKey);
	}

	if (status != EFI_SUCCESS) {
		serialPrintln("[[[efi_main]]] <ERROR> ExitBootServices: EFI_LOAD_ERROR");
		return EFI_LOAD_ERROR;
	}
	serialPrintln("[[[efi_main]]] done: ExitBootServices");

	setRamDisk(&initParameters.ramdisk);
	RamDiskAsset asset = getRamDiskAsset("loader_kernel.img");
	serialPrintf("[[[efi_main]]] loaded asset 'loader_kernel.img' %d bytes at %d\r\n", asset.size, asset.data);

	void *kernelBase = (void *) KernelStart;
	drawLoading(&initParameters.framebuffer, 2);

	serialPrintln("[[[efi_main]]] begin: preparing kernel loader");

	tmemcpy(kernelBase, asset.data, asset.size);
	InitKernel startFunction = (InitKernel) kernelBase;

	serialPrintln("[[[efi_main]]] done: all done, entering kernel loader");

	startFunction(&initParameters);

	return EFI_SUCCESS;
}
