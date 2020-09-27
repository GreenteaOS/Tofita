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
#include "../types.hpp"

#ifndef NTDLL32_DLL
#define NTDLL32_DLL __declspec(dllimport)
#endif

// TODO https://en.cppreference.com/w/cpp/numeric/bit_cast
extern "C" {
NTDLL32_DLL uint64_t KiFastSystemCall(uint64_t rcx, uint64_t rdx);
}
