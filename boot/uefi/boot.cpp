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

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

typedef void *EFI_HANDLE;
typedef void VOID;

#include <stddef.h>
#include <stdbool.h>
#include <stdarg.h>

#include "../shared/boot.hpp"
#include "../../devices/serial/log.cpp"

extern const uint8_t binFont[];
extern const uint8_t binFontBitmap[];
extern const uint8_t binLeavesBitmap[];
int64_t _fltused = 0;

#ifdef __cplusplus
extern "C++" {
template <int v>
struct display_non_zero_int_value;
template <>
struct display_non_zero_int_value<0> { static constexpr bool foo = true; };
static constexpr int v = sizeof(int);
//static_assert(v == 0 && display_non_zero_int_value<v>::foo, "v == 0");
}
#endif

EFI_HANDLE imageHandle = nullptr;
struct EFI_SYSTEM_TABLE_;
struct EFI_SYSTEM_TABLE_
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
#define HEXA_NO_DEFAULT_INCLUDES

// CR3 trampoline
externC function __attribute__((fastcall))
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
	HEXA_MAIN(0, nullptr);
#if 0
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
#endif

	return 0;
}

#ifdef __cplusplus
}
#endif
