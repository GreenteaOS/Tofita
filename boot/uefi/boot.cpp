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

// Boot loader: enters efi_main, reads all UEFI data and starts kernel loader

#include <stdint.h>

// Code Hexa-style for easier porting

#define nullptr ((void*)0)
#define null nullptr
#define function void

typedef void *EFI_HANDLE;
typedef void VOID;

#include <stddef.h>
#include <stdbool.h>
#include <stdarg.h>

extern const uint8_t binFont[];
extern const uint8_t binFontBitmap[];
extern const uint8_t binLeavesBitmap[];
int64_t _fltused = 0; // @keep

EFI_HANDLE imageHandle = nullptr;
struct EFI_SYSTEM_TABLE_;
struct EFI_SYSTEM_TABLE_
*systemTable = nullptr;

#define Virtual_$Any_ void
#define Physical_$Any_ void
#define ArrayByValue_$uint16_t$Any_ void

#define HEAP_ZERO_MEMORY 0
#define stdout 0
#define HANDLE void*

static void ExitProcess(int32_t x) {
}

static void wprintf(const wchar_t* x, const void* y) {
}

static void fflush(void* x) {}

static void free(void* x) {}

uint64_t kstrlen_(const uint8_t*);
static uint32_t strlen(const char *x) {
	return kstrlen_((const uint8_t *)x);
}

static void printf(const void* x,...) {}
#define macro_serialPrintf(print_, ...) serialPrintf((const wchar_t *)print_->_->utf16_(print_), ## __VA_ARGS__)

#define HEAP_C 655360
static uint8_t heap[HEAP_C] = {0};
static uint64_t heapOffset = 0;
static void* HeapAlloc(int8_t x,int8_t u, uint64_t size) {
	// size = ((size - 1) | 7) + 1; // Align by 8
	size = ((size - 1) | 15) + 1; // Align by 16
	if (size < 16) size = 16;
	heapOffset += 8;
	heapOffset += size;
	if (heapOffset >= HEAP_C) {
		//serialPrint_(L"Heap overflow\n");
		//serialPrint_(L"Heap overflow\n");
		//serialPrint_(L"Heap overflow\n");
		while (1) {};
	}
	return &heap[heapOffset - size];
}

#define WriteFile(x,y,z,t,r) {}
#define FlushFileBuffers(x) {}
#define GetStdHandle(x) 0

void *tmemcpy(void *dest, const void *src, uint64_t count);
void memcpy(void* x,const void* y,uint64_t z) {
	tmemcpy(x,y,z);
}

#define GetProcessHeap() 0
#define DWORD uint32_t
#define HEXA_NO_DEFAULT_INCLUDES

// CR3 trampoline
void __attribute__((fastcall))
trampolineCR3(volatile uint64_t kernelParams, volatile uint64_t pml4, volatile uint64_t stack,
			  volatile uint64_t entry);

#include "../uefi.c"

void *tmemcpy(void *dest, const void *src, uint64_t count) {
	uint8_t *dst8 = (uint8_t *)dest;
	const uint8_t *src8 = (const uint8_t *)src;

	while (count--) {
		*dst8++ = *src8++;
	}

	return dest;
}

void *memset(void *dest, int32_t e, uint64_t len) {
	uint8_t *d = (uint8_t *)dest;
	for (uint64_t i = 0; i < len; i++, d++) {
		*d = e;
	}
	return dest;
}

// Entry point
uint64_t efi_main(void* imageHandle__, void *systemTable__) {
	imageHandle = imageHandle__;
	systemTable = (EFI_SYSTEM_TABLE_*)systemTable__;
	for (uint64_t i = 0; i < HEAP_C; i++) heap[i] = 0;
	HEXA_MAIN(0, nullptr);
	return 0;
}
