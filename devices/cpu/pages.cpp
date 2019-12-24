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

// AMD64 pages format and map/unmap internal implementation

namespace pages {

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

static PageEntry* pml4entries PAGE_ALIGNED = null;
typedef uint8_t PagesArray[PAGE_SIZE];
static PagesArray* pages PAGE_ALIGNED = null;
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
		map_pml4(pml4entries, vAddress, pAddress);

		vAddress += PAGE_SIZE;
		pAddress += PAGE_SIZE;
	}
}

static const efi::EFI_MEMORY_DESCRIPTOR *getNextDescriptor(const efi::EFI_MEMORY_DESCRIPTOR *descriptor, uint64_t descriptorSize) {
	const uint8_t *desc = ((const uint8_t *) descriptor) + descriptorSize;
	return (const efi::EFI_MEMORY_DESCRIPTOR *) desc;
}
}
