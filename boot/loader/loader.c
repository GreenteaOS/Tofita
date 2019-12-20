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

#include "../shared/boot.h"
#include "../../devices/serial/log.c"
#include "../shared/paging.c"
#include "../../kernel/tofita.c"

function startTofitaKernelLoader(const KernelParams *params) {
	serialPrintln("[[boot]] begin: paging");

	uint8_t *kernelImage = (uint8_t *) 0;

	// Copy to stack
	KernelParams newParams = *params;

	enablePaging(kernelImage, &newParams.efiMemoryMap, &newParams.framebuffer, &newParams.ramdisk, &newParams);
	serialPrintln("[[boot]] done: paging enabled");

	// Replace to virtual adresses
	newParams.framebuffer.base = (void *) FramebufferStart;
	newParams.ramdisk.base = (void *) RamdiskStart;
	newParams.acpiTable = (void *) ACPIStart;

	serialPrintln("[[boot]] entering Tofita");
	kernelMain(&newParams);
}
}
