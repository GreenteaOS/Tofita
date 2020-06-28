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
	uint32_t mMagic; // PE\0\0 or 0x00004550
	uint16_t mMachine;
	uint16_t mNumberOfSections;
	uint32_t mTimeDateStamp;
	uint32_t mPointerToSymbolTable;
	uint32_t mNumberOfSymbols;
	uint16_t mSizeOfOptionalHeader;
	uint16_t mCharacteristics;
} __attribute__((packed));
#pragma pack()

_Static_assert(sizeof(PeHeader) == 24, "sizeof is incorrect");

// 1 byte aligned
#pragma pack(1)
struct Pe64OptionalHeader {
	uint16_t mMagic; // 0x010b - PE32, 0x020b - PE32+ (64 bit)
	uint8_t mMajorLinkerVersion;
	uint8_t mMinorLinkerVersion;
	uint32_t mSizeOfCode;
	uint32_t mSizeOfInitializedData;
	uint32_t mSizeOfUninitializedData;
	uint32_t mAddressOfEntryPoint;
	uint32_t mBaseOfCode;
	uint64_t mImageBase;
	uint32_t mSectionAlignment;
	uint32_t mFileAlignment;
	uint16_t mMajorOperatingSystemVersion;
	uint16_t mMinorOperatingSystemVersion;
	uint16_t mMajorImageVersion;
	uint16_t mMinorImageVersion;
	uint16_t mMajorSubsystemVersion;
	uint16_t mMinorSubsystemVersion;
	uint32_t mWin32VersionValue;
	uint32_t mSizeOfImage;
	uint32_t mSizeOfHeaders;
	uint32_t mCheckSum;
	uint16_t mSubsystem;
	uint16_t mDllCharacteristics;
	uint64_t mSizeOfStackReserve;
	uint64_t mSizeOfStackCommit;
	uint64_t mSizeOfHeapReserve;
	uint64_t mSizeOfHeapCommit;
	uint32_t mLoaderFlags;
	uint32_t mNumberOfRvaAndSizes;
} __attribute__((packed));
#pragma pack()

// 1 byte aligned
#pragma pack(1)
struct Pe32OptionalHeader {
	uint16_t mMagic; // 0x010b - PE32, 0x020b - PE32+ (64 bit)
	uint8_t mMajorLinkerVersion;
	uint8_t mMinorLinkerVersion;
	uint32_t mSizeOfCode;
	uint32_t mSizeOfInitializedData;
	uint32_t mSizeOfUninitializedData;
	uint32_t mAddressOfEntryPoint;
	uint32_t mBaseOfCode;
	uint32_t mBaseOfData; // present only in 32-bit
	uint32_t mImageBase;
	uint32_t mSectionAlignment;
	uint32_t mFileAlignment;
	uint16_t mMajorOperatingSystemVersion;
	uint16_t mMinorOperatingSystemVersion;
	uint16_t mMajorImageVersion;
	uint16_t mMinorImageVersion;
	uint16_t mMajorSubsystemVersion;
	uint16_t mMinorSubsystemVersion;
	uint32_t mWin32VersionValue;
	uint32_t mSizeOfImage;
	uint32_t mSizeOfHeaders;
	uint32_t mCheckSum;
	uint16_t mSubsystem;
	uint16_t mDllCharacteristics;
	uint32_t mSizeOfStackReserve;
	uint32_t mSizeOfStackCommit;
	uint32_t mSizeOfHeapReserve;
	uint32_t mSizeOfHeapCommit;
	uint32_t mLoaderFlags;
	uint32_t mNumberOfRvaAndSizes;
} __attribute__((packed));
#pragma pack()

#pragma pack(1)
struct ImageSectionHeader { // size 40 bytes
	uint8_t mName[8];
	uint32_t mVirtualSize;
	uint32_t mVirtualAddress;
	uint32_t mSizeOfRawData;
	uint32_t mPointerToRawData;
	uint32_t mPointerToRealocations;
	uint32_t mPointerToLinenumbers;
	uint16_t mNumberOfRealocations;
	uint16_t mNumberOfLinenumbers;
	uint32_t mCharacteristics;
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
	uint32_t Characteristics;
	uint32_t TimeDateStamp;
	uint16_t MajorVersion;
	uint16_t MinorVersion;
	uint32_t Name;
	uint32_t Base;
	uint32_t NumberOfFunctions;
	uint32_t NumberOfNames;
	uint32_t AddressOfFunctions;
	uint32_t AddressOfNames;
	uint32_t AddressOfNameOrdinals;
} IMAGE_EXPORT_DIRECTORY, *PIMAGE_EXPORT_DIRECTORY;

typedef struct _IMAGE_BASE_RELOCATION {
	uint32_t VirtualAddress;
	uint32_t SizeOfBlock;
} IMAGE_BASE_RELOCATION, *PIMAGE_BASE_RELOCATION;

typedef struct _IMAGE_DATA_DIRECTORY {
	uint32_t VirtualAddress;
	uint32_t Size;
} IMAGE_DATA_DIRECTORY, *PIMAGE_DATA_DIRECTORY;
#define IMAGE_SIZEOF_BASE_RELOCATION 8
#define IMAGE_REL_BASED_ABSOLUTE 0
#define IMAGE_REL_BASED_DIR64 10
#define IMAGE_REL_BASED_HIGHLOW 3

} // namespace exe
