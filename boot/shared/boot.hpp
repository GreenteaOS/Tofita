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

#pragma once

// Code Hexa-style for easier porting

#define null nullptr
#define let const auto
#define var auto
#define function void
#define UInt64 uint64_t
#define Int64 int64_t
#define UInt32 uint32_t
#define Int32 int32_t
#define UInt8 uint8_t
#define Int8 int8_t
#define Void void
#define Pointer(to) to*

// Data transferred from UEFI application into kernel loader

struct EfiMemoryMap {
	EFI_MEMORY_DESCRIPTOR *memoryMap;
	uint64_t memoryMapSize;
	uint64_t mapKey;
	uint64_t descriptorSize;
	uint32_t descriptorVersion;
};

struct Framebuffer {
	uint64_t base; // virtual address
	uint64_t physical; // physical address
	uint32_t size; // in bytes
	uint16_t width;
	uint16_t height;
};

struct RamDisk {
	uint64_t base; // virtual address
	uint64_t physical; // physical address
	uint32_t size; // in bytes
};

#define STR_IMPL_(x) #x      // stringify argument
#define STR(x) STR_IMPL_(x)  // indirection to expand argument macros

// Start of kernel sections in memory, see loader.ld
#define KernelVirtualBase 0xffff800000000000
#define EfiVirtualBase (KernelVirtualBase + 0x40000000)
#define FramebufferStart (EfiVirtualBase + 0x40000000)
#define RamdiskStart (FramebufferStart + 0x45000000)
// TODO proper dynamically computed numbers

// Mapping of 1:1 of physical memory as virtual = physical + WholePhysicalStart
// Note: Mapping is done on-demand per-page for faster loading
#define WholePhysicalStart (RamdiskStart + 0x15000000)
#define PAGE_SIZE 4096 // 4 KiB

struct KernelParams {
	uint64_t pml4;
	int32_t lastPageIndexCache;
	uint64_t stack;
	uint64_t physical; // physical address of where kernel loader placed
	EFI_HANDLE imageHandle;
	EfiMemoryMap efiMemoryMap;
	EFI_RUNTIME_SERVICES *efiRuntimeServices = null;
	Framebuffer framebuffer;
	RamDisk ramdisk;
	// Simple memory buffer for in-kernel allocations
	uint64_t bufferSize = 0;
	uint64_t buffer = 0;
	uint64_t acpiTablePhysical = 0;
	uint64_t ramBytes = 0;
	// uint8_t isLiveBootWithInstaller; == 1 == 0, don't show logon screen (and don't allow to login!
	// in the sense that it looks like a security hole like "let it in without entering a login-password")
};

typedef function (*InitKernel)(const KernelParams *);
// TODO ^ fix that in quake
typedef function (*InitKernelTrampoline)(uint64_t kernelParams, uint64_t pml4, uint64_t stack, uint64_t entry);
