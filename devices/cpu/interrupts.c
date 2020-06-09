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

#define PIC1		0x20		// IO base address for master PIC
#define PIC2		0xA0		// IO base address for slave PIC
#define PIC1_COMMAND	PIC1
#define PIC1_COMMAND_0x20	0x20
#define PIC1_DATA	(PIC1+1)
#define PIC2_COMMAND	PIC2
#define PIC2_DATA	(PIC2+1)
#define PIC_EOI		0x20		// End-of-interrupt command code
#define PIC_EOI_0x20		0x20		// End-of-interrupt command code

uint8_t readPort(uint16_t port);
function writePort(uint16_t port, uint8_t value);
uint8_t mouseRead();

#define PACKED __attribute__((packed))

void* tmemcpy(void* dest, const void* src, uint64_t count) {
	uint8_t* dst8 = (uint8_t*)dest;
	uint8_t* src8 = (uint8_t*)src;

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
	__asm__ volatile("inb %w1,%b0" : "=a" (data) : "d"(port));
	return data;
}

function writePort(uint16_t port, uint8_t value) {
	__asm__ volatile("outb %b0,%w1" : : "a" (value), "d"(port));
}

static inline function loadIdt(Idtr *idtr) {
	__asm__ volatile("lidtq %0" : : "m" (*idtr));
	__asm__ volatile("sti");
}

extern "C" void selectSegment(void* func);

void stub() {
	serialPrintln(u8"[cpu] welp selectSegment succeded!");
}

// http://wiki.osdev.org/Inline_Assembly/Examples#I.2FO_access
static inline function ioWait(void) {
	// TODO: reinvestigate
	__asm__ volatile ( "jmp 1f\n\t"
					 "1:jmp 2f\n\t"
					 "2:" );
}

// Handling IDT
//#define IDT_SIZE 256
#define IDT_SIZE 256
#define SYS_CODE64_SEL 0x38 // https://github.com/tianocore/edk/blob/master/Sample/Universal/DxeIpl/Pei/x64/LongMode.asm#L281
IdtEntry IDT[IDT_SIZE];

// Handling keyboard
extern function keyboardHandler();
function initializeKeyboard(IdtEntry *entry) {
	uint64_t keyboardAddress = ((uint64_t) keyboardHandler);
	entry->offsetLowerbits = keyboardAddress & 0xffff;
	entry->offsetHigherbits = (keyboardAddress & 0xffffffffffff0000) >> 16;
	entry->selector = SYS_CODE64_SEL;
	entry->zero = 0;
	entry->ist = 0;
	entry->z = 0;
	entry->dpl = 0;
	entry->present = 1;
	entry->gateType = 0xe; // Interrupt gate
}

extern function mouseHandler();
function initializeMouse(IdtEntry *entry) {
	uint64_t mouseAddress = ((uint64_t) mouseHandler);
	entry->offsetLowerbits = mouseAddress & 0xffff;
	entry->offsetHigherbits = (mouseAddress & 0xffffffffffff0000) >> 16;
	entry->selector = SYS_CODE64_SEL;
	entry->zero = 0;
	entry->ist = 0;
	entry->z = 0;
	entry->dpl = 0;
	entry->present = 1;
	entry->gateType = 0xe; // Interrupt gate
}

function initializeFallback(IdtEntry *entry, uint64_t fallback_handler_) {
	uint64_t address = ((uint64_t) fallback_handler_);
	entry->offsetLowerbits = address & 0xffff;
	entry->offsetHigherbits = (address & 0xffffffffffff0000) >> 16;
	entry->selector = 16; //2; //0x02; //SYS_CODE64_SEL;
	entry->zero = 0;
	entry->ist = 0;
	entry->z = 0;
	entry->dpl = 0;
	entry->present = 1;
	entry->gateType = 0xe; // Interrupt gate
}

#define ICW1_ICW4   0x01        // ICW4 (not) needed
#define ICW1_SINGLE 0x02        // Single (cascade) mode
#define ICW1_INTERVAL4  0x04        // Call address interval 4 (8)
#define ICW1_LEVEL  0x08        // Level triggered (edge) mode
#define ICW1_INIT   0x10        // Initialization - required!

#define ICW4_8086   0x01        // 8086/88 (MCS-80/85) mode
#define ICW4_AUTO   0x02        // Auto (normal) EOI
#define ICW4_BUF_SLAVE  0x08        // Buffered mode/slave
#define ICW4_BUF_MASTER 0x0C        // Buffered mode/master
#define ICW4_SFNM   0x10        // Special fully nested (not)

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

	writePort(PIC1_COMMAND, ICW1_INIT+ICW1_ICW4);  // starts the initialization sequence (in cascade mode)
	ioWait();
	writePort(PIC2_COMMAND, ICW1_INIT+ICW1_ICW4);
	ioWait();
	writePort(PIC1_DATA, offset1);     // ICW2: Master PIC vector offset
	ioWait();
	writePort(PIC2_DATA, offset2);     // ICW2: Slave PIC vector offset
	ioWait();
	writePort(PIC1_DATA, 4);           // ICW3: tell Master PIC that there is a slave PIC at IRQ2 (0000 0100)
	ioWait();
	writePort(PIC2_DATA, 2);           // ICW3: tell Slave PIC its cascade identity (0000 0010)
	ioWait();

	writePort(PIC1_DATA, ICW4_8086);
	ioWait();
	writePort(PIC2_DATA, ICW4_8086);
	ioWait();

	writePort(PIC1_DATA, a1);   // restore saved masks.
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

	auto ptr = (uint32_t*)physicalToVirtual(0xfee000f0);
	uint32_t val = *ptr;
	quakePrintf(u8"APIC value is %u\n", val);

	serialPrintln(u8"[cpu] done: enableLocalApic");
}

function mouseWait(uint8_t aType);
function mouseWrite(uint8_t aWrite);
uint8_t mouseRead();
#define  IRQ0 32
#define  IRQ1 0x21 // 33
#define  IRQ2 34
#define  IRQ3 35
#define  IRQ4 36
#define  IRQ5 37
#define  IRQ6 38
#define  IRQ7 39
#define  IRQ8 40
#define  IRQ9 41
#define IRQ10 42
#define IRQ11 43
#define IRQ12 44 // 2c
#define IRQ13 45
#define IRQ14 46
#define IRQ15 47

#pragma pack(1)
struct TablePtr
{
	uint16_t limit;
	uint64_t base;
} __attribute__ ((packed));
#pragma pack()

_Static_assert(sizeof(TablePtr) == 10, "sizeof is incorrect");

static inline function lgdt(volatile const struct TablePtr *gdt) {
	asm volatile ("lgdt (%0)" : : "r" (gdt) : "memory");
}

enum GdtType : uint8_t
{
	accessed	= 0x01,
	read_write	= 0x02,
	conforming	= 0x04,
	execute		= 0x08,
	system		= 0x10,
	ring1		= 0x20,
	ring2		= 0x40,
	ring3		= 0x60,
	present		= 0x80
};
//IS_BITFIELD(GdtType);

struct GdtDescriptor
{
	uint16_t limitLow;
	uint16_t baseLow;
	uint8_t baseMid;
	enum GdtType type; // Access
	uint8_t size;
	uint8_t baseHigh;
} __attribute__ ((packed));

_Static_assert(sizeof(GdtDescriptor) == 8, "sizeof is incorrect");

struct TssDescriptor
{
	uint16_t limitLow;
	uint16_t base_00;
	uint8_t base_16;
	enum GdtType type;
	uint8_t size;
	uint8_t base_24;
	uint32_t base_32;
	uint32_t reserved;
} __attribute__ ((packed));

struct TssEntry
{
	uint32_t reserved0;

	uint64_t rsp[3]; // stack pointers for CPL 0-2
	uint64_t ist[8]; // ist[0] is reserved

	uint64_t reserved1;
	uint16_t reserved2;
	uint16_t iomap_offset;
} __attribute__ ((packed));

__attribute__((aligned(64))) static volatile struct TablePtr g_gdtr;
__attribute__((aligned(64))) static volatile struct TssEntry g_tss;
__attribute__((aligned(64))) static volatile struct GdtDescriptor g_gdt_table[10];

// rsp0stack = exp0 in TSS
// Stack used by kernel to handle syscalls/interrupts
__attribute__((aligned(64))) uint8_t rsp0stack[4096 * 32] = {0};
__attribute__((aligned(64))) uint8_t rsp1stack[4096 * 32] = {0};
__attribute__((aligned(64))) uint8_t rsp2stack[4096 * 32] = {0};

typedef uint64_t uword_t;
struct interrupt_frame
{
	uword_t ip; // Instruction Pointer
	uword_t cs; // Code Segment
	uword_t flags;
	uword_t sp; // Stack Pointer
	uword_t ss; // Stack Segment
};

// LLVM did the magic expected of it. It only saved registers that are clobbered by your function (hence rax). All other register are left unchanged hence there’s no need of saving and restoring them.
// https://github.com/phil-opp/blog_os/issues/450#issuecomment-582535783

// TODO: errcode is `extra` here:
//__attribute__((aligned(64))) __attribute__((interrupt)) void foo_interrupt(struct interrupt_frame *frame, uint64_t extra) {
// https://github.com/llvm-mirror/clang/blob/master/test/SemaCXX/attr-x86-interrupt.cpp#L30
__attribute__((aligned(64))) __attribute__((interrupt)) void foo_interrupt(struct interrupt_frame *frame) {
	serialPrintf(u8"[cpu] happened foo_interrupt %u ip~=%u cs=%u flags=%u sp=%u ss=%u\n",
		frame,
		frame->ip,
		frame->cs,
		frame->flags,
		frame->sp,
		frame->ss
	);
	serialPrint(u8"[cpu] frame->ip points to: ");
	serialPrintHex((uint64_t) frame->ip);
	serialPrint(u8"\n");

	serialPrint(u8"[cpu] frame->sp points to: ");
	serialPrintHex((uint64_t) frame->sp);
	serialPrint(u8"\n");

	frame->cs = 16;
	frame->ss = 0x18;

	// Enable interrupts
	writePort(0xA0, 0x20);
	writePort(PIC1_COMMAND_0x20, PIC_EOI_0x20);

	// Several operating systems such as Windows and Linux, use some of the segments for internal usage. for instance Windows x64 uses the GS register to access the TLS (thread local storage) and in Linux it's for accessing cpu specific memory
}

// ~100 times per second
int32_t timer_called = 0;
__attribute__((aligned(64))) __attribute__((interrupt)) void timer_interrupt(struct interrupt_frame *frame) {
	serialPrintf(u8"[cpu] happened timer_interrupt <<<<<<<<<<<<<<<<<<<<<<<<<<<<< !!!!!!!!!!!!!! #%d\n", timer_called++);

	frame->cs = 16;
	frame->ss = 0x18;

	// Enable interrupts
	writePort(PIC1_COMMAND_0x20, PIC_EOI_0x20);
}

void set_tsr(uint16_t tsr_data)
{
  asm volatile("ltr %[src]"
		   : // No outputs
		   : [src] "m" (tsr_data) // Inputs
		   : // No clobbers
		 );
}

// 16 records
__attribute__((aligned(64))) static uint32_t gdtTemplate[32] = {
	0x00000000, 0x00000000,
	0x00000000, 0x00000000,
	0x00000000, 0x00209b00,
	0x0000ffff, 0x00cf9300,
	0x0000ffff, 0x00cffa00,
	0x0000ffff, 0x00cff300,
	0x00000000, 0x0020fb00,
	0x00000000, 0x00000000,
	0x60800067, 0x00008bb9,
	0xfffff800, 0x00000000,
	0xe0003c00, 0xff40f3fa,
	0x00000000, 0x00000000,
	0x0000ffff, 0x00cf9a00,
	0x00000000, 0x00000000,
	0x00000000, 0x00000000,
	0x00000000, 0x00000000
};

#define PS2_DATA_PORT 0x60
#define PS2_CONTROL_PORT 0x64

#if 1
function gdtSetEntry(uint8_t i, uint32_t base, uint64_t limit, bool is64, enum GdtType typed)
{
	g_gdt_table[i].limitLow = limit & 0xffff;
	g_gdt_table[i].size = (limit >> 16) & 0xf;
	//g_gdt_table[i].size |= (is64 ? 0xa0 : 0xc0);
	g_gdt_table[i].size |= 0xa0;
	g_gdt_table[i].size <<= 0x0D; // Long mode

	g_gdt_table[i].baseLow = base & 0xffff;
	g_gdt_table[i].baseMid = (base >> 16) & 0xff;
	g_gdt_table[i].baseHigh = (base >> 24) & 0xff;

	g_gdt_table[i].type = (enum GdtType)(typed | system | present);
}

//global gdt_write
//gdt_write:
//	lgdt [rel g_gdtr]
//	mov ax, si ; second arg is data segment
//	mov ds, ax
//	mov es, ax
//	mov fs, ax
//	mov gs, ax
//	mov ss, ax
//	push qword rdi ; first arg is code segment
//	lea rax, [rel .next]
//	push rax
//	o64 retf
//.next:
//	ltr dx
//	ret

void
tssSetEntry(uint8_t i, uint64_t base, uint64_t limit)
{
	struct TssDescriptor tssd;
	tssd.limitLow = limit & 0xffff;
	tssd.size = (limit >> 16) & 0xf;

	tssd.base_00 = base & 0xffff;
	tssd.base_16 = (base >> 16) & 0xff;
	tssd.base_24 = (base >> 24) & 0xff;
	tssd.base_32 = (base >> 32) & 0xffffffff;
	tssd.reserved = 0;

	tssd.type = (enum GdtType)(
		accessed |
		execute |
		ring3 |
		present
	);
	tmemcpy((void*)&g_gdt_table[i], &tssd, sizeof(/*TssDescriptor*/tssd));
}

void
tssSetEntry_w7(uint8_t i, uint64_t base, uint64_t limit)
{
	struct TssDescriptor tssd;
	tssd.limitLow = limit & 0xffff;
	tssd.size = (limit >> 16) & 0xf;

	tssd.base_00 = base & 0xffff;
	tssd.base_16 = (base >> 16) & 0xff;
	tssd.base_24 = (base >> 24) & 0xff;
	tssd.base_32 = (base >> 32) & 0xffffffff;
	tssd.reserved = 0;

	tssd.type = (enum GdtType)(
		accessed |
		execute |
		ring3 |
		present
	);
	tmemcpy((void*)&gdtTemplate[i * 2], &tssd, sizeof(/*TssDescriptor*/tssd));
}

function gdt_write(uint16_t cs, uint16_t ds, uint16_t tr);
__attribute__((aligned(64))) Idtr cacheIdtr;

function enableInterrupts() {
	serialPrintln(u8"[cpu] initializing lgdt");

	tmemset((void*)&g_gdt_table, 0, sizeof(g_gdt_table));
	g_gdtr.limit = sizeof(g_gdt_table) - 1;
	g_gdtr.base = (uint64_t)(&g_gdt_table[0]);
	serialPrintf(u8"[cpu] lgdt limit %u base %u\n", g_gdtr.limit, g_gdtr.base);

	#define true 1
	#define false 0

	// Kernel CS/SS - always 64bit
	gdtSetEntry(1, 0, 0xfffff,  true, (enum GdtType)(read_write | execute));
	gdtSetEntry(2, 0, 0xfffff,  true, read_write);

	// User CS32/SS/CS64 - layout expected by SYSRET
	gdtSetEntry(3, 0, 0xfffff,  false, (enum GdtType)(ring3 | read_write | execute));
	gdtSetEntry(4, 0, 0xfffff,  true,  (enum GdtType)(ring3 | read_write));
	gdtSetEntry(5, 0, 0xfffff,  true,  (enum GdtType)(ring3 | read_write | execute));

	uint64_t sizeof_TssEntry = sizeof(g_tss);

	tmemset((void*)&g_tss, 0, sizeof_TssEntry);
	g_tss.iomap_offset = sizeof_TssEntry;
	g_tss.rsp[0] = (uint64_t)&rsp0stack;
	g_tss.rsp[1] = (uint64_t)&rsp1stack;
	g_tss.rsp[2] = (uint64_t)&rsp2stack;
	// TODO zero out stacks
	g_tss.ist[0] = (uint64_t)&rsp1stack;
	g_tss.ist[1] = (uint64_t)&rsp1stack;
	g_tss.ist[2] = (uint64_t)&rsp1stack;
	g_tss.ist[3] = (uint64_t)&rsp1stack;
	g_tss.ist[4] = (uint64_t)&rsp1stack;
	g_tss.ist[5] = (uint64_t)&rsp1stack;
	g_tss.ist[6] = (uint64_t)&rsp1stack;
	g_tss.ist[7] = (uint64_t)&rsp1stack;

	serialPrint(u8"[cpu] RSP[0] points to: ");
	serialPrintHex((uint64_t) g_tss.rsp[0]);
	serialPrint(u8"\n");

	serialPrint(u8"[cpu] RSP[1] points to: ");
	serialPrintHex((uint64_t) g_tss.rsp[1]);
	serialPrint(u8"\n");

	serialPrint(u8"[cpu] RSP[2] points to: ");
	serialPrintHex((uint64_t) g_tss.rsp[2]);
	serialPrint(u8"\n");

	uint64_t tssBase = (uint64_t)(&g_tss);
	// upload
	{
		serialPrintf(u8"[cpu] gdtTemplate[9 * 2] == %u\n", gdtTemplate[9 * 2]);
		serialPrintf(u8"[cpu] gdtTemplate[10 * 2] == %u\n", gdtTemplate[10 * 2]);
		tssSetEntry_w7(8, tssBase, sizeof_TssEntry);
		serialPrintf(u8"[cpu] gdtTemplate[9 * 2] == %u\n", gdtTemplate[9 * 2]);
		serialPrintf(u8"[cpu] gdtTemplate[10 * 2] == %u\n", gdtTemplate[10 * 2]);
	}

	// Note that this takes TWO GDT entries
	//tssSetEntry(6, tssBase, sizeof_TssEntry);

	//gdt_write(1 << 3, 2 << 3, 6 << 3);

	g_gdtr.limit = sizeof(gdtTemplate) - 1;
	g_gdtr.base = (uint64_t)(&gdtTemplate[0]);
	serialPrint(u8"[cpu] GDTR points to: ");
	serialPrintHex((uint64_t) &g_gdtr);
	serialPrint(u8"\n");
	serialPrint(u8"[cpu] GDT points to: ");
	serialPrintHex((uint64_t) g_gdtr.base);
	serialPrint(u8"\n");
	serialPrintf(u8"[cpu] GDT size is %u\n", g_gdtr.limit);
	serialPrintln(u8"[cpu] calling lgdt");
	lgdt(&g_gdtr);
	serialPrintln(u8"[cpu] calling ltr");
	{
		set_tsr(64);
	}
	//initializeMouse(&IDT[IRQ4]);

	serialPrintln(u8"[cpu] initializing foo_interrupt");
	for (uint32_t i = 0; i < 286; ++i)
	{
		initializeFallback( &IDT[i], (uint64_t)&foo_interrupt);
	}

	initializeFallback( &IDT[IRQ0], (uint64_t)(&timer_interrupt));
	cacheIdtr.limit = (sizeof(IdtEntry) * IDT_SIZE) - 1;
	cacheIdtr.offset = (uint64_t) IDT;

	serialPrint(u8"[cpu] IDTR points to: ");
	serialPrintHex((uint64_t) &cacheIdtr);
	serialPrint(u8"\n");
	serialPrint(u8"[cpu] IDT points to: ");
	serialPrintHex((uint64_t) &IDT);
	serialPrint(u8"\n");
	serialPrintf(u8"[cpu] IDT size is %u of %u elements of %u==16 size\n", cacheIdtr.limit, IDT_SIZE, sizeof(IdtEntry));
	serialPrintln(u8"[cpu] loading IDTR");
	// Before you implement the IDT, make sure you have a working GDT
	remapPic(0x20, 0x28);
	serialPrintln(u8"[cpu] calling lidtq");
	loadIdt(&cacheIdtr);


	serialPrintln(u8"[cpu] selectSegment of value 16");
	selectSegment((void*)&stub);

	// Masking IRQ to only support IRQ1 (keyboard)
	//writePort(IRQ1, 0xFC);
	// 0xFA - timer
	// 0xFC - timer and kb
	// 0xFD - kb
	// 0xFE - timer
	// 0xFF - none

	//writePort(IRQ1, 0xFD);
}
#endif

function enablePS2Mouse() {
	serialPrintln(u8"[cpu] begin: setting PS/2 mouse");
	uint8_t _status;
	//Enable the auxiliary mouse device
	mouseWait(1);
	writePort(PS2_CONTROL_PORT, 0xA8);

	/*

	IOWriteB(PS2_CONTROL_PORT, 0x20);
	ControllerConfig = IOReadB(PS2_DATA_PORT);
	ControllerConfig |=  0x02; // Turn on IRQ12
	ControllerConfig &= ~0x20; // Enable mouse clock line
	IOWriteB(PS2_CONTROL_PORT, 0x60);

	*/

	//Enable the interrupts
	mouseWait(1);
	writePort(PS2_CONTROL_PORT, 0x20);
	mouseWait(0);
	_status = (readPort(PS2_DATA_PORT) | 2); // Turn on IRQ12
	//_status &= ~0x20; // Enable mouse clock line
	mouseWait(1);
	writePort(PS2_CONTROL_PORT, 0x60);
	mouseWait(1);
	writePort(PS2_DATA_PORT, _status);
	//Tell the mouse to use default settings
	mouseWrite(0xF6);
	mouseRead(); // Acknowledge
	//Enable the mouse
	mouseWrite(0xF4);
	mouseRead(); // Acknowledge
	//Setup the mouse handler
//    irq_install_handler(12, mouseHandler);
	serialPrintln(u8"[cpu] done: setting PS/2 mouse");
	function quakePrintf(const char8_t *c, ...);
	quakePrintf(u8"Enabled PS/2 mouse and keyboard\n");
}

function mouseWait(uint8_t aType)
{
  uint32_t _timeOut=100000;
  if(aType==0)
  {
	while(_timeOut--) //Data
	{
	  if((readPort(PS2_CONTROL_PORT) & 1)==1)
	  {
		return;
	  }
	}
	return;
  }
  else
  {
	while(_timeOut--) //Signal
	{
	  if((readPort(PS2_CONTROL_PORT) & 2)==0)
	  {
		return;
	  }
	}
	return;
  }
}

function mouseWrite(uint8_t aWrite)
{
  //Wait to be able to send a command
  mouseWait(1);
  //Tell the mouse we are sending a command
  writePort(PS2_CONTROL_PORT, 0xD4);
  //Wait for the final part
  mouseWait(1);
  //Finally write
  writePort(PS2_DATA_PORT, aWrite);
}

uint8_t mouseRead()
{
  //Get's response from mouse
  mouseWait(0);
  return readPort(PS2_DATA_PORT);
}
