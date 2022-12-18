// The Tofita Kernel
// Copyright (C) 2020-2022 Oleh Petrenko
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

#include <stdint.h>

namespace efi { // TODO dont use ns cause C mode!
typedef void *EFI_HANDLE;
typedef void VOID;
}

#include <stddef.h>
#include <stdbool.h>
#include <stdarg.h>

#include "../shared/boot.hpp"
#include "../../devices/serial/log.cpp"

extern const uint8_t binFont[];
extern const uint8_t binFontBitmap[];
extern const uint8_t binLeavesBitmap[];
int64_t _fltused = 0;

extern "C++" {
template <int v>
struct display_non_zero_int_value;
template <>
struct display_non_zero_int_value<0> { static constexpr bool foo = true; };
static constexpr int v = sizeof(int);
//static_assert(v == 0 && display_non_zero_int_value<v>::foo, "v == 0");
}

efi::EFI_HANDLE imageHandle = nullptr;
struct EFI_SYSTEM_TABLE_;
EFI_SYSTEM_TABLE_
*systemTable = nullptr;

#define Virtual_$Any_ void
#define Physical_$Any_ void
#define ArrayByValue_$uint16_t$Any_ void

#define HEAP_ZERO_MEMORY 0
#define stdout 0
#define HANDLE void*

static void ExitProcess(int32_t x) {
}

static void wprintf(const wchar_t* x, const void* y) {
	serialPrintf(x, y);
}

static void fflush(void* x) {/*serialPrint(L"fflush;\n");*/}

static void free(void* x) {/*serialPrint(L"free;\n");*/}

static uint32_t strlen(const char *x) {
	serialPrint(L"strlen;\n");
	return kstrlen((const uint8_t *)x);
}

static void printf(const void* x,...) {serialPrint(L"printf;\n");}
#define macro_serialPrintf(print_, ...) serialPrintf((const wchar_t *)print_->_->utf16_(print_), ## __VA_ARGS__)

#define HEAP_C 655360
static uint8_t heap[HEAP_C] = {0};
static uint64_t heapOffset = 0;
static void* HeapAlloc(int8_t x,int8_t u, uint64_t size) {
	// size = ((size - 1) | 7) + 1; // Align by 8
	size = ((size - 1) | 15) + 1; // Align by 16
	if (size < 16) size = 16;
	heapOffset += 8;
	heapOffset += size;
	if (heapOffset >= HEAP_C) {
		serialPrint(L"Heap overflow\n");
		serialPrint(L"Heap overflow\n");
		serialPrint(L"Heap overflow\n");
		while (1) {};
	}
	return &heap[heapOffset - size];
}

#define WriteFile(x,y,z,t,r) {}
#define FlushFileBuffers(x) {}
#define GetStdHandle(x) 0

void *tmemcpy(void *dest, const void *src, uint64_t count);
//extern "C" static
void memcpy(void* x,const void* y,uint64_t z) {
	serialPrintf(L"memcpy %8 %8 %u;\n", x,y,z);
	tmemcpy(x,y,z);
	serialPrintf(L"memcpy done %8 %8 %u;\n", x,y,z);
}

#define GetProcessHeap() 0
#define DWORD uint32_t
using namespace efi; // TODO remove this
#define HEXA_NO_DEFAULT_INCLUDES

// CR3 trampoline
extern "C" function __attribute__((fastcall))
trampolineCR3(volatile uint64_t kernelParams, volatile uint64_t pml4, volatile uint64_t stack,
			  volatile uint64_t entry);
#include "../uefi.c"
#if 0
_Static_assert(sizeof(efi::EFI_GUID) == sizeof(EFI_GUID_), "sizeof");
_Static_assert(sizeof(efi::EFI_TABLE_HEADER) == sizeof(EFI_TABLE_HEADER_), "sizeof");
_Static_assert(sizeof(efi::EFI_SYSTEM_TABLE) == sizeof(EFI_SYSTEM_TABLE_), "sizeof");
_Static_assert(sizeof(efi::EFI_RUNTIME_SERVICES) == sizeof(EFI_RUNTIME_SERVICES_), "sizeof");
_Static_assert(sizeof(efi::EFI_BOOT_SERVICES) == sizeof(EFI_BOOT_SERVICES_), "sizeof");
_Static_assert(sizeof(efi::EFI_CONFIGURATION_TABLE) == sizeof(EFI_CONFIGURATION_TABLE_), "sizeof");
_Static_assert(sizeof(efi::EFI_STATUS) == sizeof(uint64_t), "sizeof");
_Static_assert(sizeof(efi::EFI_GRAPHICS_OUTPUT_PROTOCOL) == sizeof(EFI_GRAPHICS_OUTPUT_PROTOCOL_), "sizeof");
_Static_assert(sizeof(efi::EFI_GRAPHICS_OUTPUT_PROTOCOL_MODE) == sizeof(EFI_GRAPHICS_OUTPUT_PROTOCOL_MODE_), "sizeof");
_Static_assert(sizeof(efi::EFI_GRAPHICS_OUTPUT_MODE_INFORMATION) == sizeof(EFI_GRAPHICS_OUTPUT_MODE_INFORMATION_), "sizeof");
_Static_assert(sizeof(efi::EFI_PIXEL_BITMASK) == sizeof(EFI_PIXEL_BITMASK_), "sizeof");
_Static_assert(sizeof(efi::EFI_MEMORY_DESCRIPTOR) == sizeof(EFI_MEMORY_DESCRIPTOR_), "sizeof");
_Static_assert(sizeof(efi::EFI_TIME_CAPABILITIES) == sizeof(EFI_TIME_CAPABILITIES_), "sizeof");
_Static_assert(sizeof(efi::EFI_SIMPLE_FILE_SYSTEM_PROTOCOL) == sizeof(EFI_SIMPLE_FILE_SYSTEM_PROTOCOL_), "sizeof");
_Static_assert(sizeof(efi::EFI_FILE_PROTOCOL) == sizeof(EFI_FILE_PROTOCOL_), "sizeof");
_Static_assert(sizeof(efi::EFI_TIME) == sizeof(EFI_TIME_), "sizeof");
#endif

#if 0
efi::INTN compareGuid(efi::EFI_GUID *guid1, efi::EFI_GUID *guid2) {
	efi::INT32 *g1, *g2, r;
	g1 = (efi::INT32 *)guid1;
	g2 = (efi::INT32 *)guid2;
	r = g1[0] - g2[0];
	r |= g1[1] - g2[1];
	r |= g1[2] - g2[2];
	r |= g1[3] - g2[3];
	return r;
}
#endif

void *tmemcpy(void *dest, const void *src, uint64_t count) {
	uint8_t *dst8 = (uint8_t *)dest;
	const uint8_t *src8 = (const uint8_t *)src;

	while (count--) {
		*dst8++ = *src8++;
	}

	return dest;
}

#if 0
// Loading animation, progress 0...2
function drawLoading(Framebuffer *framebuffer, uint8_t progress) {
	for (uint8_t y = 0; y < leavesHeight; y++)
		for (uint8_t x = 0; x < leavesWidth; x++) {
			let pixel = getBitmapPixel(binLeavesBitmap, x, y, leavesWidth, leavesHeight);
			drawPixel(framebuffer,
					  x + framebuffer->width / 2 - (leavesWidth / 2) + progress * leavesWidth * 2 -
						  leavesWidth * 2,
					  y + (framebuffer->height / 4) * 3, pixel);
		}
}
#endif

void *memset(void *dest, int32_t e, uint64_t len) {
	uint8_t *d = (uint8_t *)dest;
	for (uint64_t i = 0; i < len; i++, d++) {
		*d = e;
	}
	return dest;
}

#if 0
struct ACPITableHeader {
	uint32_t type;
	uint32_t length;
	uint8_t revision;
	uint8_t checksum;
	char8_t oem_id[6];
	char8_t oem_table[8];
	uint32_t oem_revision;
	uint32_t creator_id;
	uint32_t creator_revision;
} __attribute__((packed));
_Static_assert(sizeof(ACPITableHeader) == 36, "ACPITableHeader must be 36 bytes");

#pragma pack(1)
extern "C++"
struct XSDT {
	ACPITableHeader header;
	ACPITableHeader *headers[1];

	template <typename T> static uint64_t acpiTableEntries(const T *t, uint64_t size) {
		return (t->header.length - sizeof(T)) / size;
	}

	static constexpr uint32_t byteswap(uint32_t x) {
		return ((x >> 24) & 0x000000ff) | ((x >> 8) & 0x0000ff00) | ((x << 8) & 0x00ff0000) |
			((x << 24) & 0xff000000);
	}

	static void putSig(char8_t *into, uint32_t type) {
		for (int32_t j = 0; j < 4; ++j)
			into[j] = reinterpret_cast<char8_t *>(&type)[j];
	}
} __attribute__((packed));
#pragma pack()

SIZEOF(XSDT, sizeof(ACPITableHeader) + 8)

struct APIC {
	ACPITableHeader header;
	uint32_t localAddress;
	uint32_t flags;
	uint8_t controllerData[0];
} __attribute__((packed));

struct ACPI {
	char8_t signature[8];
	uint8_t checksum10;
	char8_t oemID[6];
	uint8_t revision;
	uint32_t rsdtAddress;

	uint32_t length;
	uint64_t xsdtAddress;
	uint8_t checksum20;
	uint8_t reserved[3];
};
#endif

// Entry point
/*efi::EFI_STATUS*/uint64_t efi_main(/*efi::EFI_HANDLE*/void* imageHandle__, /*efi::EFI_SYSTEM_TABLE*/void *systemTable__) {
	imageHandle = imageHandle__;
	systemTable = (EFI_SYSTEM_TABLE_*)systemTable__;
	for (uint64_t i = 0; i < HEAP_C; i++) heap[i] = 0;
	if (systemTable) {
		HEXA_MAIN(0, nullptr);
	}
#if 0
	initSerial();
	serialPrint(L"\n[[[efi_main]]] Tofita " versionName " UEFI bootloader. Welcome!\n");

	initText();

	// Disable watchdog timer
	systemTable->BootServices->SetWatchdogTimer(0, 0, 0, NULL);

	{
		uint32_t revision = systemTable->FirmwareRevision;
		uint16_t minor = (uint16_t)revision;
		uint16_t major = (uint16_t)(revision >> 16);
		serialPrintf(L"[[[efi_main]]] UEFI revision %d.%d\n", major, minor);
	}

	// Actually, no matter where lower is present, cause no lower-relative addressing done in kernel
	// after calling cr3 at the first instruction
	// so it is safe to allocate it at random position in conventional memory

	const uint64_t upper = (uint64_t)0xffff800000000000;
	void *acpiTable = NULL;
	{
		serialPrintln(L"[[[efi_main]]] begin: ACPI");
		efi::EFI_GUID acpi20 = ACPI_20_TABLE_GUID;
		efi::EFI_GUID acpi = ACPI_TABLE_GUID;

		for (uint64_t i = 0; i < systemTable->NumberOfTableEntries; i++) {
			efi::EFI_CONFIGURATION_TABLE *efiTable = &systemTable->ConfigurationTable[i];
			if (0 == compareGuid(&efiTable->VendorGuid, &acpi20)) { // Prefer ACPI 2.0
				acpiTable = efiTable->VendorTable;
				serialPrintln(L"[[[efi_main]]] found: ACPI 2.0");
				break;
			} else if (0 == compareGuid(&efiTable->VendorGuid, &acpi)) {
				// acpiTable = (void *)((intptr_t)efiTable->VendorTable | 0x1); // LSB high
				// ACPI 2.0 is required by Tofita
				// So we don't need to support ACPI 1.0
				serialPrintln(L"[[[efi_main]]] found: ACPI 1.0, ignoring");
			}
		}

		// TODO also transfer ACPI version to report SandyBridge
		serialPrintln(L"[[[efi_main]]] done: ACPI");
	}

	efi::EFI_STATUS status = EFI_NOT_READY;

	serialPrintln(L"[[[efi_main]]] begin: initializeFramebuffer");
	Framebuffer framebuffer;
	initializeFramebuffer(&framebuffer, systemTable);
	drawLoading(&framebuffer, 0);
	drawText(L"Greentea OS" " " versionName,
			 (framebuffer.height / 4) * 3 + 64, &framebuffer);
	const uint16_t errorY = (framebuffer.height / 4) * 3 + 64 + 32;

	// Check ACPI here, after framebuffer initialization
	while (acpiTable == NULL) {
		drawText(L"[ERROR] Tofita requires ACPI 2.0 [ERROR]", errorY, &framebuffer);
		serialPrintf(L"");
	}

	// TODO Validate ACPI signatures here instead of kernel
	// Validate this is a multi-core CPU with Local APICs
	{
		// TODO If bit 1 in the flags field is set then you need to mask all the 8259 PIC's interrupts, but you should probably do this anyway.

		const ACPI* tables = (ACPI *)acpiTable;
		auto xsdt = (const XSDT *)((uint64_t)tables->xsdtAddress);
		char8_t sig[5] = {0, 0, 0, 0, 0};
		XSDT::putSig(sig, xsdt->header.type);
		serialPrintf(L"[[[efi_main]]] XSDT table is %s\n", sig);

		uint16_t cpus = 0;
		uint64_t numTables = XSDT::acpiTableEntries(xsdt, sizeof(void *));
		let apicSignature = XSDT::byteswap('APIC');

		serialPrintf(L"[[[efi_main]]] XSDT has %d tables\n", numTables);

		for (uint64_t i = 0; i < numTables; ++i) {
			auto header = (const ACPITableHeader *)((uint64_t)xsdt->headers[i]);

			XSDT::putSig(sig, header->type);
			serialPrintf(L"[[[efi_main]]] Found table %s\n", sig);

			switch (header->type) {
			case apicSignature:
				{
					auto apic = (const APIC *)header;
					let count = XSDT::acpiTableEntries(apic, 1);
					uint8_t const *data = apic->controllerData;
					uint8_t const *end = data + count;

					while (data < end) {
						const uint8_t type = data[0];
						const uint8_t length = data[1];

						switch (type) {
							case 0:
								// TODO If flags bit 0 is set the CPU is able to be enabled, if it is not set you need to check bit 1.
								// If that one is set you can still enable it, if it is not the CPU cannot be enabled and the OS should not try.
								cpus++;
								break;
						}

						data += length;
					}
				}
				break;

			default:
				break;
			}
		}

		serialPrintf(L"[[[efi_main]]] Found %d CPUs\n", cpus);
		while (cpus < 2) {
			drawText(L"[ERROR] Tofita requires multi-core CPU [ERROR]", errorY, &framebuffer);
			serialPrintf(L"");
		};
	}

	// TODO: render something to show that loader is ok, because initial start form USB may take a while
	// TODO: show error message if ram < 512 or < 1024 mb and cancel boot (loop forever)
	serialPrintln(L"[[[efi_main]]] done: initializeFramebuffer");
	// TODO: log all ^ these to framebuffer (optionally)

	// Initial RAM disk
	RamDisk ramdisk;
	status = findAndLoadRamDisk(systemTable->BootServices, &ramdisk);
	while (status != EFI_SUCCESS) {
		drawText(L"[ERROR] Tofita cannot load ramdisk [ERROR]", errorY, &framebuffer);
		serialPrintf(L"");
	}
	drawLoading(&framebuffer, 1);

	serialPrintln(L"[[[efi_main]]] begin: fillMemoryMap");
	uint64_t sizeAlloc = (ramdisk.size / PAGE_SIZE + 1) * PAGE_SIZE;
	EfiMemoryMap efiMemoryMap;
	efiMemoryMap.memoryMapSize = sizeof(efi::EFI_MEMORY_DESCRIPTOR) * 512;
	efiMemoryMap.memoryMap = (efi::EFI_MEMORY_DESCRIPTOR *)(ramdisk.base + sizeAlloc);
	{
		uint8_t *b = (uint8_t *)efiMemoryMap.memoryMap;
		for (uint64_t i = 0; i < efiMemoryMap.memoryMapSize; ++i) {
			// TODO faster with uint64_t
			b[i] = paging::buffa[0];
		}
	}
	fillMemoryMap(&efiMemoryMap, systemTable);
	serialPrintln(L"[[[efi_main]]] done: fillMemoryMap");

	efi::EFI_TIME time;
	time.Year = 2020;
	efi::EFI_TIME_CAPABILITIES capabilities;
	{
		serialPrintln(L"[[[efi_main]]] begin: GetTime");
		efi::EFI_STATUS status = systemTable->RuntimeServices->GetTime(&time, &capabilities);
		while (status != EFI_SUCCESS) {
			serialPrintln(L"[[[efi_main]]] <ERROR> GetTime");
			drawText(L"[ERROR] Cannot get current time from UEFI [ERROR]", errorY, &framebuffer);
		}

		{
			serialPrintf(L"[[[efi_main]]] EFI_TIME Year=%d Month=%d Day=%d Hour=%d Minute=%d Second=%d "
						 L"Nanosecond=%d TimeZone=%d Daylight=%d\n",
						 time.Year, time.Month, time.Day, time.Hour, time.Minute, time.Second,
						 time.Nanosecond, time.TimeZone, time.Daylight);

			serialPrintf(L"[[[efi_main]]] EFI_TIME_CAPABILITIES Resolution=%d Accuracy=%d SetsToZero=%d\n",
						 capabilities.Resolution, capabilities.Accuracy, capabilities.SetsToZero);

			// Set 2020 as least valid, so security/HTTPS certificates at least partially work
			// and file system is not messed
			if (time.Year < 2020)
				time.Year = 2020;
		}
		serialPrintln(L"[[[efi_main]]] done: GetTime");
	}

	serialPrintln(L"[[[efi_main]]] begin: ExitBootServices");
	uint8_t oops = 0;
	status = EFI_NOT_READY;
	while (status != EFI_SUCCESS) {
		if (oops < 10)
			serialPrintln(L"[[[efi_main]]] try: ExitBootServices");
		if (oops == 100) {
			drawText(L"Loading...", errorY, &framebuffer);
			serialPrintln(L"[[[efi_main]]] <ERROR?> probably infinite loop on ExitBootServices");
			serialPrintln(L"[[[efi_main]]] <ERROR?> system may or may not start");
			oops = 200;
		}
		if (oops < 100) {
			oops++;
		}
		status = systemTable->BootServices->ExitBootServices(imageHandle, efiMemoryMap.mapKey);
	}

	if (status != EFI_SUCCESS) {
		// TODO `status` to string
		serialPrintln(L"[[[efi_main]]] <ERROR> ExitBootServices: EFI_LOAD_ERROR");
		return EFI_LOAD_ERROR;
	}
	serialPrintln(L"[[[efi_main]]] done: ExitBootServices");

	setRamDisk(&ramdisk);

	serialPrintln(L"[[[efi_main]]] begin: preparing kernel loader");

	RamDiskAsset asset = getRamDiskAsset(L"tofita.gnu");
	serialPrintf(L"[[[efi_main]]] loaded asset 'tofita.gnu' %d bytes at %d\n", asset.size, asset.data);

	const uint64_t largeBuffer = paging::conventionalAllocateLargest(&efiMemoryMap);
	serialPrintf(L"[[[efi_main]]] large buffer allocated at %u\n", largeBuffer);
	paging::conventionalOffset = largeBuffer;
	uint64_t mAddressOfEntryPoint = 0;

	{
		auto ptr = (uint8_t *)asset.data;
		auto peHeader = (const PeHeader *)((uint64_t)ptr + ptr[0x3C] + ptr[0x3C + 1] * 256);
		serialPrintf(L"PE header signature 'PE' == '%s'\n", peHeader);
		auto peOptionalHeader = (const Pe32OptionalHeader *)((uint64_t)peHeader + sizeof(PeHeader));
		serialPrintf(L"PE32(+) optional header signature 0x020B == %d == %d\n", peOptionalHeader->mMagic,
					 0x020B);
		serialPrintf(L"PE32(+) size of image == %d\n", peOptionalHeader->mSizeOfImage);
		void *kernelBase = (void *)paging::conventionalAllocateNext(peOptionalHeader->mSizeOfImage);
		memset(kernelBase, 0, peOptionalHeader->mSizeOfImage); // Zeroing

		// Copy sections
		auto imageSectionHeader =
			(const ImageSectionHeader *)((uint64_t)peOptionalHeader + peHeader->mSizeOfOptionalHeader);
		for (uint16_t i = 0; i < peHeader->mNumberOfSections; ++i) {
			serialPrintf(L"Copy section [%d] named '%s' of size %d\n", i,
				&imageSectionHeader[i].mName,
				imageSectionHeader[i].mSizeOfRawData
			);
			uint64_t where = (uint64_t)kernelBase + imageSectionHeader[i].mVirtualAddress;

			tmemcpy((void *)where,
					(void *)((uint64_t)asset.data + (uint64_t)imageSectionHeader[i].mPointerToRawData),
					imageSectionHeader[i].mSizeOfRawData);
		}

		mAddressOfEntryPoint = peOptionalHeader->mAddressOfEntryPoint;
	}

	KernelParams *params = (KernelParams *)paging::conventionalAllocateNext(sizeof(KernelParams));
	{
		uint8_t *b = (uint8_t *)params;
		for (uint64_t i = 0; i < sizeof(KernelParams); ++i) {
			b[i] = paging::buffa[0];
		}
	}

	params->time = time;
	params->capabilities = capabilities;

	// Note: stack grows from x to X-N, not X+N
	// TODO: map empty page for stack overflow protection + map larger stack (~4 MB)
	var stack = paging::conventionalAllocateNext(1024 * 1024) + 1024 * 1024;
	{
		uint8_t *b = (uint8_t *)(stack - 1024 * 1024);
		for (uint64_t i = 0; i < 1024 * 1024; ++i) {
			b[i] = paging::buffa[0];
		}
	}

	params->efiMemoryMap = efiMemoryMap;
	params->ramdisk = ramdisk;
	params->framebuffer = framebuffer;

	// RAM usage bit-map

	uint64_t ram = paging::getRAMSize(&params->efiMemoryMap);
	uint32_t megs = (uint32_t)(ram / (1024 * 1024));
	serialPrintf(L"[paging] available RAM is ~%u megabytes\n", megs);
	while (megs < 1234) {
		drawText(L"[ERROR] Tofita requires at least 2 GB of memory [ERROR]", errorY, &framebuffer);
		serialPrintf(L"Tofita requires at least 2 GB of memory\n");
	}
	params->ramBytes = ram;
	params->physicalRamBitMaskVirtual = paging::conventionalAllocateNext(ram >> 12);

	paging::pml4entries =
		(paging::PageEntry *)paging::conventionalAllocateNext(sizeof(paging::PageEntry) * PAGE_TABLE_SIZE);

	{
		uint8_t *b = (uint8_t *)paging::pml4entries;
		for (uint64_t i = 0; i < paging::conventionalOffset; ++i) {
			// TODO faster with uint64_t
			b[i] = paging::buffa[0];
		}
	}

	// Map memory

	serialPrintln(L"[[[efi_main]]] mapping pages for kernel loader");

	paging::mapMemory(upper, largeBuffer, (paging::conventionalOffset - largeBuffer) / PAGE_SIZE + 1, 1);

	// Note: framebuffer is *not* within physical memory
	paging::mapFramebuffer(&params->framebuffer);
	drawLoading(&framebuffer, 2);
	paging::mapEfi(&params->efiMemoryMap);
	paging::mapMemoryHuge(WholePhysicalStart, 0, ram / PAGE_SIZE);

	let startFunction = (InitKernelTrampoline)(paging::conventionalOffset + PAGE_SIZE);
	tmemcpy((void *)startFunction, &trampolineCR3, 32);
	paging::mapMemory((uint64_t)startFunction, (uint64_t)startFunction, 1, 0);

	// Fix virtual addresses

	params->framebuffer.physical = params->framebuffer.base;
	params->framebuffer.base = FramebufferStart;

	params->ramdisk.physical = params->ramdisk.base;
	params->ramdisk.base = WholePhysicalStart + params->ramdisk.physical;

	params->pml4 = (uint64_t)paging::pml4entries; // physical address for CPU
	params->stack = stack;						  // physical address for stack overflow detection
	params->physicalBuffer = largeBuffer;
	params->physicalBytes = paging::conventionalOffset - largeBuffer;
	params->efiRuntimeServices = systemTable->RuntimeServices;
	params->acpiTablePhysical = (uint64_t)(acpiTable);
	params->efiMemoryMap.memoryMap =
		(efi::EFI_MEMORY_DESCRIPTOR *)(WholePhysicalStart + (uint64_t)params->efiMemoryMap.memoryMap);

	// Convert addresses to upper half

	stack = (uint64_t)WholePhysicalStart + stack;
	params->physicalRamBitMaskVirtual = (uint64_t)WholePhysicalStart + params->physicalRamBitMaskVirtual;
	params = (KernelParams *)((uint64_t)WholePhysicalStart + (uint64_t)params);

	serialPrintln(L"[[[efi_main]]] done: all done, entering kernel loader");

	serialPrint(L"[[[efi_main]]] CR3 points to: ");
	serialPrintHex((uint64_t)paging::pml4entries);
	serialPrint(L"\n");

	startFunction((uint64_t)params, (uint64_t)paging::pml4entries, stack, upper + mAddressOfEntryPoint);
#endif

	return 0;//EFI_SUCCESS;
}
}
