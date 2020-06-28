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

namespace exe {

// 1 byte aligned
#pragma pack(1)
struct PeHeader {
	uint32_t magic; // PE\0\0 or 0x00004550
	uint16_t machine;
	uint16_t numberOfSections;
	uint32_t timeDateStamp;
	uint32_t pointerToSymbolTable;
	uint32_t numberOfSymbols;
	uint16_t sizeOfOptionalHeader;
	uint16_t characteristics;
} __attribute__((packed));
#pragma pack()

_Static_assert(sizeof(PeHeader) == 24, "sizeof is incorrect");

// 1 byte aligned
#pragma pack(1)
struct Pe64OptionalHeader {
	uint16_t magic; // 0x010b - PE32, 0x020b - PE32+ (64 bit)
	uint8_t majorLinkerVersion;
	uint8_t minorLinkerVersion;
	uint32_t sizeOfCode;
	uint32_t sizeOfInitializedData;
	uint32_t sizeOfUninitializedData;
	uint32_t addressOfEntryPoint;
	uint32_t baseOfCode;
	uint64_t imageBase;
	uint32_t sectionAlignment;
	uint32_t fileAlignment;
	uint16_t majorOperatingSystemVersion;
	uint16_t minorOperatingSystemVersion;
	uint16_t majorImageVersion;
	uint16_t minorImageVersion;
	uint16_t majorSubsystemVersion;
	uint16_t minorSubsystemVersion;
	uint32_t win32VersionValue;
	uint32_t sizeOfImage;
	uint32_t sizeOfHeaders;
	uint32_t checkSum;
	uint16_t subsystem;
	uint16_t dllCharacteristics;
	uint64_t sizeOfStackReserve;
	uint64_t sizeOfStackCommit;
	uint64_t sizeOfHeapReserve;
	uint64_t sizeOfHeapCommit;
	uint32_t loaderFlags;
	uint32_t numberOfRvaAndSizes;
} __attribute__((packed));
#pragma pack()

// 1 byte aligned
#pragma pack(1)
struct Pe32OptionalHeader {
	uint16_t magic; // 0x010b - PE32, 0x020b - PE32+ (64 bit)
	uint8_t majorLinkerVersion;
	uint8_t minorLinkerVersion;
	uint32_t sizeOfCode;
	uint32_t sizeOfInitializedData;
	uint32_t sizeOfUninitializedData;
	uint32_t addressOfEntryPoint;
	uint32_t baseOfCode;
	uint32_t baseOfData; // present only in 32-bit
	uint32_t imageBase;
	uint32_t sectionAlignment;
	uint32_t fileAlignment;
	uint16_t majorOperatingSystemVersion;
	uint16_t minorOperatingSystemVersion;
	uint16_t majorImageVersion;
	uint16_t minorImageVersion;
	uint16_t majorSubsystemVersion;
	uint16_t minorSubsystemVersion;
	uint32_t win32VersionValue;
	uint32_t sizeOfImage;
	uint32_t sizeOfHeaders;
	uint32_t checkSum;
	uint16_t subsystem;
	uint16_t dllCharacteristics;
	uint32_t sizeOfStackReserve;
	uint32_t sizeOfStackCommit;
	uint32_t sizeOfHeapReserve;
	uint32_t sizeOfHeapCommit;
	uint32_t loaderFlags;
	uint32_t numberOfRvaAndSizes;
} __attribute__((packed));
#pragma pack()

#pragma pack(1)
struct ImageSectionHeader { // size 40 bytes
	uint8_t name[8];
	uint32_t virtualSize;
	uint32_t virtualAddress;
	uint32_t sizeOfRawData;
	uint32_t pointerToRawData;
	uint32_t pointerToRealocations;
	uint32_t pointerToLinenumbers;
	uint16_t numberOfRealocations;
	uint16_t numberOfLinenumbers;
	uint32_t characteristics;
} __attribute__((packed));
#pragma pack()

_Static_assert(sizeof(ImageSectionHeader) == 40, "sizeof is incorrect");

#define IMAGE_DIRECTORY_ENTRY_EXPORT 0	  // Export Directory
#define IMAGE_DIRECTORY_ENTRY_IMPORT 1	  // Import Directory
#define IMAGE_DIRECTORY_ENTRY_RESOURCE 2  // Resource Directory
#define IMAGE_DIRECTORY_ENTRY_EXCEPTION 3 // Exception Directory
#define IMAGE_DIRECTORY_ENTRY_SECURITY 4  // Security Directory
#define IMAGE_DIRECTORY_ENTRY_BASERELOC 5 // Base Relocation Table
#define IMAGE_DIRECTORY_ENTRY_DEBUG 6	  // Debug Directory
//      IMAGE_DIRECTORY_ENTRY_COPYRIGHT       7   // (X86 usage)
#define IMAGE_DIRECTORY_ENTRY_ARCHITECTURE 7	// Architecture Specific Data
#define IMAGE_DIRECTORY_ENTRY_GLOBALPTR 8		// RVA of GP
#define IMAGE_DIRECTORY_ENTRY_TLS 9				// TLS Directory
#define IMAGE_DIRECTORY_ENTRY_LOAD_CONFIG 10	// Load Configuration Directory
#define IMAGE_DIRECTORY_ENTRY_BOUND_IMPORT 11	// Bound Import Directory in headers
#define IMAGE_DIRECTORY_ENTRY_IAT 12			// Import Address Table
#define IMAGE_DIRECTORY_ENTRY_DELAY_IMPORT 13	// Delay Load Import Descriptors
#define IMAGE_DIRECTORY_ENTRY_COM_DESCRIPTOR 14 // COM Runtime descriptor

typedef struct _IMAGE_EXPORT_DIRECTORY {
	uint32_t characteristics;
	uint32_t timeDateStamp;
	uint16_t majorVersion;
	uint16_t minorVersion;
	uint32_t name;
	uint32_t base;
	uint32_t numberOfFunctions;
	uint32_t numberOfNames;
	uint32_t addressOfFunctions;
	uint32_t addressOfNames;
	uint32_t addressOfNameOrdinals;
} IMAGE_EXPORT_DIRECTORY, *PIMAGE_EXPORT_DIRECTORY;

typedef struct _IMAGE_BASE_RELOCATION {
	uint32_t virtualAddress;
	uint32_t sizeOfBlock;
} IMAGE_BASE_RELOCATION, *PIMAGE_BASE_RELOCATION;

typedef struct _IMAGE_DATA_DIRECTORY {
	uint32_t virtualAddress;
	uint32_t size;
} IMAGE_DATA_DIRECTORY, *PIMAGE_DATA_DIRECTORY;
#define IMAGE_SIZEOF_BASE_RELOCATION 8
#define IMAGE_REL_BASED_ABSOLUTE 0
#define IMAGE_REL_BASED_DIR64 10
#define IMAGE_REL_BASED_HIGHLOW 3

#pragma pack(1)
typedef struct _IMAGE_IMPORT_DESCRIPTOR {
	//_ANONYMOUS_UNION union {
	//	uint32_t characteristics;
	uint32_t originalFirstThunk;
	//} DUMMYUNIONNAME;
	uint32_t timeDateStamp;
	uint32_t forwarderChain;
	uint32_t name;
	uint32_t firstThunk;
} IMAGE_IMPORT_DESCRIPTOR, *PIMAGE_IMPORT_DESCRIPTOR;
#pragma pack()

typedef void (*FARPROC)();

typedef struct _IMAGE_IMPORT_BY_NAME {
	uint16_t hint;
	uint8_t name[1];
} IMAGE_IMPORT_BY_NAME, *PIMAGE_IMPORT_BY_NAME;

typedef struct _IMAGE_THUNK_DATA {
	union {
		uint64_t function;					 // address of imported function
		uint32_t ordinal;					 // ordinal value of function
		PIMAGE_IMPORT_BY_NAME addressOfData; // RVA of imported name
		uint32_t forwarderStringl;			 // RVA to forwarder string
	} u1;
} IMAGE_THUNK_DATA, *PIMAGE_THUNK_DATA;

} // namespace exe
