// The Tofita Kernel
// Copyright (C) 2020 Oleh Petrenko
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

#ifdef __cplusplus
	constexpr auto null = nullptr;
	#define externC extern "C"
#else
	#define externC
	#define nullptr ((void*)0)
	#define null nullptr
#endif

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

#if 0
extern "C++"
template<typename T, uint32_t s, uint32_t t> struct checkSize {
	static_assert(s == t, "wrong size");
};
#define SIZEOF(T, SIZE) namespace sizeofChecks { checkSize<T, sizeof(T), SIZE> __SIZEOF__##T; }

// wchar_t is UTF-16LE with -fshort-wchar
_Static_assert(sizeof(wchar_t) == 2, "bad sizeof");
_Static_assert(sizeof(char16_t) == 2, "bad sizeof");

#define STR_IMPL_(x) #x		// stringify argument
#define STR(x) STR_IMPL_(x) // indirection to expand argument macros

// Constants
#endif
// Start of kernel sections in memory
#define KernelVirtualBase (uint64_t)0xffff800000000000
#define FramebufferStart (KernelVirtualBase + 768 * 1024 * 1024)
// TODO: no need for mapping FramebufferStart if WholePhysicalStart used
// TODO proper dynamically computed numbers

// Mapping of 1:1 of physical memory as virtual = physical + WholePhysicalStart
// Note: Mapping is done on-demand per-page for faster loading
#define WholePhysicalStart (FramebufferStart + 128 * 1024 * 1024)
#define PAGE_SIZE 4096 // 4 KiB

#if 0
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
#endif
