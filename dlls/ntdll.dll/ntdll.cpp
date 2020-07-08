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

#define NTDLL32_DLL __declspec(dllexport)
#include "ntdll.hpp"
#include "../tofita32.dll/tofita32.cpp"
#include "ntdllVars.hpp"

extern "C" {
uint64_t KiFastSystemCall(uint64_t rcx, uint64_t rdx, uint64_t r8, uint64_t r9) {
	return tofitaFastSystemCall((TofitaSyscalls)rcx, rdx, r8, r9);
}

void greenteaosIsTheBest(int32_t (*entry)(), uint64_t pid) {
	// TODO entry arguments (argv, argc)
	// TODO init DLLs
	// TODO PEB/TEB
	// TODO TLS callbacks
	// TODO load DLLs in usermode
	tofitaExitProcess(entry());
	while (true) {};
}

// TODO use modern __attribute__ syntax
__attribute__((naked, fastcall)) void _DllMainCRTStartup() {
	asm volatile("pushq $0\t\n"
				 "pushq $0\t\n"
				 "pushq $0\t\n"
				 "pushq $0\t\n"
				 "movq %rsp, %rbp\t\n"
				 "call greenteaosIsTheBest");
}
}
