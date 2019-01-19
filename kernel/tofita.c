// The Tofita Kernel
// Copyright (C) 2019  Oleg Petrenko
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

#include <efi.h>
#include <efilib.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdarg.h>

#include "../boot/shared/boot.h"

// Avoids PIT-triggered rendering
// This is not a best solution
uint8_t haveToRender = 1;

#include "../devices/serial/log.c"
#include "allocator.c"
#include "../devices/cpu/interrupts.c"
#include "../devices/screen/framebuffer.c"
#include "../devices/ps2/keyboard.c"
#include "../devices/ps2/mouse.c"
#include "../devices/cpu/fallback.c"
#include "ramdisk.c"
#include "formats/cur/cur.c"

void kernelMain(KernelParams *params) {
	serialPrintln("<Tofita> kernel loaded and operational");

	initAllocatorForBuffer(params->bufferSize, params->buffer);
	setFramebuffer(&params->framebuffer);
	setRamDisk(&params->ramdisk);

	if (sizeof(uint8_t*) == 4) serialPrintln("<Tofita> void*: 4 bytes");
	if (sizeof(uint8_t*) == 8) serialPrintln("<Tofita> void*: 8 bytes");

	#if defined(__clang__)
		serialPrintln("<Tofita> __clang__");
	#elif defined(__GNUC__) || defined(__GNUG__)
		serialPrintln("<Tofita> __GNUC__");
	#elif defined(_MSC_VER)
		serialPrintln("<Tofita> _MSC_VER");
	#endif

	enableInterrupts();

	{
		RamDiskAsset a = getRamDiskAsset("assets.json");
		serialPrintf("Asset 'assets.json' %d bytes", a.size);
		serialPrintln(a.data);
	}

	{
		RamDiskAsset a = getRamDiskAsset("README.md");
		serialPrintf("Asset 'README.md' %d bytes", a.size);
		serialPrintln(a.data);
	}

	RamDiskAsset asset = getRamDiskAsset("cursors\\normal.cur");
	serialPrintf("Asset 'cursors\\normal.cur' %d bytes", asset.size);
	struct Cursor *cur = loadCursor(&asset);

	Pixel32 color;
	color.color = 0x55AA9944;
	drawRectangleWithAlpha(color, 300, 100, 300, 100);

	while (true) {
		__asm__ volatile("hlt");
		if (haveToRender == 0) continue ;
		haveToRender = 0;

		Pixel32 color;
		color.color = 0xFF0000FF;
		drawRectangle(color, mouseX - 10, mouseY - 10, 20, 20);
		drawCursor(cur, mouseX, mouseY);
	}
}
