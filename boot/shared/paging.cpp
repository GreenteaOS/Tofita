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

namespace paging {

#define ADDRESS_BITS 12
#define PAGE_ALIGNED __attribute__((aligned(PAGE_SIZE)))
#define PACKED __attribute__((gcc_struct, packed))

struct LinearAddress {
	unsigned long long offset : 12;
	unsigned long long pt : 9;
	unsigned long long pd : 9;
	unsigned long long pdpt : 9;
	unsigned long long pml4 : 9;
	unsigned long long reserved : 16;
} __attribute__((gcc_struct, packed));

_Static_assert(sizeof(uint64_t) == 8, "uint64_t has to have 64 bits");
_Static_assert(sizeof(paging::LinearAddress) == 8, "linear address has to have 64 bits");

// 512 entries * 8 bytes = 4 KiB
// PML4 size = PDP size = PD size = PT size
//
// Since we are using 64 bit mode, the entry size
// is 64 bits.

#define PAGE_TABLE_SIZE 512

// Entry in a page table
typedef struct {
	// Is the page present in physical memory?
	unsigned long long present : 1;

	// Pages are read-only by default
	unsigned long long writeAllowed : 1;

	// Pages are only accessible by supervisor by default
	unsigned long long accessibleByAll : 1;

	// Write through abilities of the page
	unsigned long long writeThrough : 1;

	// If set, the page will not be cached
	unsigned long long cacheDisabled : 1;

	// Was the page accessed?
	unsigned long long accessed : 1;

	// Has the page been written to? Only applicable for PTE.
	unsigned long long dirty : 1;

	// Page size by default is small, enabling this bit makes it bigger. Only applicable for PTE
	unsigned long long largePage : 1;

	// Prevent the translations cache from updating
	// the address in cache if CR3 is reset.
	//
	// Page global enable bit in CR4 has to be set to
	// enable this feature.
	//
	// Only applicable for PTE
	unsigned long long global : 1;

	// Not used by the processor
	unsigned long long metadata : 3;

	// Physical address of the child table/page
	unsigned long long address : 40;

	// Not used by the processor
	unsigned long long metadata2 : 7;

	// Only applicable for PTE
	unsigned long long protectionKey : 4;

	// Disable execution of code from this page
	unsigned long long disableExecution : 1;
} PACKED PageEntry;

_Static_assert(sizeof(PageEntry) == sizeof(uint64_t), "page entry has to be 64 bits");

static PageEntry *pml4entries PAGE_ALIGNED = null;

uint64_t conventionalAllocateNext(uint64_t bytes);
static void *allocatePage() {
	// TODO bounds check
	return (void *)conventionalAllocateNext(PAGE_SIZE);
}

static LinearAddress getLinearAddress(uint64_t address) {
	return *((LinearAddress *)&address);
}

static function initializePage(PageEntry *entry, uint64_t address, uint8_t writeAllowed) {
	entry->address = address >> ADDRESS_BITS;
	entry->present = 1;
	entry->writeAllowed = (writeAllowed == 1) ? 1 : 0;
}

static function initializePageHuge(PageEntry *entry, uint64_t address) {
	entry->address = address >> ADDRESS_BITS;
	entry->present = 1;
	entry->writeAllowed = 1;
	entry->largePage = 1;
}

static void *getPage(PageEntry *table, uint64_t entryId, uint8_t writeAllowed) {
	PageEntry *entry = &table[entryId];

	if (entry->present == 1) {
		return (void *)(entry->address << ADDRESS_BITS);
	} else {
		void *newPage = allocatePage();
		initializePage(entry, (uint64_t)newPage, writeAllowed);
		return newPage;
	}
}

function map_pt(PageEntry pt[], uint64_t virtualAddr, uint64_t physicalAddr, uint8_t writeAllowed) {
	PageEntry *entry = &pt[getLinearAddress(virtualAddr).pt];
	initializePage(entry, physicalAddr, writeAllowed);
}

function map_p2huge(PageEntry pd[], uint64_t virtualAddr, uint64_t physicalAddr) {
	PageEntry *entry = &pd[getLinearAddress(virtualAddr).pd];
	initializePageHuge(entry, physicalAddr);
}

#define createMapping(fromTable, toTable)                                                                    \
	static void map_##fromTable(PageEntry fromTable[], uint64_t virtualAddr, uint64_t physicalAddr,          \
								uint8_t writeAllowed) {                                                      \
		void *toTable = getPage(fromTable, getLinearAddress(virtualAddr).fromTable, writeAllowed);           \
		map_##toTable((PageEntry *)toTable, virtualAddr, physicalAddr, writeAllowed);                        \
	}

createMapping(pd, pt);
createMapping(pdpt, pd);
createMapping(pml4, pdpt);
#undef createMapping

#define createHugeMapping(name, fromTable, calls, toTable)                                                   \
	static void map_##name(PageEntry fromTable[], uint64_t virtualAddr, uint64_t physicalAddr) {             \
		void *toTable = getPage(fromTable, getLinearAddress(virtualAddr).fromTable, 1);                      \
		map_##calls((PageEntry *)toTable, virtualAddr, physicalAddr);                                        \
	}

createHugeMapping(p3huge, pdpt, p2huge, pd);
createHugeMapping(p4huge, pml4, p3huge, pdpt);
#undef createHugeMapping

function mapMemory(uint64_t virtualAddr, uint64_t physicalAddr, uint32_t pageCount, uint8_t writeAllowed) {
	serialPrintln(u8"[paging] mapping memory range");

	uint64_t virtualAddrEnd = virtualAddr + pageCount * PAGE_SIZE;

	uint64_t vAddress = virtualAddr;
	uint64_t pAddress = physicalAddr;

	serialPrintf(u8"[paging.range] bytes = %d or %d\n", virtualAddrEnd - virtualAddr, pageCount * PAGE_SIZE);

	serialPrint(u8"[paging.range] virtual address = ");
	serialPrintHex((uint64_t)(virtualAddr));
	serialPrint(u8"\n");

	serialPrint(u8"[paging.range] physical address = ");
	serialPrintHex((uint64_t)(physicalAddr));
	serialPrint(u8"\n");

	serialPrint(u8"[paging.range] page count = ");
	serialPrintHex((uint64_t)(pageCount));
	serialPrint(u8"\n");

	serialPrint(u8"[paging.range] virtual address end = ");
	serialPrintHex((uint64_t)(virtualAddrEnd));
	serialPrint(u8"\n");

	while (vAddress < virtualAddrEnd) {
		map_pml4(pml4entries, vAddress, pAddress, writeAllowed);

		vAddress += PAGE_SIZE;
		pAddress += PAGE_SIZE;
	}
}

function mapMemoryHuge(uint64_t virtualAddr, uint64_t physicalAddr, uint32_t pageCount) {
	uint64_t virtualAddrEnd = virtualAddr + pageCount * PAGE_SIZE;

	uint64_t vAddress = virtualAddr;
	uint64_t pAddress = physicalAddr;

	while (vAddress < virtualAddrEnd) {
		map_p4huge(pml4entries, vAddress, pAddress);

		vAddress += PAGE_SIZE * 512;
		pAddress += PAGE_SIZE * 512;
	}
}

static const efi::EFI_MEMORY_DESCRIPTOR *getNextDescriptor(const efi::EFI_MEMORY_DESCRIPTOR *descriptor,
														   uint64_t descriptorSize) {
	const uint8_t *desc = ((const uint8_t *)descriptor) + descriptorSize;
	return (const efi::EFI_MEMORY_DESCRIPTOR *)desc;
}

uint64_t getRAMSize(EfiMemoryMap *memoryMap) {
	const efi::EFI_MEMORY_DESCRIPTOR *descriptor = memoryMap->memoryMap;
	const uint64_t descriptorSize = memoryMap->descriptorSize;
	uint64_t numberOfPages = 0;

	uint64_t startOfMemoryMap = (uint64_t)memoryMap->memoryMap;
	uint64_t endOfMemoryMap = startOfMemoryMap + memoryMap->memoryMapSize;
	uint64_t offset = startOfMemoryMap;

	while (offset < endOfMemoryMap) {
		numberOfPages += descriptor->NumberOfPages;

		offset += descriptorSize;
		descriptor = getNextDescriptor(descriptor, descriptorSize);
	}

	return numberOfPages * PAGE_SIZE;
}

// TODO this should be reconsidered (i.e. pointer fixups)
function mapEfi(EfiMemoryMap *memoryMap) {
	serialPrintln(u8"[paging] mapping efi");

	const efi::EFI_MEMORY_DESCRIPTOR *descriptor = memoryMap->memoryMap;
	const uint64_t descriptorSize = memoryMap->descriptorSize;
	uint64_t sum = 0;

	uint64_t startOfMemoryMap = (uint64_t)memoryMap->memoryMap;
	uint64_t endOfMemoryMap = startOfMemoryMap + memoryMap->memoryMapSize;
	uint64_t offset = startOfMemoryMap;

	while (offset < endOfMemoryMap) {
		if (descriptor->Attribute & EFI_MEMORY_RUNTIME) {
			mapMemory(descriptor->PhysicalStart, descriptor->PhysicalStart, descriptor->NumberOfPages, 0);
			sum += descriptor->NumberOfPages;
		}

		offset += descriptorSize;
		descriptor = getNextDescriptor(descriptor, descriptorSize);
	}

	serialPrintf(u8"[paging] efi mapped %u pages\n", sum);
}

#if 0
uint64_t conventionalAllocate(EfiMemoryMap *memoryMap, uint32_t pages) {
	const efi::EFI_MEMORY_DESCRIPTOR *descriptor = memoryMap->memoryMap;
	const uint64_t descriptorSize = memoryMap->descriptorSize;
	uint64_t result = 0;

	uint64_t startOfMemoryMap = (uint64_t)memoryMap->memoryMap;
	uint64_t endOfMemoryMap = startOfMemoryMap + memoryMap->memoryMapSize;
	uint64_t offset = startOfMemoryMap;

	while (offset < endOfMemoryMap) {

		if ((descriptor->Type == efi::EfiConventionalMemory) && (descriptor->NumberOfPages >= (pages + 1))) {
			serialPrintf(u8"[paging] success allocate %d pages\n", pages);
			result = ((descriptor->PhysicalStart / PAGE_SIZE) * PAGE_SIZE + PAGE_SIZE);
		}

		offset += descriptorSize;
		descriptor = getNextDescriptor(descriptor, descriptorSize);
	}

	return result;
}
#endif

uint8_t buffa[1] = {0};

uint64_t conventionalAllocateLargest(EfiMemoryMap *memoryMap) {
	const efi::EFI_MEMORY_DESCRIPTOR *descriptor = memoryMap->memoryMap;
	const uint64_t descriptorSize = memoryMap->descriptorSize;
	uint64_t result = 0;
	uint64_t largestPages = 0;

	uint64_t startOfMemoryMap = (uint64_t)memoryMap->memoryMap;
	uint64_t endOfMemoryMap = startOfMemoryMap + memoryMap->memoryMapSize;
	uint64_t offset = startOfMemoryMap;

	while (offset < endOfMemoryMap) {
		// Note: > not >= cause we should have some extra space next to this
		if ((descriptor->Type == efi::EfiConventionalMemory) && (descriptor->NumberOfPages > largestPages)) {
			largestPages = descriptor->NumberOfPages;
			result = descriptor->PhysicalStart;
		}

		offset += descriptorSize;
		descriptor = getNextDescriptor(descriptor, descriptorSize);
	}

	serialPrintf(u8"[paging] conventionalAllocateLargest is %u bytes, %d pages\n",
				 (uint32_t)(largestPages * PAGE_SIZE), largestPages);

	return result;
}

uint64_t conventionalOffset;
uint64_t conventionalAllocateNext(uint64_t bytes) {
	let result = conventionalOffset;
	let pages = bytes / PAGE_SIZE; // Math.floor
	conventionalOffset += pages * PAGE_SIZE;
	if ((bytes - (pages * PAGE_SIZE)) > 0)
		conventionalOffset += PAGE_SIZE;
	return result;
}

function mapFramebuffer(const Framebuffer *fb) {
	let framebufferBase = fb->base;
	mapMemory(FramebufferStart, (uint64_t)framebufferBase, fb->size / PAGE_SIZE + 1, 1);
}
} // namespace paging
