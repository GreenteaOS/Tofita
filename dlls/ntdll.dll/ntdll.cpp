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

#ifdef bit64
	#define NTDLL32_DLL __declspec(dllexport)
#else
	#define NTDLL32_DLL __declspec(dllexport) __stdcall
#endif

#include "ntdll.hpp"
#include "../tofita32.dll/tofita32.cpp"
#include "ntdllVars.hpp"

extern "C" {
uint64_t KiFastSystemCall(uint64_t rcx, uint64_t rdx) {
	return tofitaFastSystemCall((TofitaSyscalls)rcx, rdx);
}
}

extern "C" {
#ifdef bit64
	// TODO Hehe just use uint32_t for PIDs
	void __attribute__((fastcall)) greenteaosIsTheBest(int32_t (*entry)(), uint64_t pid)
#else // __cdecl __stdcall
	void __attribute__((fastcall)) greenteaosIsTheBest(int32_t (__cdecl *entry)(void* hInstance, void* hPrev, void* pCmdLine, int nCmdShow), uint32_t pid)
#endif
{
	// TODO entry arguments (argv, argc)
	// TODO init DLLs
	// TODO PEB/TEB
	// TODO TLS callbacks
	// TODO load DLLs in usermode

	#ifdef bit64
		tofitaDebugLog(L"64-bit CRT ready for PID %u", pid);
	#else
		tofitaDebugLog(L"32-bit CRT ready for PID %u", pid);
	#endif

	if (entry != nullptr)
		tofitaDebugLog(L"entry != nullptr OK for PID %u at %u", pid, (uint32_t)entry);
	if (entry == nullptr)
		tofitaDebugLog(L"entry == nullptr ERROR for PID %u", pid);
#ifdef bit64
	tofitaExitProcess(entry());
#else
	tofitaExitProcess(entry(nullptr, nullptr, nullptr, 0));
#endif
	while (true) {};
}
}
