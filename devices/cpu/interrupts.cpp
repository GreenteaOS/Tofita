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

#define PIC1 0x20 // IO base address for master PIC
#define PIC2 0xA0 // IO base address for slave PIC
#define PIC1_COMMAND PIC1
#define PIC1_COMMAND_0x20 0x20
#define PIC1_DATA (PIC1 + 1)
#define PIC2_COMMAND PIC2
#define PIC2_DATA (PIC2 + 1)
#define PIC_EOI 0x20	  // End-of-interrupt command code
#define PIC_EOI_0x20 0x20 // End-of-interrupt command code

uint8_t readPort(uint16_t port);
function writePort(uint16_t port, uint8_t value);
uint8_t mouseRead();

#define PACKED __attribute__((packed))

void *tmemcpy(void *dest, const void *src, uint64_t count) {
	uint8_t *dst8 = (uint8_t *)dest;
	uint8_t *src8 = (uint8_t *)src;

	while (count--) {
		*dst8++ = *src8++;
	}

	return dest;
}

#pragma pack(1)
typedef struct {
	uint16_t limit;
	uint64_t offset;
} PACKED Idtr;
#pragma pack()

_Static_assert(sizeof(Idtr) == 10, "IDTR register has to be 80 bits long");

typedef struct {
	uint16_t offsetLowerbits : 16;
	uint16_t selector : 16;
	uint8_t ist : 8;
	uint8_t gateType : 4;
	uint8_t z : 1;
	uint8_t dpl : 2;
	uint8_t present : 1;
	uint64_t offsetHigherbits : 48;
	uint32_t zero : 32;
} PACKED IdtEntry;

uint8_t readPort(uint16_t port) {
	uint8_t data;
	asm volatile("inb %w1,%b0" : "=a"(data) : "d"(port));
	return data;
}

function writePort(uint16_t port, uint8_t value) {
	asm volatile("outb %b0,%w1" : : "a"(value), "d"(port));
}

static inline function loadIdt(Idtr *idtr) {
	asm volatile("lidtq %0" : : "m"(*idtr));
}

// http://wiki.osdev.org/Inline_Assembly/Examples#I.2FO_access
static inline function ioWait(void) {
	// TODO: reinvestigate
	asm volatile("jmp 1f\n\t"
				 "1:jmp 2f\n\t"
				 "2:");
}

// Handling IDT
#define IDT_SIZE 256
// Records of the GDT - 16 items
#define NULL_SEL 0x00
//#define EMPTY_SEL 0x08
#define SYS_CODE64_SEL 0x10 // Execute/Read
#define SYS_DATA32_SEL 0x18 // Read/Write
// Layout expected by SYSRET
#define USER_CODE32_SEL 0x20 // Execute/Read
#define USER_DATA32_SEL 0x28 // Read/Write
#define USER_CODE64_SEL 0x30 // Execute/Read
//#define EMPTY_SEL 0x38
//#define TSS_SEL 0x40 // Index = 8
//#define TSS_SEL 0x48
//#define USER_DATA32_SEL 0x50 // Read/Write
//#define EMPTY_SEL 0x58
#define SYS_CODE32_SEL 0x60 // Execute/Read
//#define EMPTY_SEL 0x68
//#define EMPTY_SEL 0x70
//#define EMPTY_SEL 0x78

IdtEntry IDT[IDT_SIZE];

// Handling keyboard
extern function keyboardHandler();
function initializeKeyboard(IdtEntry *entry) {
	uint64_t keyboardAddress = ((uint64_t)keyboardHandler);
	entry->offsetLowerbits = keyboardAddress & 0xffff;
	entry->offsetHigherbits = (keyboardAddress & 0xffffffffffff0000) >> 16;
	entry->selector = SYS_CODE64_SEL;
	entry->zero = 0;
	entry->ist = 2; // Stack
	entry->z = 0;
	entry->dpl = 0; // Ring 0
	entry->present = 1;
	entry->gateType = 0xe; // Interrupt gate
}

extern function mouseHandler();
function initializeMouse(IdtEntry *entry) {
	uint64_t mouseAddress = ((uint64_t)mouseHandler);
	entry->offsetLowerbits = mouseAddress & 0xffff;
	entry->offsetHigherbits = (mouseAddress & 0xffffffffffff0000) >> 16;
	entry->selector = SYS_CODE64_SEL;
	entry->zero = 0;
	entry->ist = 2; // Stack
	entry->z = 0;
	entry->dpl = 0; // Ring 0
	entry->present = 1;
	entry->gateType = 0xe; // Interrupt gate
}

function initializeFallback(IdtEntry *entry, uint64_t fallbackHandler) {
	uint64_t address = ((uint64_t)fallbackHandler);
	entry->offsetLowerbits = address & 0xffff;
	entry->offsetHigherbits = (address & 0xffffffffffff0000) >> 16;
	entry->selector = SYS_CODE64_SEL;
	entry->zero = 0;
	entry->ist = 1; // Stack
	entry->z = 0;
	entry->dpl = 3; // Ring 3
	entry->present = 1;
	entry->gateType = 0xe; // Interrupt gate
}

#define ICW1_ICW4 0x01		// ICW4 (not) needed
#define ICW1_SINGLE 0x02	// Single (cascade) mode
#define ICW1_INTERVAL4 0x04 // Call address interval 4 (8)
#define ICW1_LEVEL 0x08		// Level triggered (edge) mode
#define ICW1_INIT 0x10		// Initialization - required!

#define ICW4_8086 0x01		 // 8086/88 (MCS-80/85) mode
#define ICW4_AUTO 0x02		 // Auto (normal) EOI
#define ICW4_BUF_SLAVE 0x08	 // Buffered mode/slave
#define ICW4_BUF_MASTER 0x0C // Buffered mode/master
#define ICW4_SFNM 0x10		 // Special fully nested (not)

/*
arguments:
	offset1 - vector offset for master PIC
			vectors on the master become offset1..offset1+7
	offset2 - same for slave PIC: offset2..offset2+7

Taken from http://wiki.osdev.org/8259_PIC
*/
function remapPic(uint8_t offset1, uint8_t offset2) {
	serialPrintln(u8"[cpu] begin: remap PIC");

	writePort(0x20, 0x11);
	writePort(0xA0, 0x11);
	writePort(0x21, 0x20);
	writePort(0xA1, 0x28);
	writePort(0x21, 0x04);
	writePort(0xA1, 0x02);
	writePort(0x21, 0x01);
	writePort(0xA1, 0x01);
	writePort(0x21, 0x0);
	writePort(0xA1, 0x0);

	serialPrintln(u8"[cpu] done: remap PIC");

	return;

	uint8_t a1, a2;

	a1 = readPort(PIC1_DATA); // save masks
	a2 = readPort(PIC2_DATA);

	writePort(PIC1_COMMAND, ICW1_INIT + ICW1_ICW4); // starts the initialization sequence (in cascade mode)
	ioWait();
	writePort(PIC2_COMMAND, ICW1_INIT + ICW1_ICW4);
	ioWait();
	writePort(PIC1_DATA, offset1); // ICW2: Master PIC vector offset
	ioWait();
	writePort(PIC2_DATA, offset2); // ICW2: Slave PIC vector offset
	ioWait();
	writePort(PIC1_DATA, 4); // ICW3: tell Master PIC that there is a slave PIC at IRQ2 (0000 0100)
	ioWait();
	writePort(PIC2_DATA, 2); // ICW3: tell Slave PIC its cascade identity (0000 0010)
	ioWait();

	writePort(PIC1_DATA, ICW4_8086);
	ioWait();
	writePort(PIC2_DATA, ICW4_8086);
	ioWait();

	writePort(PIC1_DATA, a1); // restore saved masks.
	writePort(PIC2_DATA, a2);

	serialPrintln(u8"[cpu] done: remap PIC");
}

// Sets up the legacy PIC and then disables it
void disablePic(void) {
	/* Set ICW1 */
	writePort(0x20, 0x11);
	writePort(0xa0, 0x11);

	/* Set ICW2 (IRQ base offsets) */
	writePort(0x21, 0xe0);
	writePort(0xa1, 0xe8);

	/* Set ICW3 */
	writePort(0x21, 4);
	writePort(0xa1, 2);

	/* Set ICW4 */
	writePort(0x21, 1);
	writePort(0xa1, 1);

	/* Set OCW1 (interrupt masks) */
	writePort(0x21, 0xff);
	writePort(0xa1, 0xff);
}

uint64_t physicalToVirtual(uint64_t physical) {
	uint64_t result = (uint64_t)WholePhysicalStart + (uint64_t)physical;
	return result;
}

// TODO check for CPU feature
function enableLocalApic() {
	serialPrintln(u8"[cpu] begin: enableLocalApic");

	auto ptr = (uint32_t *)physicalToVirtual(0xfee000f0);
	uint32_t val = *ptr;
	quakePrintf(u8"APIC value is %u\n", val);

	serialPrintln(u8"[cpu] done: enableLocalApic");
}

function mouseWait(uint8_t aType);
function mouseWrite(uint8_t aWrite);
uint8_t mouseRead();
#define IRQ0 32
#define IRQ1 0x21 // 33
#define IRQ2 34
#define IRQ3 35
#define IRQ4 36
#define IRQ5 37
#define IRQ6 38
#define IRQ7 39
#define IRQ8 40
#define IRQ9 41
#define IRQ10 42
#define IRQ11 43
#define IRQ12 44 // 2c
#define IRQ13 45
#define IRQ14 46
#define IRQ15 47

#pragma pack(1)
struct TablePtr {
	uint16_t limit;
	uint64_t base;
} __attribute__((packed));
#pragma pack()

_Static_assert(sizeof(TablePtr) == 10, "sizeof is incorrect");

static inline function lgdt(volatile const TablePtr *gdt) {
	asm volatile("lgdt (%0)" : : "r"(gdt) : "memory");
}

enum GdtType : uint8_t {
	accessed = 0x01,
	read_write = 0x02,
	conforming = 0x04,
	execute = 0x08,
	system = 0x10,
	ring1 = 0x20,
	ring2 = 0x40,
	ring3 = 0x60,
	present = 0x80
};

struct GdtDescriptor {
	uint16_t limitLow;
	uint16_t baseLow;
	uint8_t baseMid;
	enum GdtType type; // Access
	uint8_t size;
	uint8_t baseHigh;
} __attribute__((packed));

_Static_assert(sizeof(GdtDescriptor) == 8, "sizeof is incorrect");

#pragma pack(1)
struct GdtDescriptorEx {
	uint16_t limitLow : 16;
	uint16_t baseLow : 16;
	uint8_t baseMid : 8;
	// Access
	uint8_t accessed : 1;
	uint8_t rw : 1;		   // 0 - read, 1 - read/write
	uint8_t direction : 1; // or conform
	// If 1 code in this segment can be executed from an equal or lower privilege level
	// If 0 code in this segment can only be executed from the ring set in "privilege"
	uint8_t execute : 1; // 1 = code, 0 = data
	uint8_t one : 1;
	uint8_t privilege : 2; // 0 = ring0/kernel, 3 = ring3
	uint8_t present : 1;
	// Flags
	uint8_t limit : 4;
	uint8_t reserved : 2;
	uint8_t zero16one32 : 1;
	uint8_t blocks : 1;

	uint8_t baseHigh : 8;
} __attribute__((packed));
#pragma pack()

_Static_assert(sizeof(GdtDescriptorEx) == 8, "sizeof is incorrect");

struct TssDescriptor {
	uint16_t limitLow;
	uint16_t base_00;
	uint8_t base_16;
	enum GdtType type;
	uint8_t size;
	uint8_t base_24;
	uint32_t base_32;
	uint32_t reserved;
} __attribute__((packed));

struct TssEntry {
	uint32_t reserved0;

	uint64_t rsp[3]; // stack pointers for CPL 0-2
	uint64_t ist[8]; // ist[0] is reserved

	uint64_t reserved1;
	uint16_t reserved2;
	uint16_t iomap_offset;
} __attribute__((packed));

__attribute__((aligned(64))) static volatile TablePtr globalGdtr;
__attribute__((aligned(64))) static volatile TssEntry globalTss;

// rsp0stack = exp0 in TSS
// Stack used by kernel to handle syscalls/interrupts
__attribute__((aligned(64))) uint8_t rsp0stack[4096 * 32] = {0};
__attribute__((aligned(64))) uint8_t rsp1stack[4096 * 32] = {0};
__attribute__((aligned(64))) uint8_t rsp2stack[4096 * 32] = {0};

__attribute__((aligned(64))) uint8_t ist0stack[4096 * 4] = {0};
__attribute__((aligned(64))) uint8_t ist1stack[4096 * 4] = {0};
__attribute__((aligned(64))) uint8_t ist2stack[4096 * 4] = {0};
__attribute__((aligned(64))) uint8_t ist3stack[4096 * 4] = {0};
__attribute__((aligned(64))) uint8_t ist4stack[4096 * 4] = {0};
__attribute__((aligned(64))) uint8_t ist5stack[4096 * 4] = {0};
__attribute__((aligned(64))) uint8_t ist6stack[4096 * 4] = {0};
__attribute__((aligned(64))) uint8_t ist7stack[4096 * 4] = {0};

// LLVM did the magic expected of it. It only saved registers that are clobbered by your function (hence rax).
// All other register are left unchanged hence there’s no need of saving and restoring them.
// https://github.com/phil-opp/blog_os/issues/450#issuecomment-582535783

// TODO: errcode is `extra` here:
//__attribute__((aligned(64))) __attribute__((interrupt)) void unknownInterrupt(struct InterruptFrame *frame,
// uint64_t
// extra) {
// https://github.com/llvm-mirror/clang/blob/master/test/SemaCXX/attr-x86-interrupt.cpp#L30
void anyInterruptHandler(InterruptFrame *frame);

// TODO remove
__attribute__((aligned(64))) __attribute__((interrupt)) void unknownInterrupt(InterruptFrame *frame) {
	// TODO Find a better way to avoid LLVM bugs
	anyInterruptHandler(frame);
}

void anyInterruptHandler(InterruptFrame *frame) {
	amd64::disableAllInterrupts();
#if 0
	let stack = (InterruptStack *)((uint64_t)frame - 200);

#endif

	// Enable interrupts
	writePort(0xA0, 0x20);
	writePort(PIC1_COMMAND_0x20, PIC_EOI_0x20);

	// Several operating systems such as Windows and Linux, use some of the segments for internal usage.
	// For instance Windows x64 uses the GS register to access the TLS (thread local storage)
	// and in Linux it's for accessing cpu specific memory
}

// ~121 times per second
uint64_t timerCalled = 0;

// Scheduling
const uint8_t THREAD_INIT = 0; // kernelMain, it will be destroyed
const uint8_t THREAD_GUI = 1;
const uint8_t THREAD_KERNEL = 2;
const uint8_t THREAD_USER = 3;

volatile uint8_t currentThread = THREAD_INIT;

constexpr uint64_t stackSizeForKernelThread = 1024 * 1024;

InterruptFrame kernelThreadFrame;
function kernelThread();
__attribute__((aligned(64))) uint8_t kernelStack[stackSizeForKernelThread] = {0};

InterruptFrame guiThreadFrame;
function guiThread();
__attribute__((aligned(64))) uint8_t guiStack[stackSizeForKernelThread] = {0};

extern "C" void timerInterruptHandler(InterruptFrame *frame);


uint8_t extraMillisecond = 0;
uint8_t taskBarRedraw = 0; // Re-paint task bar current time
volatile bool nextIsUserProcess = false;
volatile bool nextIsGuiThread = true;

uint64_t getNextProcess() {
	uint64_t index = 1;
	while (index < 255) {
		if (process::processes[index].present == true) {
			if (process::processes[index].schedulable == true)
				if (process::processes[index].scheduleOnNextTick == true)
					return index;
		}
		index++;
	}

	return 0; // Idle
}

function markAllProcessessSchedulable() {
	uint64_t index = 0;
	while (index < 255) { // TODO 256?
		if (process::processes[index].present == true) {
			if (process::processes[index].schedulable == true)
				process::processes[index].scheduleOnNextTick = true;
		}
		index++;
	}
}

function switchToKernelThread(InterruptFrame *frame) {
	if (currentThread == THREAD_KERNEL)
		return;

	if (currentThread == THREAD_USER) {
		// Save
		process::Process *process = &process::processes[process::currentProcess];
		tmemcpy(&process->frame, frame, sizeof(InterruptFrame));
	} else if (currentThread == THREAD_GUI) {
		// Save
		tmemcpy(&guiThreadFrame, frame, sizeof(InterruptFrame));
	}

	// Restore
	currentThread = THREAD_KERNEL;
	amd64::writeCr3((uint64_t)pml4kernelThread - (uint64_t)WholePhysicalStart);
	tmemcpy(frame, &kernelThreadFrame, sizeof(InterruptFrame));
}

function switchToNextProcess(InterruptFrame *frame) {
	volatile var next = getNextProcess();

	if (next == 0) {
		markAllProcessessSchedulable();
		next = getNextProcess();
	}

	process::Process *process = &process::processes[next];
	volatile let old = process::currentProcess;
	process::currentProcess = process->pid;
	// TODO in .exe loader, disallow changing of CR3 until not fully loaded
	// another option is to save/restore CR3 used by kernel thread
	amd64::writeCr3((uint64_t)process->pml4 - (uint64_t)WholePhysicalStart);
	process->scheduleOnNextTick = false;

	// Note: pid 0 is not real process, so keep things in kernel
	if (next != 0) {
		if (currentThread == THREAD_GUI) {
			// Save
			tmemcpy(&guiThreadFrame, frame, sizeof(InterruptFrame));
		} else if (currentThread == THREAD_USER) {
			// Save
			process::Process *processOld = &process::processes[old];
			tmemcpy(&processOld->frame, frame, sizeof(InterruptFrame));
		} else if (currentThread == THREAD_KERNEL) {
			// Save
			tmemcpy(&kernelThreadFrame, frame, sizeof(InterruptFrame));
		}

		// Restore
		currentThread = THREAD_USER;
		tmemcpy(frame, &process->frame, sizeof(InterruptFrame));
	} else {
		switchToKernelThread(frame);
	}
}

function switchToGuiThread(InterruptFrame *frame) {
	if (currentThread == THREAD_GUI)
		return;

	if (currentThread == THREAD_USER) {
		// Save
		process::Process *process = &process::processes[process::currentProcess];
		tmemcpy(&process->frame, frame, sizeof(InterruptFrame));
	} else if (currentThread == THREAD_KERNEL) {
		// Save
		tmemcpy(&kernelThreadFrame, frame, sizeof(InterruptFrame));
	}

	// Restore
	currentThread = THREAD_GUI;
	tmemcpy(frame, &guiThreadFrame, sizeof(InterruptFrame));
}

void yieldInterruptHandler(InterruptFrame *frame) {
	amd64::disableAllInterrupts();
	switchToNextProcess(frame);
}


void timerInterruptHandler(InterruptFrame *frame) {
	amd64::disableAllInterrupts();

	if (timerCalled % 121 == 0) {
		serialPrintf(u8"[cpu] happened timerInterrupt (one second passed) < ! #%d\n", timerCalled);
		taskBarRedraw++;
		if (taskBarRedraw > 30) {
			haveToRender = 1;
			taskBarRedraw = 0;
		}
	}
	uptimeMilliseconds += 8;
	if (extraMillisecond % 4 == 0) {
		uptimeMilliseconds += 1; // PIT is somewhat imprecise
	}
	if (extraMillisecond % 64 == 0) {
		uptimeMilliseconds += 1; // PIT is somewhat imprecise
	}
	extraMillisecond++;
	timerCalled++;

	// Schedule
	if (true) {
		if (currentThread == THREAD_INIT) {
			// TODO hexa error == statement: currentThread == THREAD_INIT;
			guiThreadFrame.flags = frame->flags;	// TODO
			kernelThreadFrame.flags = frame->flags; // TODO

			// Restore
			currentThread = THREAD_GUI;
			tmemcpy(frame, &guiThreadFrame, sizeof(InterruptFrame));
		} else {
			if (currentThread == THREAD_GUI) {
				// Just give it enough time to fimish rendering
				// It will schedule to other process on finish
				nextIsGuiThread = false;
			} else {
				if (nextIsGuiThread == true) {
					nextIsGuiThread = false;
					switchToGuiThread(frame);
				} else {
					nextIsGuiThread = true;
					if (nextIsUserProcess == true) {
						switchToKernelThread(frame);
					} else {
						switchToNextProcess(frame);
					}
					nextIsUserProcess = !nextIsUserProcess;
				}
			}
		}
	}

	// Enable interrupts
	writePort(PIC1_COMMAND_0x20, PIC_EOI_0x20);
}

function syscallInterruptHandler(InterruptFrame *frame) {
	amd64::disableAllInterrupts();
	volatile process::Process *process = &process::processes[process::currentProcess];
	process->schedulable = false;
	volatile let index = (TofitaSyscalls)frame->rcxArg0;

	// No-op
	if ((uint64_t)index < USER_SYSCALLS) {
		process->schedulable = true;
		return;
	}

	process->syscallToHandle = index;

	switchToKernelThread(frame);
	// TODO weird frame+8 offset if called switchToNextProcess
	// switchToNextProcess(frame);
	return;

	if (index == TofitaSyscalls::DebugLog) {
		serialPrintf(u8"[[DebugLog:PID %d]] %s\n", process::currentProcess, frame->rdxArg1);
		return;
	}

	if (index == TofitaSyscalls::ExitProcess) {
		serialPrintf(u8"[[ExitProcess:PID %d]] %d\n", process::currentProcess, frame->rdxArg1);
		// TODO kernel wakeup
		// TODO destroy process
		process::Process *process = &process::processes[process::currentProcess];
		process->schedulable = false;
		switchToNextProcess(frame);
		process->present = false;
		return;
	}

}

function setTsr(uint16_t tsr_data) {
	asm volatile("ltr %[src]"
				 :						 // No outputs
				 : [ src ] "m"(tsr_data) // Inputs
				 :						 // No clobbers
	);
}

// 16 records
__attribute__((aligned(64))) static uint32_t gdtTemplate[32] = {
	0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00209b00, 0x0000ffff, 0x00cf9300,
	0x0000ffff, 0x00cffa00, 0x0000ffff, 0x00cff300, 0x00000000, 0x0020fb00, 0x00000000, 0x00000000,
	0x60800067, 0x00008bb9, 0xfffff800, 0x00000000, 0xe0003c00, 0xff40f3fa, 0x00000000, 0x00000000,
	0x0000ffff, 0x00cf9a00, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000};

#define PS2_DATA_PORT 0x60
#define PS2_CONTROL_PORT 0x64

function gdtSetEntry(uint8_t i, uint32_t base, uint64_t limit, bool is64, enum GdtType typed) {
	GdtDescriptor *gdtd = (GdtDescriptor *)&gdtTemplate[i * 2];
	gdtd->limitLow = limit & 0xffff;
	gdtd->size = (limit >> 16) & 0xf;
	// gdtd->size |= (is64 ? 0xa0 : 0xc0);
	gdtd->size |= 0xa0;
	gdtd->size <<= 0x0D; // Long mode

	gdtd->baseLow = base & 0xffff;
	gdtd->baseMid = (base >> 16) & 0xff;
	gdtd->baseHigh = (base >> 24) & 0xff;

	gdtd->type = (enum GdtType)(typed | system | present);
}

// Takes 2 GDT entries
function tssSetEntryNT(uint8_t i, uint64_t base, uint64_t limit) {
	TssDescriptor *tssd = (TssDescriptor *)&gdtTemplate[i * 2];
	tssd->limitLow = limit & 0xffff;
	tssd->size = (limit >> 16) & 0xf;

	tssd->base_00 = base & 0xffff;
	tssd->base_16 = (base >> 16) & 0xff;
	tssd->base_24 = (base >> 24) & 0xff;
	tssd->base_32 = (base >> 32) & 0xffffffff;
	tssd->reserved = 0;

	tssd->type = (enum GdtType)(accessed | execute | ring3 | // TODO
								present);
}

__attribute__((aligned(64))) Idtr cacheIdtr;

function dumpGDT(GdtDescriptorEx *desc) {
	serialPrintf(u8"[dumpGDT]");

	if (desc->accessed)
		serialPrintf(u8" accessed");
	if (desc->rw)
		serialPrintf(u8" rw");
	if (desc->direction)
		serialPrintf(u8" direction");
	if (desc->execute)
		serialPrintf(u8" execute");
	if (desc->one)
		serialPrintf(u8" one");

	if (desc->privilege == 0)
		serialPrintf(u8" privilege=ring0");
	if (desc->privilege == 1)
		serialPrintf(u8" privilege=ring1");
	if (desc->privilege == 2)
		serialPrintf(u8" privilege=ring2");
	if (desc->privilege == 3)
		serialPrintf(u8" privilege=ring3");

	if (desc->present)
		serialPrintf(u8" present");
	if (desc->zero16one32)
		serialPrintf(u8" zero16one32");
	if (desc->blocks)
		serialPrintf(u8" pages");

	serialPrintf(u8"\n");
}

function enableInterrupts() {
	serialPrintln(u8"[cpu] initializing lgdt");

	uint64_t sizeof_TssEntry = sizeof(globalTss);
	serialPrintf(u8"[cpu] sizeof_TssEntry 104 = %u == %u\n", sizeof_TssEntry, sizeof(TssEntry));

	memset((void *)&globalTss, 0, sizeof_TssEntry);
	globalTss.iomap_offset = sizeof_TssEntry;
	globalTss.rsp[0] = (uint64_t)&rsp0stack;
	globalTss.rsp[1] = (uint64_t)&rsp1stack;
	globalTss.rsp[2] = (uint64_t)&rsp2stack;
	// TODO zero out stacks
	// TODO more stacks
	globalTss.ist[0] = (uint64_t)&ist0stack;
	globalTss.ist[1] = (uint64_t)&ist1stack;
	globalTss.ist[2] = (uint64_t)&ist2stack;
	globalTss.ist[3] = (uint64_t)&ist3stack;
	globalTss.ist[4] = (uint64_t)&ist4stack;
	globalTss.ist[5] = (uint64_t)&ist5stack;
	globalTss.ist[6] = (uint64_t)&ist6stack;
	globalTss.ist[7] = (uint64_t)&ist7stack;

	serialPrint(u8"[cpu] RSP[0] points to: ");
	serialPrintHex((uint64_t)globalTss.rsp[0]);
	serialPrint(u8"\n");

	serialPrint(u8"[cpu] RSP[1] points to: ");
	serialPrintHex((uint64_t)globalTss.rsp[1]);
	serialPrint(u8"\n");

	serialPrint(u8"[cpu] RSP[2] points to: ");
	serialPrintHex((uint64_t)globalTss.rsp[2]);
	serialPrint(u8"\n");

	uint64_t tssBase = (uint64_t)(&globalTss);

	// upload
	{
		tssSetEntryNT(8, tssBase, sizeof_TssEntry);
		// tssSetEntryNT(8, tssBase, sizeof_TssEntry - 1); TODO
	}

	if (false) {
		let gdt = (GdtDescriptorEx *)gdtTemplate;
		for (uint32_t i = 0; i < 16; ++i) {
			if (i == 0)
				continue; // Empty
			if (i == 8)
				continue; // TSS
			if (i == 9)
				continue; // TSS
			if (gdt[i].present == 0)
				continue;
			serialPrintf(u8"[dumpGDT] #%u\n", i);
			dumpGDT(&gdt[i]);
		}
	}

	globalGdtr.limit = sizeof(gdtTemplate) - 1;
	globalGdtr.base = (uint64_t)(&gdtTemplate[0]);
	serialPrint(u8"[cpu] GDTR points to: ");
	serialPrintHex((uint64_t)&globalGdtr);
	serialPrint(u8"\n");
	serialPrint(u8"[cpu] GDT points to: ");
	serialPrintHex((uint64_t)globalGdtr.base);
	serialPrint(u8"\n");
	serialPrintf(u8"[cpu] GDT size is %u == %u\n", globalGdtr.limit, 0x7F);
	serialPrintln(u8"[cpu] calling lgdt");
	lgdt(&globalGdtr);
	serialPrintln(u8"[cpu] calling ltr");
	{
		setTsr(64);
		// setTsr(64 + 3);
	}

	serialPrintln(u8"[cpu] initializing unknownInterrupt");

	for (uint32_t i = 0; i < 286; ++i) {
		initializeFallback(&IDT[i], (uint64_t)&unknownInterrupt);
	}

	initializeFallback(&IDT[IRQ0], (uint64_t)(&timerInterrupt));
	initializeFallback(&IDT[0x80], (uint64_t)(&syscallInterrupt));
	initializeFallback(&IDT[0x81], (uint64_t)(&yieldInterrupt));
	initializeKeyboard(&IDT[IRQ1]);
	initializeMouse(&IDT[IRQ12]);

	cacheIdtr.limit = (sizeof(IdtEntry) * IDT_SIZE) - 1;
	cacheIdtr.offset = (uint64_t)IDT;

	serialPrint(u8"[cpu] IDTR points to: ");
	serialPrintHex((uint64_t)&cacheIdtr);
	serialPrint(u8"\n");
	serialPrint(u8"[cpu] IDT points to: ");
	serialPrintHex((uint64_t)&IDT);
	serialPrint(u8"\n");
	serialPrintf(u8"[cpu] IDT size is %u of %u elements of %u==16 size\n", cacheIdtr.limit, IDT_SIZE,
				 sizeof(IdtEntry));
	serialPrintln(u8"[cpu] loading IDTR");
	// Before you implement the IDT, make sure you have a working GDT
	remapPic(0x20, 0x28);
	serialPrintln(u8"[cpu] calling lidtq");
	loadIdt(&cacheIdtr);

	serialPrintln(u8"[cpu] Select segments of value SYS_CODE64_SEL & SYS_DATA32_SEL");
	enterKernelMode();

	// Set PIT frequency
	// Valid values: 57 Hz, 121 Hz, 173 Hz
	// Next valid: `i = 174; while (Math.round(1193181 / i) != (1193181 / i)) {i++;}; console.log(i)`
	writePort(0x43, 0x36);
	let frequency = 1193181 / 121;
	writePort(0x40, frequency & 0xFF);
	writePort(0x40, (frequency >> 8) & 0xFF);

	// Unmasking IRQ to support all the things
	writePort(IRQ1, 0x00);
}

function enablePS2Mouse() {
	serialPrintln(u8"[cpu] begin: setting PS/2 mouse");
	uint8_t _status;
	// Enable the auxiliary mouse device
	mouseWait(1);
	writePort(PS2_CONTROL_PORT, 0xA8);

	/*

	IOWriteB(PS2_CONTROL_PORT, 0x20);
	ControllerConfig = IOReadB(PS2_DATA_PORT);
	ControllerConfig |=  0x02; // Turn on IRQ12
	ControllerConfig &= ~0x20; // Enable mouse clock line
	IOWriteB(PS2_CONTROL_PORT, 0x60);

	*/

	// Enable the interrupts
	mouseWait(1);
	writePort(PS2_CONTROL_PORT, 0x20);
	mouseWait(0);
	_status = (readPort(PS2_DATA_PORT) | 2); // Turn on IRQ12
	//_status &= ~0x20; // Enable mouse clock line
	mouseWait(1);
	writePort(PS2_CONTROL_PORT, 0x60);
	mouseWait(1);
	writePort(PS2_DATA_PORT, _status);
	// Tell the mouse to use default settings
	mouseWrite(0xF6);
	mouseRead(); // Acknowledge
	// Enable the mouse
	mouseWrite(0xF4);
	mouseRead(); // Acknowledge
	// Setup the mouse handler
	// irq_install_handler(12, mouseHandler);
	serialPrintln(u8"[cpu] done: setting PS/2 mouse");
	function quakePrintf(const char8_t *c, ...);
	quakePrintf(u8"Enabled PS/2 mouse and keyboard\n");
}

function mouseWait(uint8_t aType) {
	uint32_t _timeOut = 100000;
	if (aType == 0) {
		while (_timeOut--) // Data
		{
			if ((readPort(PS2_CONTROL_PORT) & 1) == 1) {
				return;
			}
		}
		return;
	} else {
		while (_timeOut--) // Signal
		{
			if ((readPort(PS2_CONTROL_PORT) & 2) == 0) {
				return;
			}
		}
		return;
	}
}

function mouseWrite(uint8_t aWrite) {
	// Wait to be able to send a command
	mouseWait(1);
	// Tell the mouse we are sending a command
	writePort(PS2_CONTROL_PORT, 0xD4);
	// Wait for the final part
	mouseWait(1);
	// Finally write
	writePort(PS2_DATA_PORT, aWrite);
}

uint8_t mouseRead() {
	// Get's response from mouse
	mouseWait(0);
	return readPort(PS2_DATA_PORT);
}
