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

// TODO ntdll should have tofitaStub(){} for unresolved dll imports -> log them

void *offsetPointer(void *data, ptrdiff_t offset) {
	return (void *)((uint64_t)data + offset);
}

struct ImageDataDirectory {
	uint32_t VirtualAddress;
	uint32_t Size;
};

struct PeInterim {
	void *base;
	uint64_t entry; // _DllMainCRTStartup
	const ImageDataDirectory *imageDataDirectory;
};

struct PeExportLinkedList {
	const char8_t *name;
	uint64_t ptr;
	PeExportLinkedList *next;
};

auto loadDll(const char8_t *name, PeExportLinkedList *root) {
	RamDiskAsset asset = getRamDiskAsset(name);
	serialPrintf(u8"[loadDLL] loaded dll asset '%s' %d bytes at %d\n", name, asset.size, asset.data);
	auto ptr = (uint8_t *)asset.data;
	auto peHeader = (const PeHeader *)((uint64_t)ptr + ptr[0x3C] + ptr[0x3C + 1] * 256);
	serialPrintf(u8"PE header signature 'PE' == '%s'\n", peHeader);
	auto peOptionalHeader = (const Pe64OptionalHeader *)((uint64_t)peHeader + sizeof(PeHeader));
	serialPrintf(u8"PE32(+) optional header signature 0x020B == %d == %d\n", peOptionalHeader->mMagic,
				 0x020B);
	serialPrintf(u8"PE32(+) size of image == %d\n", peOptionalHeader->mSizeOfImage);
	void *buffer =
		(void *)PhysicalAllocator::allocatePages(DOWN_BYTES_TO_PAGES(peOptionalHeader->mSizeOfImage) + 1);
	void *base = (void *)buffer;
	memset(base, 0, peOptionalHeader->mSizeOfImage); // Zeroing

	// Copy sections
	auto imageSectionHeader =
		(const ImageSectionHeader *)((uint64_t)peOptionalHeader + peHeader->mSizeOfOptionalHeader);
	for (uint16_t i = 0; i < peHeader->mNumberOfSections; ++i) {
		serialPrintf(u8"Copy section [%d] named '%s' of size %d at %u\n", i, &imageSectionHeader[i].mName,
					 imageSectionHeader[i].mSizeOfRawData, imageSectionHeader[i].mVirtualAddress);
		uint64_t where = (uint64_t)base + imageSectionHeader[i].mVirtualAddress;

		tmemcpy((void *)where,
				(void *)((uint64_t)asset.data + (uint64_t)imageSectionHeader[i].mPointerToRawData),
				imageSectionHeader[i].mSizeOfRawData);
	}

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

	PeExportLinkedList *tail = root;
	while (tail->next != null)
		tail = tail->next;

	for (uint32_t i = 0; i < 15; ++i) {
		if (i == IMAGE_DIRECTORY_ENTRY_EXPORT) {
			uint64_t codeBase = (uint64_t)buffer;
			const ImageDataDirectory *directory = &imageDataDirectory[i];

			PIMAGE_EXPORT_DIRECTORY exports;

			exports = (PIMAGE_EXPORT_DIRECTORY)((uint64_t)buffer + (uint64_t)directory->VirtualAddress);

			uint32_t i = 0;
			uint32_t *nameRef = (uint32_t *)(codeBase + exports->AddressOfNames);
			uint16_t *ordinal = (uint16_t *)(codeBase + exports->AddressOfNameOrdinals);
			uint32_t entry = 0;
			for (i = 0; i < exports->NumberOfNames; i++, nameRef++, ordinal++, entry++) {
				let name = (const char8_t *)(codeBase + (*nameRef));
				let idx = *ordinal;

				let addr = codeBase + exports->AddressOfFunctions + (idx * 4);
				let ptr = (uint32_t *)addr;

				uint32_t (*func)() = (uint32_t(*)())(codeBase + (uint64_t)*ptr);

				PeExportLinkedList *list =
					(PeExportLinkedList *)PhysicalAllocator::allocateBytes(sizeof(PeExportLinkedList));
				list->next = null;
				list->name = name;
				list->ptr = (uint64_t)func;

				tail->next = list;
				tail = tail->next;
			}
		}
	}

	PeInterim pei;

	pei.base = base;
	pei.entry = (uint64_t)buffer + (uint64_t)peOptionalHeader->mAddressOfEntryPoint;
	pei.imageDataDirectory = imageDataDirectory;

	return pei;
}

PeExportLinkedList *getProcAddress(const char8_t *name, PeExportLinkedList *root) {
	PeExportLinkedList *list = root;

	while (list->next != null) {
		// Step upfront, to ignore empty root
		list = list->next;

		uint16_t i = 0;
		while (true) {
			if ((list->name[i] == name[i]) && (name[i] == 0)) {
				serialPrintf(u8"import {%s} resolved to {%s}\n", list->name, name);
				return list;
				break;
			}
			if (list->name[i] == name[i]) {
				i++;
				continue;
			}
			break;
		}
	}

	return null;
}

function resolveDllImports(PeInterim pei, PeExportLinkedList *root) {
	var buffer = pei.base;
	var imageDataDirectory = pei.imageDataDirectory;

	// Imports
	{
		uint8_t *at =
			(uint8_t *)((uint64_t)buffer + imageDataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress);
		auto iid = (IMAGE_IMPORT_DESCRIPTOR *)at;

		// DLL
		while (iid->OriginalFirstThunk != 0) {
			char8_t *szName = (char8_t *)((uint64_t)buffer + iid->Name);
			auto pThunkOrg = (IMAGE_THUNK_DATA *)((uint64_t)buffer + iid->OriginalFirstThunk);
			FARPROC *funcRef;
			funcRef = (FARPROC *)((uint64_t)buffer + iid->FirstThunk);

			while (pThunkOrg->u1.AddressOfData != 0) {
				char8_t *szImportName;
				uint32_t Ord = 666;
				auto fun = (void *)null;

				if ((pThunkOrg->u1.Ordinal & 0x80000000) != 0) {
					Ord = pThunkOrg->u1.Ordinal & 0xffff;
					serialPrintf(u8"import {%s}.@%d - at address: {%d} <------------ NOT IMPLEMENTED YET!\n",
								 szName, Ord, pThunkOrg->u1.Function);
				} else {
					IMAGE_IMPORT_BY_NAME *pIBN =
						(IMAGE_IMPORT_BY_NAME *)((uint64_t)buffer +
												 (uint64_t)((uint64_t)pThunkOrg->u1.AddressOfData &
															0xffffffff));
					Ord = pIBN->Hint;
					szImportName = (char8_t *)pIBN->Name;
					serialPrintf(u8"import {%s}.{%s}@%d - at address: {%d}\n", szName, szImportName, Ord,
								 pThunkOrg->u1.Function);

					// Resolve import
					fun = null;

					PeExportLinkedList *proc = getProcAddress(szImportName, root);

					if (proc != null)
						fun = (void *)proc->ptr;
				}

				*funcRef = (FARPROC)fun;
				pThunkOrg++;
				funcRef++;
			}

			iid++;
		}
	}
}

function simpleExeTest() {
	{
		PeExportLinkedList *root =
			(PeExportLinkedList *)PhysicalAllocator::allocateBytes(sizeof(PeExportLinkedList));
		root->next = null;
		root->name = null;
		root->ptr = 0;

		auto a = loadDll(u8"desktop/dllimp.exe", root);
		auto b = loadDll(u8"desktop/dll2.dll", root);
		auto c = loadDll(u8"desktop/dll1.dll", root);

		resolveDllImports(a, root);
		resolveDllImports(b, root);
		resolveDllImports(c, root);
	}

}

} // namespace exe
