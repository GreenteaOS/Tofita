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

#include <stdint.h>
#include <stddef.h>

#ifdef bit64
	#define KERNEL32_DLL __declspec(dllexport)
#else
	#define KERNEL32_DLL __declspec(dllexport) __stdcall
#endif

#include "../ntdll.dll/ntdll.hpp"

extern "C" {





void startup() {
}

#ifdef bit64
__attribute__((fastcall)) void _DllMainCRTStartup() {
	startup();
}
#else
__attribute__((stdcall)) void _DllMainCRTStartup(void *, void *, void *) {
	startup();
}
#endif
}
