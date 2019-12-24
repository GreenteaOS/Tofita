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

extern "C" {

namespace efi {
#include <efi.hpp>
}

#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdarg.h>

#include "../shared/boot.hpp"
#include "../../devices/serial/init.cpp"
#include "memory.cpp"
#include "ramdisk.cpp"
#include "../../kernel/ramdisk.cpp"

efi::INTN CompareGuid (efi::EFI_GUID *guid1, efi::EFI_GUID *guid2) {
    efi::INT32 *g1, *g2, r;
    g1 = (efi::INT32 *) guid1;
    g2 = (efi::INT32 *) guid2;
    r  = g1[0] - g2[0];
    r |= g1[1] - g2[1];
    r |= g1[2] - g2[2];
    r |= g1[3] - g2[3];
    return r;
}

void* tmemcpy(void* dest, const void* src, uint64_t count) {
	uint8_t* dst8 = (uint8_t*)dest;
	uint8_t* src8 = (uint8_t*)src;

	while (count--) {
			*dst8++ = *src8++;
	}

	return dest;
}

// Loading animation, progress 0...2
function drawLoading(Framebuffer* framebuffer, uint8_t progress) {
	uint32_t* pixels = (uint32_t*)framebuffer->base;
	for (uint8_t y = 0; y < 24; y++)
		for (uint8_t x = 0; x < 24; x++)
			pixels[
				((y + (framebuffer->height/4) * 3) * framebuffer->width)
				+ x + framebuffer->width/2
				- 12 + progress*24*2 - 48
			] = 0xFFFFFFFF;
}

#include "../shared/paging.cpp"

// Entry point
efi::EFI_STATUS efi_main(efi::EFI_HANDLE imageHandle, efi::EFI_SYSTEM_TABLE *systemTable) {
	initSerial();
	serialPrint(u8"\n[[[efi_main]]] Tofita " STR(versionMajor) "." STR(versionMinor) " " versionName " UEFI bootloader. Welcome!\n");

	// Disable watchdog timer
	systemTable->BootServices->SetWatchdogTimer(0, 0, 0, NULL);

	{
		uint32_t revision = systemTable->FirmwareRevision;
		uint16_t minor = (uint16_t)revision;
		uint16_t major = (uint16_t)(revision >> 16);
		serialPrintf(u8"[[[efi_main]]] UEFI revision %d.%d\n", major, minor);
	}

	const uint64_t lower = (uint64_t)1024*1024; // TODO Is this memory really-really allowed by UEFI?
	// TODO Actually, no matter where lower is present, cause no lower-relative addressing done in kernel
	// after calling cr3 at the first instruction
	// so it is safe to allocate it at random position in conventional memory
	const uint64_t loaderSize = (uint64_t)1024*1024;
	// TODO Size should be guessed dynamically

	{
		paging::buffa[0] = 0;

		// Zero fill bootloader memory
		uint8_t* bb = (uint8_t*)lower;
		for (uint64_t i = 0; i < loaderSize; ++i)
		{
			// TODO faster with uint64_t
			bb[i] = paging::buffa[0];
		}
	}

	const uint64_t upper = (uint64_t)0xffff800000000000;
	#define LOWER_TO_UPPER(at) ((uint64_t)(at) - lower + upper)
	KernelParams* initParameters = (KernelParams*)(lower + loaderSize - (uint64_t)4096);
	initParameters->physical = lower;

	initParameters->efiMemoryMap.memoryMapSize = sizeof(efi::EFI_MEMORY_DESCRIPTOR) * 512;
	initParameters->efiMemoryMap.memoryMap = (efi::EFI_MEMORY_DESCRIPTOR *) ((uint64_t)initParameters - initParameters->efiMemoryMap.memoryMapSize);

	// Note: stack grows from x to X-N, not X+N
	uint64_t stack = (uint64_t)initParameters->efiMemoryMap.memoryMap;
	// Align to page
	stack = (uint64_t)(stack / 4096) * 4096 - 4096;
	// TODO: map empty page for stack overflow protection + map larger stack (~4 MB)

	{
		uint8_t* bb = (uint8_t*)initParameters->efiMemoryMap.memoryMap;
		for (uint64_t i = 0; i < initParameters->efiMemoryMap.memoryMapSize; ++i)
		{
			// TODO faster with uint64_t
			bb[i] = paging::buffa[0];
		}
	}

	{
		serialPrintln(u8"[[[efi_main]]] begin: ACPI");
		void *acpiTable = NULL;
		efi::EFI_GUID acpi20 = ACPI_20_TABLE_GUID;
		efi::EFI_GUID acpi = ACPI_TABLE_GUID;

		for (uint64_t i = 0; i < systemTable->NumberOfTableEntries; i++) {
			efi::EFI_CONFIGURATION_TABLE *efiTable = &systemTable->ConfigurationTable[i];
			if (0 == CompareGuid(&efiTable->VendorGuid, &acpi20)) { // Prefer ACPI 2.0
				acpiTable = efiTable->VendorTable;
				serialPrintln(u8"[[[efi_main]]] found: ACPI 2.0");
				break;
			} else if (0 == CompareGuid(&efiTable->VendorGuid, &acpi)) {
				// acpiTable = (void *)((intptr_t)efiTable->VendorTable | 0x1); // LSB high
				// ACPI 2.0 is required by Windows 7
				// So we don't need to support ACPI 1.0
				serialPrintln(u8"[[[efi_main]]] found: ACPI 1.0, ignoring");
			}
		}

		// TODO also transfer ACPI version to report SandyBridge
		initParameters->acpiTablePhysical = (uint64_t)(acpiTable);
		serialPrintln(u8"[[[efi_main]]] done: ACPI");
	}

	initParameters->efiRuntimeServices = systemTable->RuntimeServices;
	efi::EFI_STATUS status = EFI_NOT_READY;

	serialPrintln(u8"[[[efi_main]]] begin: initializeFramebuffer");
	initializeFramebuffer(&initParameters->framebuffer, systemTable);
	drawLoading(&initParameters->framebuffer, 0);
	// TODO: render something to show that loader is ok, because initial start form USB may take a while
	// TODO: show error message if ram < 512 or < 1024 mb and cancel boot (loop forever)
	serialPrintln(u8"[[[efi_main]]] done: initializeFramebuffer");
	// TODO: log all ^ these to framebuffer (optionally)

	// Initial RAM disk
	findAndLoadRamDisk(systemTable->BootServices, &initParameters->ramdisk);
	drawLoading(&initParameters->framebuffer, 1);

	serialPrintln(u8"[[[efi_main]]] begin: fillMemoryMap");
	fillMemoryMap(&initParameters->efiMemoryMap, systemTable);
	serialPrintln(u8"[[[efi_main]]] done: fillMemoryMap");

	serialPrintln(u8"[[[efi_main]]] begin: ExitBootServices");
	uint8_t oops = 0;
	status = EFI_NOT_READY;
	while (status != EFI_SUCCESS) {
		if (oops < 10) serialPrintln(u8"[[[efi_main]]] try: ExitBootServices");
		if (oops == 100) {
			serialPrintln(u8"[[[efi_main]]] <ERROR?> probably infinite loop on ExitBootServices");
			serialPrintln(u8"[[[efi_main]]] <ERROR?> system may or may not start");
			oops = 200;
		}
		if (oops < 100) {
			oops++;
		}
		status = systemTable->BootServices->ExitBootServices(imageHandle,
			initParameters->efiMemoryMap.mapKey);
	}

	if (status != EFI_SUCCESS) {
		// TODO `status` to string
		serialPrintln(u8"[[[efi_main]]] <ERROR> ExitBootServices: EFI_LOAD_ERROR");
		return EFI_LOAD_ERROR;
	}
	serialPrintln(u8"[[[efi_main]]] done: ExitBootServices");

	setRamDisk(&initParameters->ramdisk);
	RamDiskAsset asset = getRamDiskAsset(u8"loader.tofita");
	serialPrintf(u8"[[[efi_main]]] loaded asset 'loader.tofita' %d bytes at %d\n", asset.size, asset.data);

	void *kernelBase = (void *) initParameters->physical;
	drawLoading(&initParameters->framebuffer, 2);

	serialPrintln(u8"[[[efi_main]]] begin: preparing kernel loader");

	tmemcpy(kernelBase, asset.data, asset.size);
	InitKernelTrampoline startFunction = (InitKernelTrampoline) kernelBase;

	serialPrintln(u8"[[[efi_main]]] loading trump-o-line");

	RamDiskAsset trampoline = getRamDiskAsset(u8"trampoline.tofita");
	serialPrintf(u8"[[[efi_main]]] loaded asset 'trampoline.tofita' %d bytes at %d\n", trampoline.size, trampoline.data);

	tmemcpy((void *)(initParameters->physical + 1024*1024), trampoline.data, trampoline.size);
	startFunction = (InitKernelTrampoline) (initParameters->physical + 1024*1024);

	serialPrintln(u8"[[[efi_main]]] mapping pages for kernel loader");

	const uint64_t pml4 = paging::enablePaging(&initParameters->efiMemoryMap, &initParameters->framebuffer, &initParameters->ramdisk, initParameters);
	initParameters->pml4 = pml4; // TODO set virtual address instead?
	initParameters->stack = stack; // TODO set virtual address instead?
	initParameters->lastPageIndexCache = paging::lastPageIndex;

	// Convert addresses to upper half
	initParameters = (KernelParams*)LOWER_TO_UPPER(initParameters);
	// TODO deep pointer conversion: initParameters->efiMemoryMap.memoryMap = (EFI_MEMORY_DESCRIPTOR *)LOWER_TO_UPPER(initParameters->efiMemoryMap.memoryMap);
	stack = LOWER_TO_UPPER(stack);

	serialPrintln(u8"[[[efi_main]]] done: all done, entering kernel loader");
	startFunction((uint64_t)initParameters, pml4, stack, upper);

	return EFI_SUCCESS;
}
}
