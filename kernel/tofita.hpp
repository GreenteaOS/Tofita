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

#include "syscalls/syscalls.hpp"
#include "../devices/cpu/pages.hpp"
#include "../devices/cpu/interrupts.hpp"
#include "../devices/cpu/seh.hpp"
#include "../devices/cpu/exceptions.hpp"
#include "process.hpp"
#include "gui/quake.hpp"

// Globals

// For syscalls & scheduler
volatile pages::PageEntry *volatile pml4kernelThread = null;

// Avoids PIT-triggered rendering
// This is not a best solution
volatile bool haveToRender = true;

uint64_t uptimeMilliseconds = 0;

// Forward for global usage
function drawVibrancedRectangle(int16_t x, int16_t y, uint16_t width, uint16_t height, bool dark = true);
struct Pixel32;
uint16_t drawAsciiText(const char8_t *text, double x, int16_t y, Pixel32 color);
function quakePrintf(const char8_t *c, ...);
function guiThread();
function kernelThread();
function guiThreadStart();
function kernelThreadStart();
function kernelThreadLoop();
