#include "exe.hpp"

namespace exe {

uint8_t buffer[16384] = {0};
typedef function (__fastcall *Entry)();

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
	for (int i = 0; i < peHeader->mNumberOfSections; ++i) {
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
