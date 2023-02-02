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

#include <stdint.h>
#include <stddef.h>

#ifdef bit64
	#define NTDLL32_DLL __declspec(dllexport)
#else
	#define NTDLL32_DLL __declspec(dllexport) __stdcall
#endif

#include "ntdll.h"

uint64_t KiFastSystemCall(uint64_t rcx, uint64_t rdx) {
	// TODO return tofitaFastSystemCall((TofitaSyscalls)rcx, rdx);
	return 0;
}

#define tofitaDebugLog(...) {};
const wchar_t stubText[] = L"STUB --> called %S with value %u";
#define STUB(name) uint32_t name(void* value) { tofitaDebugLog(stubText, (uint64_t)L"" #name, (uint64_t)value); return 0; }
/*
STUB(CloseHandle)
*/
STUB(free)

#ifdef bit64
	#define CONV
#else // __cdecl __stdcall
	#define CONV __cdecl
#endif

// DWORD == uint32_t // TODO write down all types sizes NTAPI callbacks etc
typedef int32_t(CONV *DllEntry)(void* hinstDLL, uint32_t fdwReason, void* lpvReserved);
typedef int32_t(CONV *ExeEntry)(void* hInstance, void* hPrev, void* pCmdLine, int nCmdShow);

#define HEXA_NO_DEFAULT_INCLUDES
#define HEXA_MAIN mainHexa
#define HEAP_C 4096 * 16
static volatile uint8_t heap[HEAP_C] = {0};
static volatile uint64_t heapOffset = 0;
static void* HeapAlloc(volatile int8_t x,volatile void* u, volatile uint64_t size) {
	// size = ((size - 1) | 7) + 1; // Align by 8
	size = ((size - 1) | 15) + 1; // Align by 16
	if (size < 16) size = 16;
	heapOffset += 8;
	heapOffset += size;
	if (heapOffset >= HEAP_C) {
		while (1) {};
	}
	void* result = (void *)&heap[heapOffset - size];
	return result;
}
static void* HeapAllocAt(size_t lineNumber, char const* filename, char const* functionName, volatile int8_t x,volatile void* u, volatile uint64_t size) {
	return HeapAlloc(x, u, size);
}
#define HeapAlloc(a, b, c) HeapAllocAt(__LINE__, __FILE__, __func__, a, b, c)
#define HEXA_NEW(z) HeapAllocAt(__LINE__, __FILE__, __func__, 0,nullptr,z)

#define strlen(z) 0
void *tmemcpy(void *dest, const void *src, uint64_t count) {
	uint8_t *dst8 = (uint8_t *)dest;
	const uint8_t *src8 = (const uint8_t *)src;

	while (count--) {
		*dst8++ = *src8++;
	}

	return dest;
}
#define memcpy(z,u,x) tmemcpy(z,u,x)
#define wprintf(z,...) {}
#define HEAP_ZERO_MEMORY ((void*)0)
#define GetProcessHeap() 0
#define fflush(z) {}
#define free(z) {}
#define HEXA_UNREACHABLE(z) {}
#define stdout ((void*)0)
int64_t _fltused = 0;

#ifdef bit64
#else
int64_t __alldiv() asm("__alldiv");
int64_t __alldiv() { return 0; } // TODO

int64_t __allrem() asm("__allrem");
int64_t __allrem() { return 0; } // TODO

uint64_t __aulldiv() asm("__aulldiv");
uint64_t __aulldiv() { return 0; } // TODO

uint64_t __aullrem() asm("__aullrem");
uint64_t __aullrem() { return 0; } // TODO

void _memset() asm("_memset");
void _memset() { } // TODO TODO TODO
#endif

uint64_t hexa_startup;

#define TRACER() tofitaDebugLog_(L"~TRACER~ [%s:%d]\n", __func__, __LINE__);

#ifdef bit64
	#include "ntdll.64.c"
#else
	#include "ntdll.32.c"
#endif

// TODO Hehe just use uint32_t for PIDs
void __attribute__((fastcall)) greenteaosIsTheBest(size_t startup) asm("greenteaosIsTheBest");
void __attribute__((fastcall)) greenteaosIsTheBest(size_t startup) {
	_wcmdln = L"_wcmdln";
	for (uint64_t i = 0; i < HEAP_C; i++) heap[i] = 0;
	heapOffset = 0;
	tofitaDebugLog_(L"HEXA_MAIN", 0, 0);
	hexa_startup = startup;
	HEXA_MAIN(0, nullptr);
}
