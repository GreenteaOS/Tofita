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

extern "C" {

namespace efi {
#include <efi.hpp>
}

#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdarg.h>

#include "../boot/shared/boot.hpp"

#define unsigned do_not_use_such_types_please
#define long do_not_use_such_types_please

#include "tofita.hpp"

void *memset(void *dest, int32_t e, uint64_t len) {
	uint8_t *d = (uint8_t *)dest;
	for (uint64_t i = 0; i < len; i++, d++) {
		*d = e;
	}
	return dest;
}

void ___chkstk_ms(){};

#include "util/Math.cpp"
#include "util/String.cpp"

#include "../devices/cpu/cpu.hpp"
#include "../devices/cpu/amd64.cpp"
#include "../devices/serial/log.cpp"
#include "../devices/cpu/cpuid.cpp"
#include "../devices/cpu/spinlock.cpp"
#include "../devices/cpu/exceptions.cpp"
#include "../devices/cpu/interrupts.cpp"
#include "../devices/cpu/seh.cpp"
#include "../devices/cpu/rdtsc.cpp"
#include "../devices/efi/efi.cpp"
#include "../devices/cpu/physical.cpp"
#include "../devices/cpu/pages.cpp"
#include "../devices/screen/framebuffer.cpp"
#include "../devices/ps2/keyboard.cpp"
#include "../devices/ps2/mouse.cpp"
#include "../devices/ps2/polling.cpp"
#include "../devices/cpu/fallback.cpp"
#include "../devices/acpi/acpi.cpp"
#include "formats/exe/exe.hpp"
#include "ramdisk.cpp"
#include "module.cpp"
#include "sandbox.cpp"
#include "user.cpp"
#include "process.cpp"
// STB library
#define STBI_NO_SIMD
#define STBI_NO_STDIO
#define STB_IMAGE_IMPLEMENTATION
#define STBI_ONLY_PNG
#include "formats/stb_image/libc.cpp"
#include "formats/stb_image/stb_image.hpp"
#include "formats/stb_image/unlibc.cpp"
#include "formats/exe/exe.cpp"
#include "formats/cur/cur.cpp"
#include "formats/ico/ico.cpp"
#include "formats/bmp/bmp.cpp"
#include "gui/blur.cpp"
#include "gui/quake.cpp"
#include "gui/text.cpp"
#include "gui/windows.cpp"
#include "gui/compositor.cpp"
#include "gui/dwm.cpp"
#include "syscalls/user32/userCall.cpp"

function handleKeyDown(uint8_t key) {
	if (keyDownHandler)
		keyDownHandler(key);
}

const KernelParams *paramsCache = null;
function kernelInit(const KernelParams *params) {
	serialPrintln(u8"<Tofita> GreenteaOS " versionName " " STR(versionMajor) "." STR(
		versionMinor) " " versionTag " kernel loaded and operational");
	serialPrintf(u8"<Tofita> CR3 points to: %8\n", (uint64_t)params->pml4);
	paramsCache = params;
	PhysicalAllocator::init(params);
	pages::pml4entries = (pages::PageEntry *)((uint64_t)WholePhysicalStart + (uint64_t)(params->pml4));
	setFramebuffer(&params->framebuffer);
	setRamDisk(&params->ramdisk);

	if (sizeof(uint8_t *) == 4)
		serialPrintln(u8"<Tofita> void*: 4 bytes");
	if (sizeof(uint8_t *) == 8)
		serialPrintln(u8"<Tofita> void*: 8 bytes");

#ifdef __cplusplus
	serialPrintln(u8"<Tofita> __cplusplus");
#else
	serialPrintln(u8"<Tofita> !__cplusplus");
#endif

#if defined(__clang__)
	serialPrintln(u8"<Tofita> __clang__");
#elif defined(__GNUC__) || defined(__GNUG__)
	serialPrintln(u8"<Tofita> __GNUC__");
#elif defined(_MSC_VER)
	serialPrintln(u8"<Tofita> _MSC_VER");
#endif

	disablePic();
	enableInterrupts();
	enablePS2Mouse();

	initText();
	initializeCompositor();

	quakePrintf(u8"GreenteaOS " versionName
				" " STR(versionMajor) "." STR(versionMinor) " " versionTag " loaded and operational\n");

	CPUID cpuid = getCPUID();

	uint32_t megs = Math::round((double)params->ramBytes / (1024.0 * 1024.0));
	quakePrintf(u8"[CPU] %s %s %d MB RAM\n", cpuid.vendorID, cpuid.brandName, megs);

	if (!ACPIParser::parse(params->acpiTablePhysical)) {
		quakePrintf(u8"ACPI is *not* loaded\n");
	} else {
		quakePrintf(u8"ACPI 2.0 is loaded and ready\n");
	}

	quakePrintf(u8"Enter 'help' for commands\n");

	{
		RamDiskAsset a = getRamDiskAsset(u8"root/Windows/Web/Wallpaper/Tofita/default.bmp");
		Bitmap32 *bmp = bmp::loadBmp24(&a);
		setWallpaper(bmp, Center);
	}

	mouseX = _framebuffer->width / 2;
	mouseY = _framebuffer->height / 2;

	// Setup scheduling
	currentThread = THREAD_INIT;

	// GUI thread
	{
		memset(&guiThreadFrame, 0, sizeof(InterruptFrame)); // Zeroing
		memset(&guiStack, 0, sizeof(stackSizeForKernelThread)); // Zeroing

		guiThreadFrame.ip = (uint64_t)&guiThreadStart;
		guiThreadFrame.cs = SYS_CODE64_SEL;
		// TODO allocate as physicall memory
		guiThreadFrame.sp = (uint64_t)&guiStack + stackSizeForKernelThread;
		guiThreadFrame.ss = SYS_DATA32_SEL;
	}

	// Main thread
	{
		memset(&kernelThreadFrame, 0, sizeof(InterruptFrame)); // Zeroing
		memset(&kernelStack, 0, sizeof(stackSizeForKernelThread)); // Zeroing

		kernelThreadFrame.ip = (uint64_t)&kernelThreadStart;
		kernelThreadFrame.cs = SYS_CODE64_SEL;
		kernelThreadFrame.sp = (uint64_t)&kernelStack + stackSizeForKernelThread;
		kernelThreadFrame.ss = SYS_DATA32_SEL;
	}

	// Idle process
	{
		memset(&process::processes, 0, sizeof(process::processes)); // Zeroing
		process::Process *idle = &process::processes[0];
		idle->pml4 = pages::pml4entries; // Save CR3 template to idle process
		idle->schedulable = true;		 // At idle schedule to idle process
		idle->present = true;
		idle->syscallToHandle = TofitaSyscalls::Noop;
		process::currentProcess = 0;
		pml4kernelThread = process::processes[0].pml4;
	}

	// Show something before scheduling delay
	composite();
	copyToScreen();
	serialPrintln(u8"<Tofita> [ready for scheduling]");
}

function switchToUserProcess() {
	var next = getNextProcess();

	if (next == 0) {
		markAllProcessessSchedulable();
		next = getNextProcess();
	}

	if (next == 0) {
		// serialPrintln(u8"<Tofita> [halt]");
		// amd64::enableAllInterruptsAndHalt(); // Nothing to do
	}
	// else
	asm volatile("int $0x81"); // yield
							   // TODO
}

function kernelThread() {
	serialPrintln(u8"<Tofita> [kernelThread] thread started");
	while (true) {

		volatile uint64_t index = 1; // Idle process ignored
		while (index < 255) {		 // TODO
			volatile process::Process *process = &process::processes[index];
			if (process->present == true) {
				if (process->syscallToHandle != TofitaSyscalls::Noop) {
					volatile let syscall = process->syscallToHandle;
					process->syscallToHandle = TofitaSyscalls::Noop;
					volatile var frame = &process->frame;

					// Select pml4 to work within current process memory
					// Remember pml4 for proper restore from scheduling
					pml4kernelThread = process->pml4;
					amd64::writeCr3((uint64_t)pml4kernelThread - (uint64_t)WholePhysicalStart);

					// TODO refactor to separate syscall handler per-DLL

					if (syscall == TofitaSyscalls::DebugLog) {
						serialPrintf(u8"[[DebugLog:PID %d]] ", index);
						serialPrintf(u8"[[rcx=%u rdx=%u r8=%u]] ", frame->rcxArg0, frame->rdxArg1,
									 frame->r8Arg2);

						// Note this is still very usafe
						if (probeForReadOkay(frame->rdxArg1, 1))
							serialPrintf((const char8_t *)frame->rdxArg1, frame->r8Arg2, frame->r9Arg3);

						serialPrintf(u8"\n");
						process->schedulable = true;
					} else if (syscall == TofitaSyscalls::ExitProcess) {
						serialPrintf(u8"[[ExitProcess:PID %d]] %d\n", index, frame->rdxArg1);
						process->present = false;

						// Select pml4 of idle process for safety
						pml4kernelThread = process::processes[0].pml4;
						amd64::writeCr3((uint64_t)pml4kernelThread - (uint64_t)WholePhysicalStart);

						// Deallocate process
						process::Process_destroy(process);
					} else if (syscall == TofitaSyscalls::Cpu) {
						serialPrintf(u8"[[Cpu:PID %d]] %d\n", index, frame->rdxArg1);
						quakePrintf(u8"Process #%d closed due to CPU exception #%u\n", index, frame->index);
						process->present = false;

						// Page fault
						if (frame->index == 0x0E)
							quakePrintf(u8"#PF at %8\n", process->cr2PageFaultAddress);
						if (frame->index == 0x0D)
							quakePrintf(u8"#GPF at %8\n", frame->ip);
						if (frame->index == 0x03)
							quakePrintf(u8"#BP at %8\n", frame->ip);

						// Select pml4 of idle process for safety
						pml4kernelThread = process::processes[0].pml4;
						amd64::writeCr3((uint64_t)pml4kernelThread - (uint64_t)WholePhysicalStart);

						// TODO deallocate process
					} else {
						frame->raxReturn = 0; // Must return at least something
						// Note ^ some code in syscall handlers may *read* this value
						// So set it to zero just in case

						if (!userCall::userCallHandled(process, syscall)) {
							// Unknown syscall is no-op
							serialPrintf(u8"[[PID %d]] Unknown or unhandled syscall %d\n", index,
										 frame->rcxArg0);
							frame->raxReturn = 0;
							process->schedulable = true;
						}
					}
				}
			}
			index++;
		}

		switchToUserProcess();
	}
}

__attribute__((naked, fastcall)) function guiThreadStart() {
	asm volatile("pushq $0\t\n"
				 "pushq $0\t\n"
				 "pushq $0\t\n"
				 "pushq $0\t\n"
				 "movq %rsp, %rbp\t\n"
				 "call guiThread");
}

__attribute__((naked, fastcall)) function kernelThreadStart() {
	asm volatile("pushq $0\t\n"
				 "pushq $0\t\n"
				 "pushq $0\t\n"
				 "pushq $0\t\n"
				 "movq %rsp, %rbp\t\n"
				 "call kernelThread");
}

// In case of kernel crash set instruction pointer to here
function kernelThreadLoop() {
	serialPrintln(u8"<Tofita> [looping forever]");
	while (true) {
		asm volatile("pause");
	};
}

function guiThread() {
	serialPrintln(u8"<Tofita> [guiThread] thread started");

	while (true) {
		// Poll PS/2 devices

		// TODO move this to DWM
		if (mouseX > _framebuffer->width)
			mouseX = _framebuffer->width;
		if (mouseY > _framebuffer->height)
			mouseY = _framebuffer->height;
		if (mouseY < 0)
			mouseY = 0;
		if (mouseX < 0)
			mouseX = 0;

		if (haveToRender == false) {
			switchToUserProcess();
		}

		haveToRender = false;

		composite();
		copyToScreen();

		switchToUserProcess();
	}
}

function kernelMain(const KernelParams *params) {
	kernelInit(params);
	__sync_synchronize();
	// sti -> start sheduling here
	// It will erase whole stack on next sheduling
	while (true) {
		amd64::enableAllInterruptsAndHalt();
	}
	// TODO hexa: error if code present in unreachable block
	// (no break/continue/throw)
}
}
