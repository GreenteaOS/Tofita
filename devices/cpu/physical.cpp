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

// 31 is 0b11111 which is a lot of non-zero bits
#define PHYSICAL_NOT_FOUND 0
#define BUSY 31
#define FREE 0
#define POS_TO_PAGE(pos) ((pos) / PAGE_SIZE)
// POS_TO_PAGE(123) == 0 (.floor)
#define DOWN_BYTES_TO_PAGES(bytes) ((bytes) >> 12)
// 4095 >> 12 == 0
// 4096 >> 12 == 1
// 8191 >> 12 == 1
// 8192 >> 12 == 2

class PhysicalAllocator {
private:
	static uint8_t* buffer; // TODO use bits, not bytes
	static uint64_t pages;
	static uint64_t last;
public:
	static function init(const EfiMemoryMap *memoryMap, uint64_t bufferVirtual, uint64_t ramPages) {
		buffer = (uint8_t*)bufferVirtual;
		pages = ramPages;
		last = PHYSICAL_NOT_FOUND;

		{
			const efi::EFI_MEMORY_DESCRIPTOR *descriptor = memoryMap->memoryMap;
			const uint64_t descriptorSize = memoryMap->descriptorSize;
			uint64_t numberOfPages = 0;

			uint64_t startOfMemoryMap = (uint64_t)memoryMap->memoryMap;
			uint64_t endOfMemoryMap = startOfMemoryMap + memoryMap->memoryMapSize;
			uint64_t offset = startOfMemoryMap;

			while (offset < endOfMemoryMap) {
				let kind = (descriptor->Type == efi::EfiConventionalMemory)? FREE : BUSY;
				let where = DOWN_BYTES_TO_PAGES(descriptor->PhysicalStart);
				let steps = descriptor->NumberOfPages;

				uint64_t i = where;
				while (i < steps) {
					buffer[i] = kind;
					i++;
				}

				offset += descriptorSize;
				descriptor = pages::getNextDescriptor(descriptor, descriptorSize);
			}
		}

		reserveOnePage(PHYSICAL_NOT_FOUND); // Ensure special case

		// TODO reserve large buffer
	}

	static function reserveOnePage(uint64_t physical) {
		let where = DOWN_BYTES_TO_PAGES(physical);
		buffer[where] = BUSY;
		if (last == where) last = PHYSICAL_NOT_FOUND;
	}

	// Returns physical 64-bit address, not page number
	static uint64_t allocateOnePage() {
		if (last != PHYSICAL_NOT_FOUND) {
			let where = last * PAGE_SIZE;
			last = PHYSICAL_NOT_FOUND;
			return where;
		}

		uint64_t i = 0;
		// TODO fast large-step outer search
		uint64_t steps = pages; // Avoid non-register access
		while (i < steps) {
			if (buffer[i] == FREE) {
				buffer[i] = BUSY;
				return i * PAGE_SIZE;
			}

			i++;
		}

		return PHYSICAL_NOT_FOUND;
	}

	static function freeOnePage(uint64_t physical) {
		let where = DOWN_BYTES_TO_PAGES(physical);
		buffer[where] = FREE;
		last = where;
	}
};

#undef BUSY
#undef FREE
#undef POS_TO_PAGE
uint8_t* PhysicalAllocator::buffer;
uint64_t PhysicalAllocator::pages;
uint64_t PhysicalAllocator::last;
