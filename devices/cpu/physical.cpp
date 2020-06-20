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

#define PHYSICAL_NOT_FOUND 0
// A lot of non-zero bits
// i.e. good in case of cosmic rays or something
// Completely reserved by hardware and kernel executable image
// Anything that is not defined considered to be PAGE_RESERVED
#define PAGE_RESERVED 0xFF
#define PAGE_FREE 0b1010
#define PAGE_FULL 0b0101
#define PAGE_MASK 0b1111
// TODO #define GRANULAR_PARTIAL 0b1010
// TODO #define GRANULAR_FULL 0b1001
#define DOWN_BYTES_TO_PAGES(bytes) ((bytes) >> 12)
// 4095 >> 12 == 0
// 4096 >> 12 == 1
// 8191 >> 12 == 1
// 8192 >> 12 == 2

class PhysicalAllocator {
  private:
	// Approx. 1 megabyte per each 4 gygabyte of RAM
	static uint8_t *buffer; // TODO use bits, not bytes
	static uint64_t count;
	static uint64_t last;

  public:
	static function init(const KernelParams *params) {
		buffer = (uint8_t *)params->physicalRamBitMaskVirtual;
		const EfiMemoryMap *memoryMap = &params->efiMemoryMap;
		count = DOWN_BYTES_TO_PAGES(params->ramBytes);
		last = PHYSICAL_NOT_FOUND;

		memset((void *)buffer, PAGE_RESERVED, count);

		// Reserve UEFI memory map
		{
			const efi::EFI_MEMORY_DESCRIPTOR *descriptor = memoryMap->memoryMap;
			const uint64_t descriptorSize = memoryMap->descriptorSize;
			uint64_t numberOfPages = 0;

			uint64_t startOfMemoryMap = (uint64_t)memoryMap->memoryMap;
			uint64_t endOfMemoryMap = startOfMemoryMap + memoryMap->memoryMapSize;
			uint64_t offset = startOfMemoryMap;

			while (offset < endOfMemoryMap) {
				let kind = (descriptor->Type == efi::EfiConventionalMemory) ? PAGE_FREE : PAGE_RESERVED;
				let where = DOWN_BYTES_TO_PAGES(descriptor->PhysicalStart);
				let steps = descriptor->NumberOfPages;

				if (kind == PAGE_RESERVED)
					serialPrintf(u8"[physical] Reserve %u pages from %u page\n", steps, where);

				uint64_t i = where;
				while (i < steps) {
					buffer[i] = kind;
					i++;
				}

				offset += descriptorSize;
				descriptor = efi::getNextDescriptor(descriptor, descriptorSize);
			}
		}

		reserveOnePage(PHYSICAL_NOT_FOUND); // Ensure special case

		// Reserve bootloader buffer
		{
			uint64_t physical = params->physicalBuffer;
			serialPrintf(u8"[physical] Reserve %u bytes at %u of bootloader buffer\n", params->physicalBytes,
						 physical);
			uint64_t i = DOWN_BYTES_TO_PAGES(params->physicalBytes) + 1;
			while (i > 0) {
				i--;
				reserveOnePage(physical + i * PAGE_SIZE);
			}
		}

		{
			uint64_t available = 0;
			uint64_t i = count;
			while (i > 0) {
				i--;
				if (buffer[i] == PAGE_FREE)
					available++;
			}

			serialPrintf(u8"[physical] Available %u of %u physical pages\n", available, count);
		}
	}

	// Note: takes real physical address, without mapped offset
	static function reserveOnePage(uint64_t physical) {
		let where = DOWN_BYTES_TO_PAGES(physical);
		buffer[where] = PAGE_RESERVED;
		if (last == where)
			last = PHYSICAL_NOT_FOUND;
	}

// Must test as (buffer[_] != PAGE_FREE) == PAGE_RESERVED
#undef PAGE_RESERVED

	// Returns physical 64-bit address, not page number
	static uint64_t allocateOnePage() {
		if (last != PHYSICAL_NOT_FOUND) {
			let where = last;
			last = PHYSICAL_NOT_FOUND;
			return where * PAGE_SIZE + (uint64_t)WholePhysicalStart;
		}

		uint64_t i = 0;
		// TODO fast large-step outer search with uint64_t
		uint64_t steps = count; // Avoid non-register access
		while (i < steps) {
			if (buffer[i] == PAGE_FREE) {
				buffer[i] = PAGE_FULL;
				return i * PAGE_SIZE + (uint64_t)WholePhysicalStart;
			}

			i++;
		}

		serialPrintf(u8"[physical] allocateOnePage == PHYSICAL_NOT_FOUND\n");
		return PHYSICAL_NOT_FOUND;
	}

	static uint64_t allocateOnePagePreZeroed() {
		var result = allocateOnePage();
		if (result != PHYSICAL_NOT_FOUND)
			memset((void *)result, 0, PAGE_SIZE);
		return result;
	}

	// TODO faster
	static uint64_t allocatePages(uint64_t pages) {
		// 0 or 1 == 1 page anyway
		if (pages < 2)
			return allocateOnePage();
		last = PHYSICAL_NOT_FOUND;

		// Find largest free block
		uint64_t largest = 0;
		uint64_t largestAt = 0;

		{
			uint64_t current = 0;
			uint64_t currentAt = 0;
			uint64_t i = 0;
			uint64_t steps = count; // Avoid non-register access
			// TODO ^ same for buffer[]
			// TODO probably uint32_t is completely enough to represent pages
			while (i < steps) {
				if (buffer[i] == PAGE_FREE) {
					if (current == 0)
						currentAt = i;
					current++;
				}

				if (current >= pages) {
					largest = current;
					largestAt = currentAt;
					break;
				}

				if (buffer[i] != PAGE_FREE) {
					current = 0;
				}

				i++;
			}
		}

		if (largest >= pages) {
			uint64_t i = pages;
			while (i > 0) {
				i--;
				buffer[largestAt + i] = PAGE_FULL;
			}
			return largestAt * PAGE_SIZE + (uint64_t)WholePhysicalStart;
		}

		serialPrintf(u8"[physical] allocatePages %u pages == PHYSICAL_NOT_FOUND\n", pages);
		return PHYSICAL_NOT_FOUND;
	}

	// Current granularity: 4096 bytes
	static uint64_t allocateBytes(uint64_t bytes) {
		// TODO
		return allocatePages(DOWN_BYTES_TO_PAGES(bytes) + 1);
	}

	static function freeOnePage(uint64_t physical) {
		physical -= (uint64_t)WholePhysicalStart;
		let where = DOWN_BYTES_TO_PAGES(physical);
		// Cosmic rays
		if (buffer[where] == PAGE_FULL)
			buffer[where] = PAGE_FREE;
		last = where;
	}

	static function freePages(uint64_t physical, uint64_t pages) {
		// TODO
		freeOnePage(physical);
	}

	static function freeBytes(uint64_t physical, uint64_t bytes) {
		// TODO
		freePages(physical, DOWN_BYTES_TO_PAGES(bytes) + 1);
	}
};

#undef PAGE_FULL
#undef PAGE_FREE
#undef PAGE_MASK
uint8_t *PhysicalAllocator::buffer;
uint64_t PhysicalAllocator::count;
uint64_t PhysicalAllocator::last;
