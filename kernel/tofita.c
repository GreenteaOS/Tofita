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

#ifdef __cplusplus
extern "C" {
#else
	#define nullptr ((void*)0)
#endif
#include <efi.h>
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
#include "../devices/cpu/rdtsc.c"
#include "../devices/screen/framebuffer.c"
#include "../devices/ps2/keyboard.c"
#include "../devices/ps2/mouse.c"
#include "../devices/ps2/polling.c"
#include "../devices/cpu/fallback.c"
#include "ramdisk.c"
#include "formats/cur/cur.c"
#include "formats/bmp/bmp.c"
#include "gui/blur.c"
#include "gui/compositor.c"

// STB library
#define STBI_NO_SIMD
#define STBI_NO_STDIO
#define STB_IMAGE_IMPLEMENTATION
#define STBI_ONLY_PNG
#include "formats/stb_image/libc.c"
#include "formats/stb_image/stb_image.h"
#include "formats/stb_image/unlibc.c"

void (*keyDownHandler)(uint8_t) = nullptr;
void handleKeyDown(uint8_t key) {
	if (keyDownHandler) keyDownHandler(key);
}

void kernelMain(KernelParams *params) {
	serialPrintln("<Tofita> kernel loaded and operational");

	initAllocatorForBuffer(params->bufferSize, params->buffer);
	setFramebuffer(&params->framebuffer);
	setRamDisk(&params->ramdisk);

	if (sizeof(uint8_t*) == 4) serialPrintln("<Tofita> void*: 4 bytes");
	if (sizeof(uint8_t*) == 8) serialPrintln("<Tofita> void*: 8 bytes");

	#ifdef __cplusplus
		serialPrintln("<Tofita> __cplusplus");
	#else
		serialPrintln("<Tofita> !__cplusplus");
	#endif

	#if defined(__clang__)
		serialPrintln("<Tofita> __clang__");
	#elif defined(__GNUC__) || defined(__GNUG__)
		serialPrintln("<Tofita> __GNUC__");
	#elif defined(_MSC_VER)
		serialPrintln("<Tofita> _MSC_VER");
	#endif

	enablePS2Mouse();

	initializeCompositor();

	{
		RamDiskAsset a = getRamDiskAsset("hello.bmp");
		Bitmap32* bmp = loadBmp24(&a);
		setWallpaper(bmp, Center);
	}

	RamDiskAsset asset = getRamDiskAsset("cursors\\normal.cur");
	serialPrintf("Asset 'cursors\\normal.cur' %d bytes at %d\r\n", asset.size, asset.data);
	struct Cursor *cur = loadCursor(&asset);

	Pixel32 color;
	color.color = 0x55AA9944;
	drawRectangleWithAlpha(color, 300, 100, 300, 100);

	while (true) {
		// Poll PS/2 devices
		haveToRender = haveToRender || (pollPS2Devices() == PollingPS2SomethingHappened);

		if (mouseX > _framebuffer->width) mouseX = _framebuffer->width;
		if (mouseY > _framebuffer->height) mouseY = _framebuffer->height;

		if (haveToRender == 0) continue ;
		haveToRender = 0;

		composite();

		drawVibrancedRectangle(mouseX - 150, mouseY - 150, 300, 300);

		drawCursor(cur, mouseX, mouseY);

		copyToScreen();
	}
}
#ifdef __cplusplus
}
#endif
