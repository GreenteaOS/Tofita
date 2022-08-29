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

// Used only for STB

#define free libc_free
#define malloc libc_malloc
#define realloc libc_realloc
#define memset libc_memset
#define abs libc_abs
#define pow libc_pow

extern "C" uint64_t PhysicalAllocator_$allocateBytes_(uint64_t);

function libc_free(void *addr) {
	// Do nothing for now
	serialPrintf(L"[libc] libc_free\n");
}

void *libc_malloc(uint64_t size) {
	serialPrintf(L"[libc] libc_malloc of size %u\n", size);
	return (void *)(kernelParams? PhysicalAllocator_$allocateBytes_(size) : PhysicalAllocator::allocateBytes(size));
}

void *libc_realloc(void *addr, uint64_t size) {
	serialPrintf(L"[libc] libc_realloc %u of size %u\n", addr, size);

	{
		uint64_t result = kernelParams? PhysicalAllocator_$allocateBytes_(size) : PhysicalAllocator::allocateBytes(size);
		return (void *)result;
	}
}

void *libc_memset(void *b, int32_t c, int32_t len) {
	int32_t i;
	uint8_t *p = (uint8_t *)b;
	i = 0;
	while (len > 0) {
		*p = c;
		p++;
		len--;
	}
	return (b);
}

int32_t libc_abs(int32_t v) {
	return v * ((v < 0) * (-1) + (v > 0));
}

int32_t libc_pow(int32_t a, int32_t n) {
	int32_t r = 1;
	while (n > 0) {
		if (n & 1)
			r *= a;
		a *= a;
		n >>= 1;
	}
	return r;
}

function libc_memcpy(void *dest, const void *src, uint64_t n) {
	const uint8_t *csrc = (uint8_t *)src;
	uint8_t *cdest = (uint8_t *)dest;
	for (int32_t i = 0; i < n; i++)
		cdest[i] = csrc[i];
}

function memcpy(void *dest, const void *src, uint64_t n) {
	libc_memcpy(dest, src, n);
}
