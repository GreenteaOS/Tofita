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

// Loads EXE and DLL files
// TODO 32-bit loader

namespace exe {

__attribute__((aligned(64))) uint8_t buffer[16384 * 8] = {0};

void *offsetPointer(void *data, ptrdiff_t offset) {
	return (void *)((uint64_t)data + offset);
}

function loadDLL(const char8_t *name) {
	RamDiskAsset asset = getRamDiskAsset(name);
	serialPrintf(u8"[loadDLL] loaded dll asset '%s' %d bytes at %d\n", name, asset.size, asset.data);
	auto ptr = (uint8_t *)asset.data;
	auto peHeader = (const PeHeader *)((uint64_t)ptr + ptr[0x3C] + ptr[0x3C + 1] * 256);
	serialPrintf(u8"PE header signature 'PE' == '%s'\n", peHeader);
	auto peOptionalHeader = (const Pe64OptionalHeader *)((uint64_t)peHeader + sizeof(PeHeader));
	serialPrintf(u8"PE32(+) optional header signature 0x020B == %d == %d\n", peOptionalHeader->mMagic,
				 0x020B);
	serialPrintf(u8"PE32(+) size of image == %d\n", peOptionalHeader->mSizeOfImage);
	void *base = (void *)buffer;
	memset(base, 0, peOptionalHeader->mSizeOfImage); // Zeroing

	// Copy sections
	auto imageSectionHeader =
		(const ImageSectionHeader *)((uint64_t)peOptionalHeader + peHeader->mSizeOfOptionalHeader);
	for (uint16_t i = 0; i < peHeader->mNumberOfSections; ++i) {
		serialPrintf(u8"Copy section [%d] named '%s' of size %d\n", i, &imageSectionHeader[i].mName,
					 imageSectionHeader[i].mSizeOfRawData);
		uint64_t where = (uint64_t)base + imageSectionHeader[i].mVirtualAddress;

		tmemcpy((void *)where,
				(void *)((uint64_t)asset.data + (uint64_t)imageSectionHeader[i].mPointerToRawData),
				imageSectionHeader[i].mSizeOfRawData);
	}

	struct ImageDataDirectory {
		uint32_t VirtualAddress;
		uint32_t Size;
	};
	auto imageDataDirectory =
		(const ImageDataDirectory *)((uint64_t)peOptionalHeader + sizeof(Pe64OptionalHeader));

	// Relocate
	ptrdiff_t delta = (uint64_t)buffer - (uint64_t)peOptionalHeader->mImageBase;
	if (delta != 0) {
		uint8_t *codeBase = (uint8_t *)buffer;

		PIMAGE_BASE_RELOCATION relocation;

		PIMAGE_DATA_DIRECTORY directory =
			(PIMAGE_DATA_DIRECTORY)&imageDataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC];
		if (directory->Size == 0) {
			// return (delta == 0);
		}

		relocation = (PIMAGE_BASE_RELOCATION)(codeBase + directory->VirtualAddress);
		for (; relocation->VirtualAddress > 0;) {
			uint32_t i = 0;
			uint8_t *dest = codeBase + relocation->VirtualAddress;
			uint16_t *relInfo = (uint16_t *)offsetPointer(relocation, IMAGE_SIZEOF_BASE_RELOCATION);
			for (i = 0; i < ((relocation->SizeOfBlock - IMAGE_SIZEOF_BASE_RELOCATION) / 2); i++, relInfo++) {
				// the upper 4 bits define the type of relocation
				int32_t type = *relInfo >> 12;
				// the lower 12 bits define the offset
				int32_t offset = *relInfo & 0xfff;

				switch (type) {
				case IMAGE_REL_BASED_ABSOLUTE:
					// skip relocation
					break;

				case IMAGE_REL_BASED_HIGHLOW:
					// change complete 32 bit address
					{
						uint32_t *patchAddrHL = (uint32_t *)(dest + offset);
						*patchAddrHL += (uint32_t)delta;
					}
					break;

					//#ifdef _WIN64
				case IMAGE_REL_BASED_DIR64: {
					uint64_t *patchAddr64 = (uint64_t *)(dest + offset);
					*patchAddr64 += (uint64_t)delta;
				} break;
					//#endif

				default:
					break;
				}
			}

			// advance to next relocation block
			relocation = (PIMAGE_BASE_RELOCATION)offsetPointer(relocation, relocation->SizeOfBlock);
		}
	}

	// Exports

	for (uint32_t i = 0; i < 15; ++i) {
		if (i == IMAGE_DIRECTORY_ENTRY_EXPORT) {
			uint64_t codeBase = (uint64_t)buffer;
			const ImageDataDirectory *directory = &imageDataDirectory[i];

			PIMAGE_EXPORT_DIRECTORY exports;

			exports = (PIMAGE_EXPORT_DIRECTORY)((uint64_t)buffer + (uint64_t)directory->VirtualAddress);

			uint32_t i;
			uint32_t *nameRef = (uint32_t *)(codeBase + exports->AddressOfNames);
			uint16_t *ordinal = (uint16_t *)(codeBase + exports->AddressOfNameOrdinals);
			uint32_t entry = 0;
			for (i = 0; i < exports->NumberOfNames; i++, nameRef++, ordinal++, entry++) {
				let name = (const char8_t *)(codeBase + (*nameRef));
				let idx = *ordinal;

				let addr = codeBase + exports->AddressOfFunctions + (idx * 4);
				let ptr = (uint32_t *)addr;

				uint32_t (*func)() = (uint32_t(*)())(codeBase + (uint64_t)*ptr);
			}
		}
	}
}

function simpleExeTest() {
	RamDiskAsset asset = getRamDiskAsset(u8"mexe.exe");
	serialPrintf(u8"[[[efi_main]]] loaded asset '_.exe' %d bytes at %d\n", asset.size, asset.data);

	uint64_t mAddressOfEntryPoint = 0;
	auto ptr = (uint8_t*)asset.data;
	auto peHeader = (const PeHeader*)((uint64_t)ptr + ptr[0x3C] + ptr[0x3C + 1] * 256);
	serialPrintf(u8"PE header signature 'PE' == '%s'\n", peHeader);
	auto peOptionalHeader = (const Pe32OptionalHeader*)((uint64_t)peHeader + sizeof(PeHeader));
	serialPrintf(u8"PE32(+) optional header signature 0x020B == %d == %d\n", peOptionalHeader->mMagic, 0x020B);
	serialPrintf(u8"PE32(+) size of image == %d\n", peOptionalHeader->mSizeOfImage);
	void *base = (void *) buffer;
	memset(base, 0, peOptionalHeader->mSizeOfImage); // Zeroing

	// Copy sections
	auto imageSectionHeader = (const ImageSectionHeader*)((uint64_t)peOptionalHeader + peHeader->mSizeOfOptionalHeader);
	for (uint16_t i = 0; i < peHeader->mNumberOfSections; ++i) {
		serialPrintf(u8"Copy section [%d] named '%s' of size %d\n", i, &imageSectionHeader[i].mName, imageSectionHeader[i].mSizeOfRawData);
		uint64_t where = (uint64_t)base + imageSectionHeader[i].mVirtualAddress;

		tmemcpy(
			(void *)where,
			(void *)((uint64_t)asset.data + (uint64_t)imageSectionHeader[i].mPointerToRawData),
			imageSectionHeader[i].mSizeOfRawData
		);
	}

	mAddressOfEntryPoint = peOptionalHeader->mAddressOfEntryPoint;

	mAddressOfEntryPoint += (uint64_t)base;

	let entry = (Entry)mAddressOfEntryPoint;

	serialPrintf(u8"entry begin...\n", 0);

	entry();
	entry();
	entry();

	serialPrintf(u8"entry recheck...\n", 0);

	entry();
	entry();
	entry();

	serialPrintf(u8"entry end...\n", 0);
}

}
