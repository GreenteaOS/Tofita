#include <efi.h>
#include <efilib.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdarg.h>

#include "../shared/boot.h"
#include "../../devices/serial/log.c"
#include "../shared/paging.c"

extern char _binary__mnt_r_tofita_tofita_img_start;

static void startKernel(const KernelParams *params) {
	InitKernel start = (InitKernel) KERNEL_VIRTUAL_BASE;
	serialPrint("[[boot]] kernel start address = ");
	serialPrintHex((uint64_t) (start));
	serialPrint("\r\n");
	start(params);
}

void startTofitaKernel(const KernelParams *params) {
	serialPrintln("[[boot]] begin: paging");

	char *kernelImage = (char *) &_binary__mnt_r_tofita_tofita_img_start;

	KernelParams newParams = *params;

	enablePaging(kernelImage, &newParams.efiMemoryMap, &newParams.framebuffer);
	serialPrintln("[[boot]] done: paging enabled");

	newParams.framebuffer.base = (void *) FRAMEBUFFER_START;

	serialPrintln("[[boot]] entering Tofita");
	startKernel(&newParams);
}
