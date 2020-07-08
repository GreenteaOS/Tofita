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

#define KERNEL32_DLL __declspec(dllexport)
#include "kernel32.hpp"
#include "kernel32Vars.hpp"

extern "C" {
void ExitProcess(uint32_t exitCode) {
	tofitaExitProcess(exitCode);
	while (true) {};
}

wapi::HLocal LocalAlloc(uint32_t flags, size_t bytes) {
	auto result = &kernel32::buffer[kernel32::bufferOffset];
	kernel32::bufferOffset += bytes;
	// Align to 8 bytes
	// Because handle has some bits reserved for tags
	// TODO how many bits exactly?
	// TODO also pre-align at CRT startup, cause section may be misaligned
	while (kernel32::bufferOffset % 8 != 0)
		kernel32::bufferOffset++;
	return (wapi::HLocal)result;
}

wapi::HGlobal GlobalAlloc(uint32_t flags, size_t bytes) {
	return (wapi::HGlobal)LocalAlloc(0, bytes);
}

void *HeapAlloc(wapi::Handle heap, uint32_t flags, size_t bytes) {
	return (void *)LocalAlloc(0, bytes);
}

__attribute__((fastcall)) void _DllMainCRTStartup() {}
}
