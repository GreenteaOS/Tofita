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

#define TOFITA32_DLL __declspec(dllexport)
#include "tofita32.hpp"

uint64_t KiFastSystemCall(TofitaSyscalls rcx, uint64_t rdx, uint64_t r8, uint64_t r9) {
	asm volatile("int $0x80" ::);
	asm volatile("ret" ::);
}

void tofitaExitProcess(uint32_t exitCode) {
	KiFastSystemCall(TofitaSyscalls::ExitProcess, exitCode);
}
