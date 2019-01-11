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

static void startKernel(const KernelParams *params) {
	InitKernel start = (InitKernel) KERNEL_VIRTUAL_BASE;
	serialPrint("[[boot]] kernel start address = ");
	serialPrintHex((uint64_t) (start));
	serialPrint("\r\n");
	start(params);
}

void startTofitaKernel(const KernelParams *params) {
	serialPrintln("[[boot]] begin: paging");

	uint8_t *kernelImage = (uint8_t *) &_binary__mnt_r_tofita_tofita_img_start;

	KernelParams newParams = *params;

	enablePaging(kernelImage, &newParams.efiMemoryMap, &newParams.framebuffer, &newParams.ramdisk);
	serialPrintln("[[boot]] done: paging enabled");

	newParams.framebuffer.base = (void *) FRAMEBUFFER_START;
	newParams.ramdisk.base = (void *) RAMDISK_START;

	serialPrintln("[[boot]] entering Tofita");
	startKernel(&newParams);
}
