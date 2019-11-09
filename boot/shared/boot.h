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

#define null nullptr
#define let const auto
#define var auto
#define function void

struct EfiMemoryMap {
	EFI_MEMORY_DESCRIPTOR *memoryMap;
	uint64_t memoryMapSize;
	uint64_t mapKey;
	uint64_t descriptorSize;
	uint32_t descriptorVersion;
};

struct Framebuffer {
	void *base; // physical address
	uint32_t size; // in bytes
	uint16_t width;
	uint16_t height;
};

struct RamDisk {
	void *base; // physical address
	uint32_t size; // in bytes
};

#define STR_IMPL_(x) #x      // stringify argument
#define STR(x) STR_IMPL_(x)  // indirection to expand argument macros

// Start of kernel sections in memory, see loader.ld
#define KernelStart (1 * 1024 * 1024)
#define KernelVirtualBase 0xffff800000000000
#define EfiVirtualBase (KernelVirtualBase + 0x40000000)
#define FramebufferStart (EfiVirtualBase + 0x40000000)
#define RamdiskStart (FramebufferStart + 0x45000000)
#define ACPIStart (RamdiskStart + 0x45000000)
#define PAGE_SIZE 4096  // 4 KiB

struct KernelParams {
	EFI_HANDLE imageHandle;
	EfiMemoryMap efiMemoryMap;
	EFI_RUNTIME_SERVICES *efiRuntimeServices;
	Framebuffer framebuffer;
	RamDisk ramdisk;
	uint64_t bufferSize;
	void* acpiTable; // TODO = nullptr in C++
	void* buffer = null;
	uint64_t ramBytes = 0;
typedef function (*InitKernel)(const KernelParams *);

