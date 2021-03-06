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

#include "../devices/cpu/physical.hpp"
#include "util/String.hpp"
#include "../dlls/wrappers.hpp"
#include "../dlls/types.hpp"
#include "../devices/screen/framebuffer.hpp"
#include "../devices/ps2/mouse.hpp"
#include "../devices/ps2/polling.hpp"
#include "gui/dwm.hpp"
#include "gui/desktop.hpp"
#include "syscalls/syscalls.hpp"
#include "formats/lnk/lnk.hpp"
#include "../devices/cpu/pages.hpp"
#include "../devices/cpu/interrupts.hpp"
#include "../devices/cpu/seh.hpp"
#include "../devices/cpu/exceptions.hpp"
#include "process.hpp"
#include "syscalls/user32/userCall.hpp"
#include "gui/quake.hpp"

// Globals

// For syscalls & scheduler
volatile pages::PageEntry *volatile pml4kernelThread = null;

// Flag to keep in kernel mode and not switch to user mode
volatile bool kernelBusy = false;

// Avoids PIT-triggered rendering
// This is not a best solution
volatile bool haveToRender = true;

uint64_t uptimeMilliseconds = 0;

// Forward for global usage
function drawVibrancedRectangle(int16_t x, int16_t y, uint16_t width, uint16_t height, bool dark = true);
struct Pixel32;

// TODO refactor to .hpp!
uint16_t drawAsciiText(const char8_t *text, double x, int16_t y, Pixel32 color);
function quakePrintf(const wchar_t *c, ...);
extern "C" function guiThread();
extern "C" function kernelThread();
extern "C" function guiThreadStart();
extern "C" function kernelThreadStart();
function kernelThreadLoop();
