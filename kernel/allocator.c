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

// Very basic allocator within pre-defined buffer, single heap for all kernel memory

uint64_t _allocatorBase = 0;
uint64_t _allocatorSize = 0;
uint64_t _allocatorOffset = 0;
uint64_t _allocatorDoubleFreeProtector = 0;

function initAllocatorForBuffer(uint64_t size, uint64_t base) {
	_allocatorBase = base;
	_allocatorSize = size;
}

void* allocateFromBuffer(uint64_t size) {
	uint64_t result = _allocatorBase + _allocatorOffset;
	_allocatorOffset += size;

	// Invalidate protector
	if (_allocatorDoubleFreeProtector == result) _allocatorDoubleFreeProtector = 0;
	return (void*)result;
}

// Currently deallocates only latest (if possible) allocation
function freeFromBuffer(uint64_t size, void* base) {
	uint64_t address = (uint64_t)base;
	if (_allocatorDoubleFreeProtector == address) {
		serialPrintln(
			u8"[allocator.freeFromBuffer] <ERROR> (non-fatal, but code is surely broken): "
			"double-free detected and avoided, no memory ruined, report to the kernel developer!"
		);
		return ; // Phew!!!
	}

	if (address == (_allocatorBase + _allocatorOffset - size)) {
		_allocatorOffset -= size;
	}
}
