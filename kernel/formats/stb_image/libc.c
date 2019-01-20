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

// Used only for STB

#define free libc_free
#define malloc libc_malloc
#define realloc libc_realloc
#define memset libc_memset
#define abs libc_abs
#define pow libc_pow
#define memcpy libc_memcpy

void libc_free(void* addr) {
	// Do nothing for now
	serialPrintf("[libc] libc_free\r\n");
}

void* libc_malloc(size_t size) {
	serialPrintf("[libc] libc_malloc\r\n");

	void* result = allocateFromBuffer(size + 4);

	// Save size information for realloc
	uint32_t* sizes = result;
	sizes[0] = size;

	serialPrintf("[libc] libc_malloc done\r\n");
	return result + 4;
}

void* libc_realloc(void* addr, size_t size) {
	serialPrintf("[libc] libc_realloc\r\n");

	// Get old size information
	uint32_t* sizes = addr;
	//uint32_t old = sizes[-1];

	// Alloc
	uint8_t* result = allocateFromBuffer(size + 4);

	// Copy
	//uint8_t* source = addr;
	//if (size < old) old = size;
	//for (uint32_t i = 0; i < old; i++) result[i + 4] = source[i];

	// Save size
	sizes = result;
	sizes[0] = size;

	serialPrintf("[libc] libc_realloc done\r\n");
	return result;
}

void *libc_memset(void *b, int c, int len) {
	serialPrintf("[libc] libc_memset\r\n");
	int i;
	unsigned char *p = b;
	i = 0;
	while (len > 0) {
		*p = c;
		p++;
		len--;
	}
	serialPrintf("[libc] libc_memset done\r\n");
	return(b);
}


int libc_abs(int v) {
	return v * ( (v<0) * (-1) + (v>0) );
}

int libc_pow(int a, int n) {
	int r = 1;
	while (n > 0) {
		if (n & 1)
			r *= a;
		a *= a;
		n >>= 1;
	}
	return r;
}

void libc_memcpy(void *dest, void *src, size_t n) {
	serialPrintf("[libc] libc_memcpy\r\n");
	char *csrc = src;
	char *cdest = dest;
	for (int i = 0; i < n; i++) cdest[i] = csrc[i];
	serialPrintf("[libc] libc_memcpy done\r\n");
}
