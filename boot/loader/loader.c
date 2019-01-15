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

#include "../shared/boot.h"
#include "../../devices/serial/log.c"
#include "../shared/paging.c"

extern uint8_t _binary__mnt_r_tofita_tofita_img_start;

static void startTofitaKernel(const KernelParams *params) {
	InitKernel start = (InitKernel) KernelVirtualBase;
	serialPrint("[[boot]] kernel start address = ");
	serialPrintHex((uint64_t) (start));
	serialPrint("\r\n");
	start(params);
}

void startTofitaKernelLoader(const KernelParams *params) {
	serialPrintln("[[boot]] begin: paging");

	uint8_t *kernelImage = (uint8_t *) &_binary__mnt_r_tofita_tofita_img_start;

	// Copy to stack
	KernelParams newParams = *params;

	enablePaging(kernelImage, &newParams.efiMemoryMap, &newParams.framebuffer, &newParams.ramdisk);
	serialPrintln("[[boot]] done: paging enabled");

	newParams.framebuffer.base = (void *) FramebufferStart;
	newParams.ramdisk.base = (void *) RamdiskStart;

	serialPrintln("[[boot]] entering Tofita");
	startTofitaKernel(&newParams);
}
