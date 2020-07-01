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

typedef function(__fastcall *Entry)(uint64_t entry);

void *offsetPointer(void *data, ptrdiff_t offset) {
	return (void *)((uint64_t)data + offset);
}

struct ImageDataDirectory {
	uint32_t virtualAddress;
	uint32_t size;
};

struct PeInterim {
	void *base;
	uint64_t entry; // _DllMainCRTStartup
	const ImageDataDirectory *imageDataDirectory;
	uint64_t sizeOfStackReserve;
};

struct ExeInterim {
	PeInterim pei;
	uint64_t stackVirtual;
};

struct PeExportLinkedList {
	const char8_t *name;
	uint64_t ptr;
	PeExportLinkedList *next;
};

struct Executable {
	uint64_t nextBase;
	pages::PageEntry *pml4;
};

auto loadDll(const char8_t *name, PeExportLinkedList *root, Executable *exec) {
	RamDiskAsset asset = getRamDiskAsset(name);
	serialPrintf(u8"[loadDLL] loaded dll asset '%s' %d bytes at %d\n", name, asset.size, asset.data);
	auto ptr = (uint8_t *)asset.data;
	auto peHeader = (const PeHeader *)((uint64_t)ptr + ptr[0x3C] + ptr[0x3C + 1] * 256);
	serialPrintf(u8"[loadDLL] PE header signature 'PE' == '%s'\n", peHeader);
	auto peOptionalHeader = (const Pe64OptionalHeader *)((uint64_t)peHeader + sizeof(PeHeader));
	serialPrintf(u8"[loadDLL] PE32(+) optional header signature 0x020B == %d == %d\n",
				 peOptionalHeader->magic, 0x020B);
	serialPrintf(u8"[loadDLL] PE32(+) size of image == %d\n", peOptionalHeader->sizeOfImage);

	let pages = DOWN_BYTES_TO_PAGES(peOptionalHeader->sizeOfImage) + 1;
	let physical = PhysicalAllocator::allocatePages(pages);

	if (exec->nextBase == 0) {
		// TODO round to pages?
		exec->nextBase = peOptionalHeader->imageBase;
	}

	pages::mapMemory(exec->pml4, exec->nextBase, physical - (uint64_t)WholePhysicalStart, pages);

	const uint64_t buffer = exec->nextBase;
	memset((void *)buffer, 0, peOptionalHeader->sizeOfImage); // Zeroing

	exec->nextBase = exec->nextBase + pages * PAGE_SIZE;

	// Copy sections
	auto imageSectionHeader =
		(const ImageSectionHeader *)((uint64_t)peOptionalHeader + peHeader->sizeOfOptionalHeader);

	// TODO copy PE headers?
	for (uint16_t i = 0; i < peHeader->numberOfSections; ++i) {
		serialPrintf(u8"[loadDLL] Copy section [%d] named '%s' of size %d at %u\n", i,
					 &imageSectionHeader[i].name, imageSectionHeader[i].sizeOfRawData,
					 imageSectionHeader[i].virtualAddress);
		uint64_t where = (uint64_t)buffer + imageSectionHeader[i].virtualAddress;

		tmemcpy((void *)where,
				(void *)((uint64_t)asset.data + (uint64_t)imageSectionHeader[i].pointerToRawData),
				imageSectionHeader[i].sizeOfRawData);
	}

	auto imageDataDirectory =
		(const ImageDataDirectory *)((uint64_t)peOptionalHeader + sizeof(Pe64OptionalHeader));

	// Relocate EXE or DLL to a new base
	ptrdiff_t delta = (uint64_t)buffer - (uint64_t)peOptionalHeader->imageBase;
	if (delta != 0) {
		uint8_t *codeBase = (uint8_t *)buffer;

		PIMAGE_BASE_RELOCATION relocation;

		PIMAGE_DATA_DIRECTORY directory =
			(PIMAGE_DATA_DIRECTORY)&imageDataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC];

		if (directory->size == 0) {
			// return (delta == 0);
		}

		relocation = (PIMAGE_BASE_RELOCATION)(codeBase + directory->virtualAddress);
		for (; relocation->virtualAddress > 0;) {
			uint32_t i = 0;
			uint8_t *dest = codeBase + relocation->virtualAddress;
			uint16_t *relInfo = (uint16_t *)offsetPointer(relocation, IMAGE_SIZEOF_BASE_RELOCATION);
			for (i = 0; i < ((relocation->sizeOfBlock - IMAGE_SIZEOF_BASE_RELOCATION) / 2); i++, relInfo++) {
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
					serialPrintf(u8"[loadDLL] Unknown relocation: %d\n", type);
					break;
				}
			}

			// advance to next relocation block
			relocation = (PIMAGE_BASE_RELOCATION)offsetPointer(relocation, relocation->sizeOfBlock);
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

			exports = (PIMAGE_EXPORT_DIRECTORY)((uint64_t)buffer + (uint64_t)directory->virtualAddress);

			uint32_t i = 0;
			uint32_t *nameRef = (uint32_t *)(codeBase + exports->addressOfNames);
			uint16_t *ordinal = (uint16_t *)(codeBase + exports->addressOfNameOrdinals);
			uint32_t entry = 0;
			for (i = 0; i < exports->numberOfNames; i++, nameRef++, ordinal++, entry++) {
				let name = (const char8_t *)(codeBase + (*nameRef));
				let idx = *ordinal;

				let addr = codeBase + exports->addressOfFunctions + (idx * 4);
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

	pei.base = (void *)buffer;
	pei.entry = (uint64_t)buffer + (uint64_t)peOptionalHeader->addressOfEntryPoint;
	pei.imageDataDirectory = imageDataDirectory;
	pei.sizeOfStackReserve = peOptionalHeader->sizeOfStackReserve;

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
				serialPrintf(u8"[getProcAddress] import {%s} resolved to {%s}\n", list->name, name);
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

	serialPrintf(u8"[getProcAddress] import {%s} unresolved\n", list->name);
	return null;
}

function resolveDllImports(PeInterim pei, PeExportLinkedList *root) {
	var buffer = pei.base;
	var imageDataDirectory = pei.imageDataDirectory;

	// Imports
	{
		uint8_t *at =
			(uint8_t *)((uint64_t)buffer + imageDataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].virtualAddress);
		auto iid = (IMAGE_IMPORT_DESCRIPTOR *)at;

		// DLL
		while (iid->originalFirstThunk != 0) {
			char8_t *szName = (char8_t *)((uint64_t)buffer + iid->name);
			auto pThunkOrg = (IMAGE_THUNK_DATA *)((uint64_t)buffer + iid->originalFirstThunk);
			FARPROC *funcRef;
			funcRef = (FARPROC *)((uint64_t)buffer + iid->firstThunk);

			while (pThunkOrg->u1.addressOfData != 0) {
				char8_t *szImportName;
				uint32_t ord = 666;
				auto fun = (void *)null;

				if ((pThunkOrg->u1.ordinal & 0x80000000) != 0) {
					ord = pThunkOrg->u1.ordinal & 0xffff;
					serialPrintf(u8"[resolveDllImports] import {%s}.@%d - at address: {%d} <------------ NOT "
								 u8"IMPLEMENTED YET!\n",
								 szName, ord, pThunkOrg->u1.function);
				} else {
					IMAGE_IMPORT_BY_NAME *pIBN =
						(IMAGE_IMPORT_BY_NAME *)((uint64_t)buffer +
												 (uint64_t)((uint64_t)pThunkOrg->u1.addressOfData &
															0xffffffff));
					ord = pIBN->hint;
					szImportName = (char8_t *)pIBN->name;
					serialPrintf(u8"[resolveDllImports] import {%s}.{%s}@%d - at address: {%d}\n", szName,
								 szImportName, ord, pThunkOrg->u1.function);

					// Resolve import
					fun = null;

					PeExportLinkedList *proc = getProcAddress(szImportName, root);

					if (proc != null) {
						fun = (void *)proc->ptr;
					} else
						fun = (void *)getProcAddress(u8"tofitaFastStub", root)->ptr;
				}

				*funcRef = (FARPROC)fun;
				pThunkOrg++;
				funcRef++;
			}

			iid++;
		}
	}
}

auto loadExe(const char8_t *name, PeExportLinkedList *root, Executable *exec) {
	ExeInterim ei;

	ei.pei = loadDll(name, root, exec);

	// Allocate stack
	{
		let pages = DOWN_BYTES_TO_PAGES(ei.pei.sizeOfStackReserve) + 1;
		let physical = PhysicalAllocator::allocatePages(pages);

		const uint64_t buffer = exec->nextBase;
		pages::mapMemory(exec->pml4, exec->nextBase, physical - (uint64_t)WholePhysicalStart, pages);
		memset((void *)buffer, 0, ei.pei.sizeOfStackReserve); // Zeroing

		exec->nextBase = exec->nextBase + pages * PAGE_SIZE;

		ei.stackVirtual = buffer;
	}

	return ei;
}

function resolveExeImports(const ExeInterim ei, PeExportLinkedList *root) {
	resolveDllImports(ei.pei, root);
}

function loadExeIntoProcess(const char8_t *file, process::Process *process) {
	{
		PeExportLinkedList *root =
			(PeExportLinkedList *)PhysicalAllocator::allocateBytes(sizeof(PeExportLinkedList));
		root->next = null;
		root->name = null;
		root->ptr = 0;

		Executable exec;
		exec.nextBase = 0;
		exec.pml4 = process->pml4;

		auto app = loadExe(file, root, &exec);
		auto ntdll = loadDll(u8"desktop/ntdll.dll", root, &exec);
		auto kernel32 = loadDll(u8"desktop/kernel32.dll", root, &exec);
		auto gdi32 = loadDll(u8"desktop/gdi32.dll", root, &exec);
		auto user32 = loadDll(u8"desktop/user32.dll", root, &exec);

		resolveDllImports(ntdll, root);
		resolveDllImports(kernel32, root);
		resolveDllImports(gdi32, root);
		resolveDllImports(user32, root);
		resolveExeImports(app, root);

		Entry entry = (Entry)ntdll.base;
		process->frame.ip = ntdll.entry; // Contains crt0
		process->frame.sp = app.stackVirtual;
		process->stack.rcx = app.pei.entry; // First argument
	}

}

} // namespace exe
