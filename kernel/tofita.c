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

extern "C" {

#include <efi.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdarg.h>

#include "../boot/shared/boot.h"

// Avoids PIT-triggered rendering
// This is not a best solution
uint8_t haveToRender = 1;

#include "util/Math.cpp"

#include "../devices/cpu/amd64.cpp"
#include "../devices/serial/log.c"
#include "../boot/shared/paging.c"
#include "allocator.c"
#include "../devices/cpu/cpuid.cpp"
#include "../devices/cpu/interrupts.c"
#include "../devices/cpu/rdtsc.c"
#include "../devices/screen/framebuffer.c"
#include "../devices/ps2/keyboard.c"
#include "../devices/ps2/mouse.c"
#include "../devices/ps2/polling.c"
#include "../devices/cpu/fallback.c"
#include "../devices/acpi/acpi.cpp"
#include "ramdisk.c"
#include "formats/cur/cur.c"
#include "formats/bmp/bmp.c"
#include "gui/blur.c"
#include "gui/quake.c"
#include "gui/text.cpp"
#include "gui/compositor.c"

// STB library
#define STBI_NO_SIMD
#define STBI_NO_STDIO
#define STB_IMAGE_IMPLEMENTATION
#define STBI_ONLY_PNG
#include "formats/stb_image/libc.c"
#include "formats/stb_image/stb_image.h"
#include "formats/stb_image/unlibc.c"

function (*keyDownHandler)(uint8_t) = null;
function handleKeyDown(uint8_t key) {
	if (keyDownHandler) keyDownHandler(key);
}

const KernelParams *paramsCache = null;
function kernelMain(const KernelParams *params) {
	serialPrintln("<Tofita> GreenteaOS " STR(versionMajor) "." STR(versionMinor) " " versionName " kernel loaded and operational");
	paramsCache = params;
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

	initText();
	initializeCompositor();

	quakePrintf("GreenteaOS " STR(versionMajor) "." STR(versionMinor) " " versionName " loaded and operational\n");

	CPUID cpuid = getCPUID();

	uint32_t megs = Math::round((double)params->ramBytes/(1024.0*1024.0));
	quakePrintf("[CPU] %s %s %d MB RAM\n", cpuid.vendorID, cpuid.brandName, megs);

	if (!acpi::parse(params->acpiTable)) {
		quakePrintf("ACPI is *not* loaded\n");
	} else {
		quakePrintf("ACPI 2.0 is loaded and ready\n");
	}

	quakePrintf("Enter 'help' for commands\n");

	{
		RamDiskAsset a = getRamDiskAsset("hello.bmp");
		Bitmap32* bmp = loadBmp24(&a);
		setWallpaper(bmp, Center);
	}


	mouseX = _framebuffer->width / 2;
	mouseY = _framebuffer->height / 2;

	while (true) {
		// Poll PS/2 devices
		haveToRender = haveToRender || (pollPS2Devices() == PollingPS2SomethingHappened);

		if (mouseX > _framebuffer->width) mouseX = _framebuffer->width;
		if (mouseY > _framebuffer->height) mouseY = _framebuffer->height;
		if (mouseY < 0) mouseY = 0;
		if (mouseX < 0) mouseX = 0;

		if (haveToRender == 0) continue ;
		haveToRender = 0;

		composite();

		copyToScreen();
	}
}
}
