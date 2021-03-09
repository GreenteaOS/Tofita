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

#include "../ntdll.dll/ntdll.hpp"
#include "../tofita32.dll/tofita32.hpp"

#ifndef KERNEL32_DLL
	#ifdef bit64
		#define KERNEL32_DLL __declspec(dllimport)
	#else
		#define KERNEL32_DLL __declspec(dllimport) __stdcall
	#endif
#endif

extern "C" {
KERNEL32_DLL void ExitProcess(uint32_t exitCode);
KERNEL32_DLL wapi::HLocal LocalAlloc(uint32_t flags, size_t bytes);
KERNEL32_DLL wapi::HGlobal GlobalAlloc(uint32_t flags, size_t bytes);
KERNEL32_DLL void *HeapAlloc(wapi::Handle heap, uint32_t flags, size_t bytes);
}
