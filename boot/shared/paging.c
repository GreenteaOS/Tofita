#define PAGE_SIZE 4096  // 4 KiB
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

// How many pages to allocate in order to assign them later to be used by paging
#define PAGES_TO_ALLOCATE 32

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

static void writeCr3(uint64_t value) {
	__asm__("movq %0, %%cr3" :: "r"(value));
}

static uint8_t pages[PAGES_TO_ALLOCATE][PAGE_SIZE] PAGE_ALIGNED;
static uint8_t lastPageIndex = 0;

static void *allocatePage() {
	return (void *) pages[lastPageIndex++];
}

static LinearAddress getLinearAddress(uint64_t address) {
	return *((LinearAddress *) &address);
}

static void initializePage(PageEntry *entry, uint64_t address) {
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

static void map_pt(PageEntry pt[], uint64_t virtualAddr, uint64_t physicalAddr) {
	PageEntry *entry = &pt[getLinearAddress(virtualAddr).pt];
	initializePage(entry, physicalAddr);
}

#define CREATE_MAPPING(fromTable, toTable) \
	static void map_ ## fromTable (PageEntry fromTable[],                            \
			uint64_t virtualAddr, uint64_t physicalAddr) {                           \
		void *toTable = getPage(fromTable, getLinearAddress(virtualAddr).fromTable); \
		map_ ## toTable (toTable, virtualAddr, physicalAddr);                        \
	}

CREATE_MAPPING(pd, pt)
CREATE_MAPPING(pdpt, pd)
CREATE_MAPPING(pml4, pdpt)

static void mapMemory(uint64_t virtualAddr, uint64_t physicalAddr, uint32_t pageCount) {
	serialPrintln("[paging] mapping memory range");

	uint64_t virtualAddrEnd = virtualAddr + pageCount * PAGE_SIZE;

	uint64_t vAddress = virtualAddr;
	uint64_t pAddress = physicalAddr;

	serialPrintf("[paging.range] bytes = %d or %d\r\n", virtualAddrEnd - virtualAddr, pageCount * PAGE_SIZE);

	serialPrint("[paging.range] virtual address = ");
	serialPrintHex((uint64_t) (virtualAddr));
	serialPrint("\r\n");

	serialPrint("[paging.range] physical address = ");
	serialPrintHex((uint64_t) (physicalAddr));
	serialPrint("\r\n");

	serialPrint("[paging.range] page count = ");
	serialPrintHex((uint64_t) (pageCount));
	serialPrint("\r\n");

	serialPrint("[paging.range] virtual address end = ");
	serialPrintHex((uint64_t) (virtualAddrEnd));
	serialPrint("\r\n");

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

static void mapEfi(EfiMemoryMap *memoryMap) {
	const EFI_MEMORY_DESCRIPTOR *descriptor = memoryMap->memoryMap;
	const uint64_t descriptorSize = memoryMap->descriptorSize;

	for (uint64_t i = 0; i < memoryMap->memoryMapSize; i++) {
		if (descriptor->Attribute & EFI_MEMORY_RUNTIME) {
			mapMemory(descriptor->PhysicalStart, descriptor->PhysicalStart,
					descriptor->NumberOfPages);
		}

		descriptor = getNextDescriptor(descriptor, descriptorSize);
	}
}

static void mapFramebuffer(Framebuffer *fb) {
	void *framebufferBase = fb->base;
	mapMemory(FramebufferStart, (uint64_t) framebufferBase, fb->size / PAGE_SIZE + 1);
}

static void mapRamDisk(RamDisk *ramdisk) {
	void *ramdiskBase = ramdisk->base;
	mapMemory(RamdiskStart, (uint64_t) ramdiskBase, ramdisk->size / PAGE_SIZE + 1);
}

void enablePaging(void *tofitaKernel, EfiMemoryMap *memoryMap, Framebuffer *fb, RamDisk *ramdisk, KernelParams *params) {
	mapMemory(KernelStart, KernelStart, 256);
	serialPrintln("[paging] kernel mapped");

	mapMemory(KernelVirtualBase, (uint64_t) tofitaKernel, 256);
	serialPrintln("[paging] Tofita kernel mapped");

	mapEfi(memoryMap);
	serialPrintln("[paging] efi mapped");

	mapFramebuffer(fb);
	serialPrintln("[paging] framebuffer mapped");

	mapRamDisk(ramdisk);
	serialPrintln("[paging] ramdisk mapped");

	// Round upto page size
	uint64_t BUFFER_START = RamdiskStart / PAGE_SIZE + params->ramdisk.size / PAGE_SIZE + 1;
	BUFFER_START *= PAGE_SIZE;
	mapMemory(BUFFER_START, (uint64_t) params->buffer, params->bufferSize / PAGE_SIZE + 1);
	params->buffer = BUFFER_START;
	serialPrintln("[paging] buffer mapped");

	serialPrint("[paging] CR3 points to: ");
	serialPrintHex((uint64_t) pml4);
	serialPrint("\r\n");

	writeCr3((uint64_t) pml4);
}
