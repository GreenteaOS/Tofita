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

extern "C++" {
namespace exe {

// TODO ntdll should have tofitaStub(){} for unresolved dll imports -> log them

typedef function(__fastcall *Entry)(uint64_t entry);

void *offsetPointer(const void *data, ptrdiff_t offset) {
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
	bool is64bit;
};

struct ExeInterim {
	PeInterim pei;
	uint64_t stackVirtual;
};

struct PeExportLinkedList {
	const char8_t *name;
	uint64_t ptr;
	PeExportLinkedList *next;
	uint8_t hash;
};

struct Executable {
	uint64_t nextBase;
	pages::PageEntry *pml4;
};

// TODO probe for read/write
template<typename SIZE, typename HEADER>
auto loadDllAsset(RamDiskAsset asset, const wchar_t *name, PeExportLinkedList *root, Executable *exec) {
	auto ptr = (uint8_t *)asset.data;
	auto peHeader = (const PeHeader *)((uint64_t)ptr + ptr[0x3C] + ptr[0x3C + 1] * 256);
	auto peOptionalHeader = (const HEADER *)((uint64_t)peHeader + sizeof(PeHeader));

	serialPrintf(L"[loadDLL] PE32(+) size of image == %d\n", peOptionalHeader->sizeOfImage);

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
		serialPrintf(L"[loadDLL] Copy section [%d] named '%s' of size %d at %u\n", i,
					 &imageSectionHeader[i].name, imageSectionHeader[i].sizeOfRawData,
					 imageSectionHeader[i].virtualAddress);
		uint64_t where = (uint64_t)buffer + imageSectionHeader[i].virtualAddress;

		tmemcpy((void *)where,
				(void *)((uint64_t)asset.data + (uint64_t)imageSectionHeader[i].pointerToRawData),
				imageSectionHeader[i].sizeOfRawData);
	}

	// TODO
	{
		uint32_t (*func)() =
			(uint32_t(*)())((uint64_t)buffer + (uint64_t)peOptionalHeader->addressOfEntryPoint);
		if (peOptionalHeader->addressOfEntryPoint == 4128)
			serialPrintf(L"[[[IMAGE_DIRECTORY_ENTRY_EXPORT]]] calling _DllMainCRTStartup == %d\n", func());
	}

	auto imageDataDirectory =
		(const ImageDataDirectory *)((uint64_t)peOptionalHeader + sizeof(HEADER));

	// Relocate EXE or DLL to a new base
	ptrdiff_t delta = (uint64_t)buffer - (uint64_t)peOptionalHeader->imageBase;
	if (delta != 0) {
		uint8_t *codeBase = (uint8_t *)buffer;

		const IMAGE_BASE_RELOCATION *relocation;

		auto directory =
			(const IMAGE_DATA_DIRECTORY*)&imageDataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC];

		if (directory->size == 0) {
			// return (delta == 0);
		}

		relocation = (const IMAGE_BASE_RELOCATION*)(codeBase + directory->virtualAddress);
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
					// TODO x32
				case IMAGE_REL_BASED_DIR64: {
					uint64_t *patchAddr64 = (uint64_t *)(dest + offset);
					*patchAddr64 += (uint64_t)delta;
				} break;
					//#endif

				default:
					serialPrintf(L"[loadDLL] Unknown relocation: %d\n", type);
					break;
				}
			}

			// advance to next relocation block
			relocation = (const IMAGE_BASE_RELOCATION*)offsetPointer(relocation, relocation->sizeOfBlock);
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

			const IMAGE_EXPORT_DIRECTORY *exports;

			exports = (const IMAGE_EXPORT_DIRECTORY*)((uint64_t)buffer + (uint64_t)directory->virtualAddress);

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
	pei.is64bit = sizeof(SIZE) == sizeof(uint64_t);

	return pei;
}

auto loadDll(const wchar_t *name, PeExportLinkedList *root, Executable *exec) {
	RamDiskAsset asset = getRamDiskAsset(name);
	serialPrintf(L"[loadDLL] loaded dll asset '%S' %d bytes at %d\n", name, asset.size, asset.data);

	auto ptr = (uint8_t *)asset.data;
	auto peHeader = (const PeHeader *)((uint64_t)ptr + ptr[0x3C] + ptr[0x3C + 1] * 256);
	serialPrintf(L"[loadDLL] PE header signature 'PE' == '%s'\n", peHeader);

	auto peOptionalHeader = (const Pe64OptionalHeader *)((uint64_t)peHeader + sizeof(PeHeader));

	serialPrintf(L"[loadDLL] PE32(+) optional header signature 0x020B == %d == %d\n", peOptionalHeader->magic,
				 0x020B);

	auto is64bit = peOptionalHeader->magic == 0x020B;

	if (is64bit) {
		return loadDllAsset<uint64_t, Pe64OptionalHeader>(asset, name, root, exec);
	} else {
		return loadDllAsset<uint32_t, Pe32OptionalHeader>(asset, name, root, exec);
	}
}

// TODO for system dlls (ntdll, user32, etc) search EVERY dll for the name, cause they
// may be moved between versions.
// Search only selected .dll only within the user provided dlls
PeExportLinkedList *getProcAddress(const char8_t *name, PeExportLinkedList *root) {
	PeExportLinkedList *list = root;
	// TODO compute crc for faster .dll import names lookup
	while (list->next != null) {
		// Step upfront, to ignore empty root
		list = list->next;

		uint16_t i = 0;
		while (true) {
			if ((list->name[i] == name[i]) && (name[i] == 0)) {
				serialPrintf(L"[getProcAddress] import {%s} resolved to {%s}\n", name, list->name);
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

	serialPrintf(L"[getProcAddress] import {%s} unresolved\n", name);
	return null;
}

template<typename SIZE>
function resolveDllImports(PeInterim pei, PeExportLinkedList *root) {
	var buffer = pei.base;
	var imageDataDirectory = pei.imageDataDirectory;
	let is64bit = pei.is64bit;

	// Imports
	{
		uint8_t *at =
			(uint8_t *)((uint64_t)buffer + imageDataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].virtualAddress);
		auto iid = (const IMAGE_IMPORT_DESCRIPTOR *)at;

		// DLL
		while (iid->originalFirstThunk != 0) {
			char8_t *szName = (char8_t *)((uint64_t)buffer + iid->name);
			auto pThunkOrg = (const IMAGE_THUNK_DATA<SIZE> *)((uint64_t)buffer + iid->originalFirstThunk);
			auto funcRef = (SIZE *)((uint64_t)buffer + iid->firstThunk);

			while (pThunkOrg->u1.addressOfData != 0) {
				char8_t *szImportName;
				uint32_t ord = 666;
				SIZE func = 0;

				if ((pThunkOrg->u1.ordinal & 0x80000000) != 0) {
					ord = pThunkOrg->u1.ordinal & 0xffff;
					serialPrintf(L"[resolveDllImports] import {%s}.@%d - at address: {%d} <------------ NOT "
								 L"IMPLEMENTED YET!\n",
								 szName, ord, pThunkOrg->u1.function);
				} else {
					const IMAGE_IMPORT_BY_NAME *pIBN =
						(const IMAGE_IMPORT_BY_NAME *)((uint64_t)buffer +
												 (uint64_t)((uint64_t)pThunkOrg->u1.addressOfData &
															0xffffffff));
					ord = pIBN->hint;
					szImportName = (char8_t *)pIBN->name;
					serialPrintf(L"[resolveDllImports] import {%s}.{%s}@%d - at address: {%d}\n", szName,
								 szImportName, ord, pThunkOrg->u1.function);

					// Resolve import
					func = 0;

					PeExportLinkedList *proc = getProcAddress(szImportName, root);

					if (proc != null) {
						func = (SIZE)proc->ptr;
					} else
						func = (SIZE)getProcAddress(
							is64bit? u8"tofitaFastStub" : u8"tofitaStdStub",
							root
						)->ptr;
				}

				*funcRef = (SIZE)func;
				pThunkOrg++;
				funcRef++;
			}

			iid++;
		}
	}
}

auto loadExe(const wchar_t *name, PeExportLinkedList *root, Executable *exec) {
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

	// Allocate PEB
	{
		let pages = 1;
		let physical = PhysicalAllocator::allocateOnePagePreZeroed();
		let where = (uint64_t)0x00000000FFFAE000; // From GDT
		// TODO TEB 000007FF`FFFAE000 on 64-bit?

		pages::mapMemory(exec->pml4, where, physical - (uint64_t)WholePhysicalStart, pages);
	}

	return ei;
}

template<typename SIZE>
function resolveExeImports(const ExeInterim ei, PeExportLinkedList *root) {
	resolveDllImports<SIZE>(ei.pei, root);
}

function loadExeIntoProcess(const wchar_t *file, process::Process *process) {
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
		auto is64bit = app.pei.is64bit;

		// Dependencies
		// TODO proper system path!!! + Windows 7 order of search
		// TODO respect ENV PATH
		#define BIT32 L"C:\\Windows\\SysWOW64\\"
		#define BIT64 L"C:\\Windows\\System32\\"
		#define BIT(DLL) (is64bit?BIT64 DLL:BIT32 DLL)

		auto ntdll = loadDll(BIT(L"ntdll.dll"), root, &exec);
		auto kernel32 = loadDll(BIT(L"kernel32.dll"), root, &exec);
		auto gdi32 = loadDll(BIT(L"gdi32.dll"), root, &exec);
		auto user32 = loadDll(BIT(L"user32.dll"), root, &exec);

		#undef BIT32
		#undef BIT64
		#undef BIT

		if (is64bit) {
			resolveDllImports<uint64_t>(ntdll, root);
			resolveDllImports<uint64_t>(kernel32, root);
			resolveDllImports<uint64_t>(gdi32, root);
			resolveDllImports<uint64_t>(user32, root);
			resolveExeImports<uint64_t>(app, root);

			process->is64bit = true;
			process->frame.cs = USER_CODE64_SEL + 3;
		} else {
			resolveDllImports<uint32_t>(ntdll, root);
			resolveDllImports<uint32_t>(kernel32, root);
			resolveDllImports<uint32_t>(gdi32, root);
			resolveDllImports<uint32_t>(user32, root);
			resolveExeImports<uint32_t>(app, root);

			process->is64bit = false;
			// TODO IMAGE_FILE_LARGE_ADDRESS_AWARE
			// TODO IMAGE_FILE_LARGE_ADDRESS_AWARE with compatibility mode (3 GB, 128 TB)
			process->limits = pages::AddressAwareness::Bit32limit2GB;
			process->frame.cs = USER_CODE32_SEL + 3;
		}

		Entry entry = (Entry)ntdll.base;
		process->frame.ip = ntdll.entry; // Contains crt0
		process->frame.sp = app.stackVirtual;
		process->frame.rcxArg0 = app.pei.entry; // First argument
	}
}

} // namespace exe

}
