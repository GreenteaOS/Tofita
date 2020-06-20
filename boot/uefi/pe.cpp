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
struct Pe32OptionalHeader {
	uint16_t mMagic; // 0x010b - PE32, 0x020b - PE32+ (64 bit)
	uint8_t mMajorLinkerVersion;
	uint8_t mMinorLinkerVersion;
	uint32_t mSizeOfCode;
	uint32_t mSizeOfInitializedData;
	uint32_t mSizeOfUninitializedData;
	uint32_t mAddressOfEntryPoint;
	uint32_t mBaseOfCode;
	// uint32_t mBaseOfData; // present only in 32-bit
	uint64_t mImageBase; // uint32_t mImageBase;
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
	uint64_t mSizeOfStackReserve; // uint32_t mSizeOfStackReserve;
	uint64_t mSizeOfStackCommit;  // uint32_t mSizeOfStackCommit;
	uint64_t mSizeOfHeapReserve;  // uint32_t mSizeOfHeapReserve;
	uint64_t mSizeOfHeapCommit;	  // uint32_t mSizeOfHeapCommit;
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
