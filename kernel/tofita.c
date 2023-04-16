// The Tofita Kernel
// Copyright (C) 2020-2023 Oleh Petrenko
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

#include <stddef.h>
#include <stdint.h>

void *malloc(uint64_t bytes) asm("malloc");
static void* HeapAllocAt(size_t lineNumber, char const* filename, char const* functionName, int8_t x,int8_t u, uint64_t size);
void *malloc(uint64_t bytes) {
	return (void *)HeapAllocAt(1, "unsafe", "unsafe", 1, 1, bytes);
	// TODO return (void *)PhysicalAllocator_$allocateBytes_(((bytes - 1) | 15) + 1);
};
#include "../kernel-diff/kernel.c"
