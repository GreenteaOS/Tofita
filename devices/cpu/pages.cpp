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
	uint16_t offset : 12;
	uint16_t pt : 9;
	uint16_t pd : 9;
	uint16_t pdpt : 9;
	uint16_t pml4 : 9;
	uint16_t reserved : 16;
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
	uint64_t address : 40;

	// Not used by the processor
	uint8_t metadata2 : 7;

	// Only applicable for PTE
	uint8_t protectionKey : 4;

	// Disable execution of code from this page
	uint8_t disableExecution : 1;
} PACKED PageEntry;

_Static_assert(sizeof(PageEntry) == sizeof(uint64_t), "page entry has to be 64 bits");

// TODO remove this
static PageEntry *pml4entries PAGE_ALIGNED = null;

static void *allocatePage() {
	return (void *)PhysicalAllocator::allocateOnePagePreZeroed();
	// TODO bounds check
}

static LinearAddress getLinearAddress(uint64_t virtualAddr) {
	return *((LinearAddress *)&virtualAddr);
}

static function initializePage(PageEntry *entry, uint64_t address) {
	entry->address = address >> ADDRESS_BITS;
	entry->present = 1;
	entry->writeAllowed = 1;

	// TODO
	entry->accessibleByAll = 1;
}

static void *getPage(PageEntry *table, uint64_t entryId) {
	PageEntry *entry = &table[entryId];

	if (entry->present == 1) {
		return (void *)((entry->address << ADDRESS_BITS) + (uint64_t)WholePhysicalStart);
	} else {
		void *newPage = allocatePage();
		initializePage(entry, (uint64_t)newPage - (uint64_t)WholePhysicalStart);
		return newPage;
	}
}

function map_pt(PageEntry pt[], uint64_t virtualAddr, uint64_t physicalAddr) {
	PageEntry *entry = &pt[getLinearAddress(virtualAddr).pt];
	initializePage(entry, physicalAddr);
}

// TODO handle huge pages returned from bootloader
#define createMapping(fromTable, toTable)                                                                    \
	static void map_##fromTable(PageEntry fromTable[], uint64_t virtualAddr, uint64_t physicalAddr) {        \
		void *toTable = getPage(fromTable, getLinearAddress(virtualAddr).fromTable);                         \
		map_##toTable((PageEntry *)toTable, virtualAddr, physicalAddr);                                      \
	}

createMapping(pd, pt);
createMapping(pdpt, pd);
createMapping(pml4, pdpt);

#undef createMapping

// Returns actual physical address from virtual address
uint64_t resolveAddr(const PageEntry *pml4entries, uint64_t virtualAddr) {
	auto linear = getLinearAddress(virtualAddr);
	// pml4
	const PageEntry *entry = &pml4entries[linear.pml4];
	if (entry->present == 0)
		return PHYSICAL_NOT_FOUND;
	entry = (const PageEntry *)((entry->address << ADDRESS_BITS) + (uint64_t)WholePhysicalStart);
	// pdpt
	entry = &entry[linear.pdpt];
	if (entry->present == 0)
		return PHYSICAL_NOT_FOUND;
	// TODO handle mega pages
	entry = (const PageEntry *)((entry->address << ADDRESS_BITS) + (uint64_t)WholePhysicalStart);
	// pd
	entry = &entry[linear.pd];
	if (entry->present == 0)
		return PHYSICAL_NOT_FOUND;
	if (entry->largePage == 1)
		return (entry->address << ADDRESS_BITS) + linear.pt * 4096 + linear.offset;
	entry = (const PageEntry *)((entry->address << ADDRESS_BITS) + (uint64_t)WholePhysicalStart);
	// pt
	entry = &entry[linear.pt];
	if (entry->present == 0)
		return PHYSICAL_NOT_FOUND;
	return (entry->address << ADDRESS_BITS) + linear.offset;
}

// TODO return error code (as enum)
// TODO ^ hexa @mustCheckReturn for return values (like Golang)
// TODO ^ same clang warning
// TODO should accept usermode flag so shared memory and GDI buffers can be mapped out of user scope
// ^ probably should be other way for WoW processes
function mapMemory(PageEntry *pml4entries, uint64_t virtualAddr, uint64_t physicalAddr, uint32_t pageCount) {
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
		map_pml4(pml4entries, vAddress, pAddress);

		vAddress += PAGE_SIZE;
		pAddress += PAGE_SIZE;
	}

	// TODO rethink where to call this
	amd64::writeCr3((uint64_t)pml4entries - (uint64_t)WholePhysicalStart);
}

// Unmaps pages without actually deallocating them, still deallocates unused page tables
// (page tables are managed only and only here anyway)
function unmapMemory(PageEntry *pml4entries, uint64_t virtualAddr, uint32_t pageCount) {
	// TODO do NOT unmap reserved pages, i.e. WholePhysicalStart and upper half region!
	// TODO probably special shared buffer allocator+mapper which uses linked list
	// way upper than WholePhysicalStart limit
	// TODO
}

// Completely unmaps and deallocates pages and page tables
function unmapAndFreeMemory(PageEntry *pml4entries, uint64_t virtualAddr, uint32_t pageCount) {
	// TODO resolve and dealloc pages
	unmapMemory(pml4entries, virtualAddr, pageCount);
}

// Upper half
// Just makes a copy of upper half's PML4,
// because it is always the same between processes
function copyKernelMemory(const PageEntry *pml4source, PageEntry *pml4destination) {}

// Creates new PML4 for new process
PageEntry *newCR3(const PageEntry *pml4source) {
	PageEntry *pml4result;
	copyKernelMemory(pml4source, pml4result);
	return pml4result;
}

PageEntry *freeCR3(PageEntry *pml4) {
	// TODO deallocate full lower half
}

// Lower half
// TODO protection ring
// TODO lower half limit bounds check (it is less than upper starting range)
function mapUserspaceMemory(PageEntry *pml4entries, uint64_t virtualAddr, uint64_t physicalAddr,
							uint32_t pageCount) {}

// Same as VirtualAlloc
// TODO protection ring
// TODO respect WoW limits
// TODO VirtualAllocEx-like behavior (i.e. alloc for other processes, not only current)
// Decides automatically where to allocate
// if virtualAddr == 0
// Note: also allocates physical pages, i.e consumes extra memory
function allocUserspaceMemory(PageEntry *pml4entries, uint64_t virtualAddr, uint32_t pageCount) {}
} // namespace pages
