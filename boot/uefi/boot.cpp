// The Tofita Kernel
// Copyright (C) 2020  Oleg Petrenko
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
#include "pe.cpp"
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

void *memset(void *dest, int32_t e, uint64_t len) {
	uint8_t *d = (uint8_t *)dest;
	for (uint64_t i = 0; i < len; i++, d++) {
		*d = e;
	}
	return dest;
}

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

	// Actually, no matter where lower is present, cause no lower-relative addressing done in kernel
	// after calling cr3 at the first instruction
	// so it is safe to allocate it at random position in conventional memory

	const uint64_t upper = (uint64_t)0xffff800000000000;
	void *acpiTable = NULL;
	{
		serialPrintln(u8"[[[efi_main]]] begin: ACPI");
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
		serialPrintln(u8"[[[efi_main]]] done: ACPI");
	}

	efi::EFI_STATUS status = EFI_NOT_READY;

	serialPrintln(u8"[[[efi_main]]] begin: initializeFramebuffer");
	Framebuffer framebuffer;
	initializeFramebuffer(&framebuffer, systemTable);
	drawLoading(&framebuffer, 0);
	// TODO: render something to show that loader is ok, because initial start form USB may take a while
	// TODO: show error message if ram < 512 or < 1024 mb and cancel boot (loop forever)
	serialPrintln(u8"[[[efi_main]]] done: initializeFramebuffer");
	// TODO: log all ^ these to framebuffer (optionally)

	// Initial RAM disk
	RamDisk ramdisk;
	findAndLoadRamDisk(systemTable->BootServices, &ramdisk);
	drawLoading(&framebuffer, 1);

	serialPrintln(u8"[[[efi_main]]] begin: fillMemoryMap");
	uint64_t sizeAlloc = (ramdisk.size / PAGE_SIZE + 1) * PAGE_SIZE;
	EfiMemoryMap efiMemoryMap;
	efiMemoryMap.memoryMapSize = sizeof(efi::EFI_MEMORY_DESCRIPTOR) * 512;
	efiMemoryMap.memoryMap = (efi::EFI_MEMORY_DESCRIPTOR *)(ramdisk.base + sizeAlloc);
	{
		uint8_t* b = (uint8_t*)efiMemoryMap.memoryMap;
		for (uint64_t i = 0; i < efiMemoryMap.memoryMapSize; ++i)
		{
			// TODO faster with uint64_t
			b[i] = paging::buffa[0];
		}
	}
	fillMemoryMap(&efiMemoryMap, systemTable);
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
			efiMemoryMap.mapKey);
	}

	if (status != EFI_SUCCESS) {
		// TODO `status` to string
		serialPrintln(u8"[[[efi_main]]] <ERROR> ExitBootServices: EFI_LOAD_ERROR");
		return EFI_LOAD_ERROR;
	}
	serialPrintln(u8"[[[efi_main]]] done: ExitBootServices");

	setRamDisk(&ramdisk);

	serialPrintln(u8"[[[efi_main]]] begin: preparing kernel loader");

	RamDiskAsset asset = getRamDiskAsset(u8"tofita.gnu");
	serialPrintf(u8"[[[efi_main]]] loaded asset 'tofita.gnu' %d bytes at %d\n", asset.size, asset.data);

	const uint64_t largeBuffer = paging::conventionalAllocateLargest(&efiMemoryMap);
	serialPrintf(u8"[[[efi_main]]] large buffer allocated at %u\n", largeBuffer);
	paging::conventionalOffset = largeBuffer;
	uint64_t mAddressOfEntryPoint = 0;

	{
		auto ptr = (uint8_t*)asset.data;
		auto peHeader = (const PeHeader*)((uint64_t)ptr + ptr[0x3C] + ptr[0x3C + 1] * 256);
		serialPrintf(u8"PE header signature 'PE' == '%s'\n", peHeader);
		auto peOptionalHeader = (const Pe32OptionalHeader*)((uint64_t)peHeader + sizeof(PeHeader));
		serialPrintf(u8"PE32(+) optional header signature 0x020B == %d == %d\n", peOptionalHeader->mMagic, 0x020B);
		serialPrintf(u8"PE32(+) size of image == %d\n", peOptionalHeader->mSizeOfImage);
		void *kernelBase = (void *) paging::conventionalAllocateNext(peOptionalHeader->mSizeOfImage);
		memset(kernelBase, 0, peOptionalHeader->mSizeOfImage); // Zeroing

		// Copy sections
		auto imageSectionHeader = (const ImageSectionHeader*)((uint64_t)peOptionalHeader + peHeader->mSizeOfOptionalHeader);
		for (int i = 0; i < peHeader->mNumberOfSections; ++i) {
			serialPrintf(u8"Copy section [%d] named '%s' of size %d\n", i, &imageSectionHeader[i].mName, imageSectionHeader[i].mSizeOfRawData);
			uint64_t where = (uint64_t)kernelBase + imageSectionHeader[i].mVirtualAddress;

			tmemcpy(
				(void *)where,
				(void *)((uint64_t)asset.data + (uint64_t)imageSectionHeader[i].mPointerToRawData),
				imageSectionHeader[i].mSizeOfRawData
			);
		}

		mAddressOfEntryPoint = peOptionalHeader->mAddressOfEntryPoint;
	}

	KernelParams* params = (KernelParams*) paging::conventionalAllocateNext(sizeof(KernelParams));
	{
		uint8_t* b = (uint8_t*)params;
		for (uint64_t i = 0; i < sizeof(KernelParams); ++i)
		{
			b[i] = paging::buffa[0];
		}
	}

	// Note: stack grows from x to X-N, not X+N
	// TODO: map empty page for stack overflow protection + map larger stack (~4 MB)
	var stack = paging::conventionalAllocateNext(1024 * 1024) + 1024 * 1024;
	{
		uint8_t* b = (uint8_t*)(stack - 1024 * 1024);
		for (uint64_t i = 0; i < 1024 * 1024; ++i)
		{
			b[i] = paging::buffa[0];
		}
	}

	params->efiMemoryMap = efiMemoryMap;
	params->ramdisk = ramdisk;
	params->framebuffer = framebuffer;

	serialPrintln(u8"[[[efi_main]]] loading trump-o-line");
	RamDiskAsset trampoline = getRamDiskAsset(u8"trampoline.tofita");
	serialPrintf(u8"[[[efi_main]]] loaded asset 'trampoline.tofita' %d bytes at %d\n", trampoline.size, trampoline.data);

	// RAM usage bit-map

	uint64_t ram = paging::getRAMSize(&params->efiMemoryMap);
	uint32_t megs = (uint32_t)(ram/(1024*1024));
	serialPrintf(u8"[paging] available RAM is ~%u megabytes\n", megs);
	while (megs < 768) {
		serialPrintf(u8"Tofita requires at least 1 GB of memory\n");
	}
	params->ramBytes = ram;
	params->physicalRamBitMaskVirtual = paging::conventionalAllocateNext(ram / PAGE_SIZE);

	// Note: paging::PagesArray allocated last

	paging::pml4entries = (paging::PageEntry*) paging::conventionalAllocateNext(sizeof(paging::PageEntry) * PAGE_TABLE_SIZE);

	{
		uint8_t* b = (uint8_t*)paging::pml4entries;
		for (uint64_t i = 0; i < paging::conventionalOffset; ++i)
		{
			// TODO faster with uint64_t
			b[i] = paging::buffa[0];
		}
	}

	// Map memory

	serialPrintln(u8"[[[efi_main]]] mapping pages for kernel loader");

	paging::mapMemory(
		upper,
		largeBuffer,
		(paging::conventionalOffset - largeBuffer) / PAGE_SIZE + 1
	);
	// Note: framebuffer is *not* within physical memory
	paging::mapFramebuffer(&params->framebuffer);
	drawLoading(&framebuffer, 2);
	paging::mapEfi(&params->efiMemoryMap);
	paging::mapMemoryHuge(WholePhysicalStart, 0, ram / PAGE_SIZE);

	let startFunction = (InitKernelTrampoline) (paging::conventionalOffset + PAGE_SIZE);
	tmemcpy((void *)startFunction, trampoline.data, trampoline.size);
	paging::mapMemory((uint64_t)startFunction, (uint64_t)startFunction, 1);

	// Fix virtual addresses

	params->framebuffer.physical = params->framebuffer.base;
	params->framebuffer.base = FramebufferStart;

	params->ramdisk.physical = params->ramdisk.base;
	params->ramdisk.base = WholePhysicalStart + params->ramdisk.physical;

	params->pml4 = (uint64_t)paging::pml4entries; // physical address for CPU
	params->stack = stack; // physical address for stack overflow detection
	params->physicalBuffer = largeBuffer;
	params->physicalBytes = paging::conventionalOffset - largeBuffer;
	params->efiRuntimeServices = systemTable->RuntimeServices;
	params->acpiTablePhysical = (uint64_t)(acpiTable);
	params->efiMemoryMap.memoryMap = (efi::EFI_MEMORY_DESCRIPTOR *)(WholePhysicalStart + (uint64_t)params->efiMemoryMap.memoryMap);

	// Convert addresses to upper half

	stack = (uint64_t)WholePhysicalStart + stack;
	params->physicalRamBitMaskVirtual = (uint64_t)WholePhysicalStart + params->physicalRamBitMaskVirtual;
	params = (KernelParams*)((uint64_t)WholePhysicalStart + (uint64_t)params);

	serialPrintln(u8"[[[efi_main]]] done: all done, entering kernel loader");

	serialPrint(u8"[[[efi_main]]] CR3 points to: ");
	serialPrintHex((uint64_t) paging::pml4entries);
	serialPrint(u8"\n");

	startFunction((uint64_t)params, (uint64_t)paging::pml4entries, stack, upper + mAddressOfEntryPoint);

	return EFI_SUCCESS;
}
}
