// The Tofita Kernel
// Copyright (C) 2021  Oleg Petrenko
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

#include "../dlls/wrappers.hpp"
#include "../dlls/types.hpp"
#include "syscalls/syscalls.hpp"
#include "../devices/cpu/pages.hpp"
#include "../devices/cpu/interrupts.hpp"
#include "../devices/cpu/seh.hpp"
#include "process.hpp"

// For syscalls & scheduler
volatile pages::PageEntry *volatile pml4kernelThread = null;

// Flag to keep in kernel mode and not switch to user mode
volatile bool kernelBusy = false;

// Avoids PIT-triggered rendering
// This is not a best solution
volatile bool haveToRender = true;

uint64_t uptimeMilliseconds = 0;

function quakePrintf(const wchar_t *c, ...);
extern "C" function guiThreadStart();
extern "C" function kernelThreadStart();
