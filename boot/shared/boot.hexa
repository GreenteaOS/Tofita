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

#pragma once

// Code Hexa-style for easier porting

constexpr auto null = nullptr;
#define let const auto
#define var auto
typedef void function;
#define UInt64 uint64_t
#define Int64 int64_t
#define UInt32 uint32_t
#define Int32 int32_t
#define UInt8 uint8_t
#define Int8 int8_t
#define Void void
#define Pointer(to) to *

// wchar_t is UTF-16LE with -fshort-wchar
_Static_assert(sizeof(wchar_t) == 2, "bad sizeof");
_Static_assert(sizeof(char16_t) == 2, "bad sizeof");

// Use uint64_t, int32_t, char8_t, etc

#define char do_not_use_such_types_please
#define int do_not_use_such_types_please
#define signed do_not_use_such_types_please
#define short do_not_use_such_types_please
#define size_t do_not_use_such_types_please
#define intptr_t do_not_use_such_types_please
#define uintptr_t do_not_use_such_types_please
#define nullptr do_not_use_such_types_please

#define STR_IMPL_(x) #x		// stringify argument
#define STR(x) STR_IMPL_(x) // indirection to expand argument macros

// Constants

// Start of kernel sections in memory
#define KernelVirtualBase (uint64_t)0xffff800000000000
#define FramebufferStart (KernelVirtualBase + 768 * 1024 * 1024)
// TODO: no need for mapping FramebufferStart if WholePhysicalStart used
// TODO proper dynamically computed numbers

// Mapping of 1:1 of physical memory as virtual = physical + WholePhysicalStart
// Note: Mapping is done on-demand per-page for faster loading
#define WholePhysicalStart (FramebufferStart + 128 * 1024 * 1024)
#define PAGE_SIZE 4096 // 4 KiB

// Helpers

extern "C++" template <typename T> struct Physical {
	uint64_t physical;

	static Physical<T> toPhysical(uint64_t physical) {
		return (Physical<T>)physical;
	};

	T *toVirtual() const {
		uint64_t result = (uint64_t)WholePhysicalStart + physical;
		return (T *)result;
	};

	T *toVirtualOffset(uint64_t offset) const {
		uint64_t result = (uint64_t)WholePhysicalStart + physical + offset;
		return (T *)result;
	};
};

_Static_assert(sizeof(Physical<char8_t>) == sizeof(uint64_t), "physical address has to have 64 bits");

// Data transferred from UEFI application into kernel loader

struct EfiMemoryMap {
	efi::EFI_MEMORY_DESCRIPTOR *memoryMap;
	uint64_t memoryMapSize;
	uint64_t mapKey;
	uint64_t descriptorSize;
	uint32_t descriptorVersion;
};

struct Framebuffer {
	uint64_t base;	   // virtual address
	uint64_t physical; // physical address
	uint32_t size;	   // in bytes
	uint32_t width;
	uint32_t height;
	uint32_t pixelsPerScanLine;
};

struct RamDisk {
	uint64_t base;	   // virtual address
	uint64_t physical; // physical address
	uint32_t size;	   // in bytes
};

struct KernelParams {
	uint64_t pml4;
	uint64_t stack;
	uint64_t physicalBuffer; // physical address of where kernel loader placed
	uint64_t physicalBytes;
	uint64_t physicalRamBitMaskVirtual; // virtual address of where RAM usage
										// bit-map stored
	efi::EFI_HANDLE imageHandle;
	EfiMemoryMap efiMemoryMap;
	efi::EFI_RUNTIME_SERVICES *efiRuntimeServices = null;
	Framebuffer framebuffer;
	RamDisk ramdisk;
	uint64_t acpiTablePhysical;
	uint64_t ramBytes;

	efi::EFI_TIME time;
	efi::EFI_TIME_CAPABILITIES capabilities;
	// uint8_t isLiveBootWithInstaller; == 1 == 0, don't show logon screen (and
	// don't allow to login! in the sense that it looks like a security hole
	// like "let it in without entering a login-password")
};

typedef function(__fastcall *InitKernel)(const KernelParams *);
// TODO ^ fix that in quake
typedef function(__fastcall *InitKernelTrampoline)(uint64_t kernelParams, uint64_t pml4, uint64_t stack,
												   uint64_t entry);
