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

// AMD64 pages format and map/unmap internal implementation

namespace pages {

#define ADDRESS_BITS 12
#define PAGE_ALIGNED __attribute__((aligned(PAGE_SIZE)))
#define PACKED __attribute__((gcc_struct, packed))

typedef struct {
	unsigned long long offset: 12;
	unsigned long long pt: 9;
	unsigned long long pd: 9;
	unsigned long long pdpt: 9;
	unsigned long long pml4: 9;
	unsigned long long reserved: 16;
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
	unsigned long long address  : 40;

	// Not used by the processor
	unsigned long long metadata2 : 7;

	// Only applicable for PTE
	unsigned long long protectionKey : 4;

	// Disable execution of code from this page
	unsigned long long disableExecution : 1;
} PACKED PageEntry;

_Static_assert(sizeof(PageEntry) == sizeof(uint64_t), "page entry has to be 64 bits");

static PageEntry* pml4entries PAGE_ALIGNED = null;

static void *allocatePage() {
	return (void*)PhysicalAllocator::allocateOnePagePreZeroed();
	// TODO bounds check
}

static LinearAddress getLinearAddress(uint64_t virtualAddr) {
	return *((LinearAddress *) &virtualAddr);
}

static function initializePage(PageEntry *entry, uint64_t address) {
	entry->address = address >> ADDRESS_BITS;
	entry->present = 1;
	entry->writeAllowed = 1;
}

static void *getPage(PageEntry *table, uint64_t entryId) {
	PageEntry *entry = &table[entryId];

	if (entry->present == 1) {
		return (void *) ((entry->address << ADDRESS_BITS) + (uint64_t)WholePhysicalStart);
	} else {
		void *newPage = allocatePage();
		initializePage(entry, (uint64_t) newPage - (uint64_t)WholePhysicalStart);
		return newPage;
	}
}

function map_pt(PageEntry pt[], uint64_t virtualAddr, uint64_t physicalAddr) {
	PageEntry *entry = &pt[getLinearAddress(virtualAddr).pt];
	initializePage(entry, physicalAddr);
}

#define createMapping(fromTable, toTable)                                            \
	static void map_ ## fromTable (                                                  \
			PageEntry fromTable[],                                                   \
			uint64_t virtualAddr,                                                    \
			uint64_t physicalAddr) {                                                 \
		void *toTable = getPage(fromTable, getLinearAddress(virtualAddr).fromTable); \
		map_ ## toTable ((PageEntry *)toTable, virtualAddr, physicalAddr);           \
	}

createMapping(pd, pt)
createMapping(pdpt, pd)
createMapping(pml4, pdpt)

#undef createMapping

// TODO should take pml4 as argument
uint64_t resolveAddr(uint64_t virtualAddr) {
	auto linear = getLinearAddress(virtualAddr);
	// pml4
	PageEntry *entry = &pml4entries[linear.pml4];
	if (entry->present == 0) return 0;
	entry = (PageEntry *) ((entry->address << ADDRESS_BITS) + (uint64_t)WholePhysicalStart);
	// pdpt
	entry = &entry[linear.pdpt];
	if (entry->present == 0) return 0;
	// TODO handle mega pages
	entry = (PageEntry *) ((entry->address << ADDRESS_BITS) + (uint64_t)WholePhysicalStart);
	// pd
	entry = &entry[linear.pd];
	if (entry->present == 0) return 0;
	// TODO handle huge pages
	entry = (PageEntry *) ((entry->address << ADDRESS_BITS) + (uint64_t)WholePhysicalStart);
	// pt
	entry = &entry[linear.pt];
	if (entry->present == 0) return 0;
	return (entry->address << ADDRESS_BITS) + linear.offset;
}

// TODO return error code (as enum)
// TODO ^ hexa @mustCheckReturn for return values (like Golang)
// TODO ^ same clang warning
function mapMemory(PageEntry* pml4entries, uint64_t virtualAddr, uint64_t physicalAddr, uint32_t pageCount) {
	serialPrintln(u8"[paging] mapping memory range");

	uint64_t virtualAddrEnd = virtualAddr + pageCount * PAGE_SIZE;

	uint64_t vAddress = virtualAddr;
	uint64_t pAddress = physicalAddr;

	serialPrintf(u8"[paging.range] bytes = %d or %d\n", virtualAddrEnd - virtualAddr, pageCount * PAGE_SIZE);

	serialPrint(u8"[paging.range] virtual address = ");
	serialPrintHex((uint64_t) (virtualAddr));
	serialPrint(u8"\n");

	serialPrint(u8"[paging.range] physical address = ");
	serialPrintHex((uint64_t) (physicalAddr));
	serialPrint(u8"\n");

	serialPrint(u8"[paging.range] page count = ");
	serialPrintHex((uint64_t) (pageCount));
	serialPrint(u8"\n");

	serialPrint(u8"[paging.range] virtual address end = ");
	serialPrintHex((uint64_t) (virtualAddrEnd));
	serialPrint(u8"\n");

	while (vAddress < virtualAddrEnd) {
		map_pml4(pml4entries, vAddress, pAddress);

		vAddress += PAGE_SIZE;
		pAddress += PAGE_SIZE;
	}
}

// Upper half
// Just makes a copy of upper half's PML4,
// because it is always the same between processes
function copyKernelMemory(const PageEntry* pml4source, PageEntry* pml4destination) {
}

// Creates new PML4 for new process
PageEntry* newCR3(const PageEntry* pml4source) {
	PageEntry* pml4result;
	copyKernelMemory(pml4source, pml4result);
	return pml4result;
}

PageEntry* freeCR3(PageEntry* pml4) {
	// TODO deallocate full lower half
}

// Lower half
// TODO protection ring
// TODO lower half limit bounds check (it is less than upper starting range)
function mapUserspaceMemory(PageEntry* pml4entries, uint64_t virtualAddr, uint64_t physicalAddr, uint32_t pageCount) {
}

// Same as VirtualAlloc
// TODO protection ring
// TODO respect WoW limits
// TODO VirtualAllocEx-like behavior (i.e. alloc for other processes, not only current)
// Decides automatically where to allocate
// if virtualAddr == 0
// Note: also allocates physical pages, i.e consumes extra memory
function allocUserspaceMemory(PageEntry* pml4entries, uint64_t virtualAddr, uint32_t pageCount) {
}
}
