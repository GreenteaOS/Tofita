// The Tofita Kernel
// Copyright (C) 2019  Oleg Petrenko
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
#define PACKED __attribute__((packed))

typedef struct {
	uint16_t offset: 12;
	uint16_t pt: 9;
	uint16_t pd: 9;
	uint16_t pdpt: 9;
	uint16_t pml4: 9;
	uint16_t reserved: 16;
} PACKED LinearAddress;

_Static_assert(sizeof(LinearAddress) == sizeof(uint64_t), "linear address has to have 64 bits");

// 512 entries * 8 bytes = 4 KiB
// PML4 size = PDP size = PD size = PT size
//
// Since we are using 64 bit mode, the entry size
// is 64 bits.

#define PAGE_TABLE_SIZE 512

// Entry in a page table
typedef struct {
	// Is the page present in physical memory?
	uint8_t present : 1;

	// Pages are read-only by default
	uint8_t writeAllowed : 1;

	// Pages are only accessible by supervisor by default
	uint8_t accessibleByAll : 1;

	// Write through abilities of the page
	uint8_t writeThrough : 1;

	// If set, the page will not be cached
	uint8_t cacheDisabled : 1;

	// Was the page accessed?
	uint8_t accessed : 1;

	// Has the page been written to? Only applicable for PTE.
	uint8_t dirty : 1;

	// Page size by default is small, enabling this bit makes it bigger. Only applicable for PTE
	uint8_t largePage : 1;

	// Prevent the translations cache from updating
	// the address in cache if CR3 is reset.
	//
	// Page global enable bit in CR4 has to be set to
	// enable this feature.
	//
	// Only applicable for PTE
	uint8_t global : 1;

	// Not used by the processor
	uint8_t metadata : 3;

	// Physical address of the child table/page
	uint64_t address  : 40;

	// Not used by the processor
	uint8_t metadata2 : 7;

	// Only applicable for PTE
	uint8_t protectionKey : 4;

	// Disable execution of code from this page
	uint8_t disableExecution : 1;
} PACKED PageEntry;

_Static_assert(sizeof(PageEntry) == sizeof(uint64_t), "page entry has to be 64 bits");

PageEntry pml4[PAGE_TABLE_SIZE] PAGE_ALIGNED;
typedef uint8_t pagesArray[PAGE_SIZE];
static pagesArray* pages PAGE_ALIGNED = null;
static int32_t lastPageIndex = 0;

static void *allocatePage() {
	// TODO bounds check
	return (void *) pages[lastPageIndex--];
}

static LinearAddress getLinearAddress(uint64_t address) {
	return *((LinearAddress *) &address);
}

static function initializePage(PageEntry *entry, uint64_t address) {
	entry->address = address >> ADDRESS_BITS;
	entry->present = 1;
	entry->writeAllowed = 1;
}

static void *getPage(PageEntry *table, uint64_t entryId) {
	PageEntry *entry = &table[entryId];

	if (entry->present == 1) {
		return (void *) (entry->address << ADDRESS_BITS);
	} else {
		void *newPage = allocatePage();
		initializePage(entry, (uint64_t) newPage);
		return newPage;
	}
}

function map_pt(PageEntry pt[], uint64_t virtualAddr, uint64_t physicalAddr) {
	PageEntry *entry = &pt[getLinearAddress(virtualAddr).pt];
	initializePage(entry, physicalAddr);
}

#define createMapping(fromTable, toTable)                                            \
	static void map_ ## fromTable (PageEntry fromTable[],                            \
			uint64_t virtualAddr, uint64_t physicalAddr) {                           \
		void *toTable = getPage(fromTable, getLinearAddress(virtualAddr).fromTable); \
		map_ ## toTable ((PageEntry *)toTable, virtualAddr, physicalAddr);           \
	}

createMapping(pd, pt)
createMapping(pdpt, pd)
createMapping(pml4, pdpt)

#undef createMapping

function mapMemory(uint64_t virtualAddr, uint64_t physicalAddr, uint32_t pageCount) {
	serialPrintln("[paging] mapping memory range");

	uint64_t virtualAddrEnd = virtualAddr + pageCount * PAGE_SIZE;

	uint64_t vAddress = virtualAddr;
	uint64_t pAddress = physicalAddr;

	serialPrintf("[paging.range] bytes = %d or %d\n", virtualAddrEnd - virtualAddr, pageCount * PAGE_SIZE);

	serialPrint("[paging.range] virtual address = ");
	serialPrintHex((uint64_t) (virtualAddr));
	serialPrint("\n");

	serialPrint("[paging.range] physical address = ");
	serialPrintHex((uint64_t) (physicalAddr));
	serialPrint("\n");

	serialPrint("[paging.range] page count = ");
	serialPrintHex((uint64_t) (pageCount));
	serialPrint("\n");

	serialPrint("[paging.range] virtual address end = ");
	serialPrintHex((uint64_t) (virtualAddrEnd));
	serialPrint("\n");

	while (vAddress < virtualAddrEnd) {
		map_pml4(pml4, vAddress, pAddress);

		vAddress += PAGE_SIZE;
		pAddress += PAGE_SIZE;
	}
}

static const EFI_MEMORY_DESCRIPTOR *getNextDescriptor(const EFI_MEMORY_DESCRIPTOR *descriptor, uint64_t descriptorSize) {
	const uint8_t *desc = ((const uint8_t *) descriptor) + descriptorSize;
	return (const EFI_MEMORY_DESCRIPTOR *) desc;
}

uint64_t getRAMSize(EfiMemoryMap *memoryMap) {
	const EFI_MEMORY_DESCRIPTOR *descriptor = memoryMap->memoryMap;
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

function mapEfi(EfiMemoryMap *memoryMap) {
	serialPrintln("[paging] mapping efi");

	const EFI_MEMORY_DESCRIPTOR *descriptor = memoryMap->memoryMap;
	const uint64_t descriptorSize = memoryMap->descriptorSize;

	uint64_t startOfMemoryMap = (uint64_t)memoryMap->memoryMap;
	uint64_t endOfMemoryMap = startOfMemoryMap + memoryMap->memoryMapSize;
	uint64_t offset = startOfMemoryMap;

	while (offset < endOfMemoryMap) {
		if (descriptor->Attribute & EFI_MEMORY_RUNTIME) {
			mapMemory(descriptor->PhysicalStart, descriptor->PhysicalStart,
					descriptor->NumberOfPages);
		}

		offset += descriptorSize;
		descriptor = getNextDescriptor(descriptor, descriptorSize);
	}

	serialPrintln("[paging] efi mapped");
}

uint64_t conventionalAllocate(EfiMemoryMap *memoryMap, uint32_t pages) {
	const EFI_MEMORY_DESCRIPTOR *descriptor = memoryMap->memoryMap;
	const uint64_t descriptorSize = memoryMap->descriptorSize;
	uint64_t result = 0;

	uint64_t startOfMemoryMap = (uint64_t)memoryMap->memoryMap;
	uint64_t endOfMemoryMap = startOfMemoryMap + memoryMap->memoryMapSize;
	uint64_t offset = startOfMemoryMap;

	while (offset < endOfMemoryMap) {

		if ((descriptor->Type == EfiConventionalMemory) && (descriptor->NumberOfPages >= (pages + 1))) {
			serialPrintf("[paging] success allocate %d pages\n", pages);
			result = ((descriptor->PhysicalStart / PAGE_SIZE) * PAGE_SIZE + PAGE_SIZE);
		}

		offset += descriptorSize;
		descriptor = getNextDescriptor(descriptor, descriptorSize);
	}

	return result;
}

function mapFramebuffer(Framebuffer *fb) {
	let framebufferBase = fb->base;
	mapMemory(FramebufferStart, (uint64_t) framebufferBase, fb->size / PAGE_SIZE + 1);
}

function mapRamDisk(RamDisk *ramdisk) {
	let ramdiskBase = ramdisk->base;
	mapMemory(RamdiskStart, (uint64_t) ramdiskBase, ramdisk->size / PAGE_SIZE + 1);
}

uint8_t buffa[1] = {0};
uint64_t enablePaging(EfiMemoryMap *memoryMap, Framebuffer *fb, RamDisk *ramdisk, KernelParams *params) {
	params->bufferSize = 32 * 1024 * 1024;
	uint32_t bufferPages = params->bufferSize / PAGE_SIZE;
	params->buffer = conventionalAllocate(memoryMap, bufferPages);

	uint8_t* bb = (uint8_t*)params->buffer;
	buffa[0] = 0;
	for (int i = 0; i < params->bufferSize; ++i)
	{
		// TODO faster with uint64_t
		bb[i] = buffa[0];
	}

	pages = (pagesArray*)((uint64_t)params->buffer + bufferPages * PAGE_SIZE - PAGE_SIZE * 1);

	// CR3 trampoline
	mapMemory(params->physical + 1024*1024, params->physical + 1024*1024, 1);
	serialPrintln("[paging] kernel loader mapped");

	// TODO use actual kernel asset size
	mapMemory(KernelVirtualBase, params->physical, 256);
	serialPrintln("[paging] Tofita kernel mapped");

	mapEfi(memoryMap);

	mapFramebuffer(fb);
	serialPrintln("[paging] framebuffer mapped");

	mapRamDisk(ramdisk);
	serialPrintln("[paging] ramdisk mapped");

	// Round upto page size
	uint64_t BUFFER_START = RamdiskStart / PAGE_SIZE + params->ramdisk.size / PAGE_SIZE + 1;
	BUFFER_START *= PAGE_SIZE;
	// TODO use largest conventional memory region as buffer!
	mapMemory(BUFFER_START, (uint64_t) params->buffer, params->bufferSize / PAGE_SIZE + 1);
	params->buffer = BUFFER_START;
	serialPrintln("[paging] buffer mapped");

	uint64_t ram = getRAMSize(memoryMap);
	serialPrintf("[paging] available RAM is ~%d megabytes\n", (uint32_t)(ram/(1024*1024)));
	params->ramBytes = ram;

	// TODO map whole memory with 2 MB huge pages
	// otherwise would be really hard to operate on plm4 in kernel

	// Replace to virtual adresses
	params->framebuffer.physical = params->framebuffer.base;
	params->framebuffer.base = FramebufferStart;

	params->ramdisk.physical = params->ramdisk.base;
	params->ramdisk.base = RamdiskStart;

	serialPrint("[paging] CR3 points to: ");
	serialPrintHex((uint64_t) pml4);
	serialPrint("\n");

	return (uint64_t) pml4;
}
}
