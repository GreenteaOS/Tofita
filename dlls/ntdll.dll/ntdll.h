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

#ifndef NTDLL32_DLL
	#ifdef bit64
		#define NTDLL32_DLL __declspec(dllimport)
	#else
		#define NTDLL32_DLL __declspec(dllimport) __stdcall
	#endif
	extern const wchar_t* _wcmdln asm("_wcmdln");
#else
	// TODO describe the purpose
	__declspec(dllexport) const wchar_t* _wcmdln asm("_wcmdln") = L"_wcmdln TODO";
#endif

NTDLL32_DLL uint32_t free(void* value) asm("free");
