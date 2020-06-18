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

// Very basic allocator within pre-defined buffer, single heap for all kernel memory

volatile uint64_t _allocatorBase = 0;
volatile uint64_t _allocatorOffset = 0;
uint8_t _allocatorBuffer[32 * 1024 * 1024] = {0};

function initAllocatorForBuffer() {
	uint8_t* base = _allocatorBuffer;
	_allocatorBase = (uint64_t)base;
	_allocatorOffset = 0;
	tmemset((void*)_allocatorBase, 0, sizeof(_allocatorBuffer));
}

void* allocateFromBuffer(uint64_t size) {
	serialPrintf(u8"[allocator] allocateFromBuffer of size %u\n", size);
	uint64_t result = _allocatorBase + _allocatorOffset;
	_allocatorOffset += size;

	return (void*)result;
}
