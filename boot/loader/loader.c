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


function startTofitaKernelLoader(uint64_t pml4, const KernelParams *params) {

	// Copy to stack
	KernelParams newParams = *params;

	serialPrintln("[[boot]] done: paging enabled");

	// Replace to virtual adresses
	newParams.framebuffer.base = (void *) FramebufferStart;
	newParams.ramdisk.base = (void *) RamdiskStart;

	serialPrintln("[[boot]] entering Tofita");
	kernelMain(&newParams);
}
}
