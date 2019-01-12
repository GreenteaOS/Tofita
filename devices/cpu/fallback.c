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

// Interruptions fallbacks if no interruption handler defined

void handle_fallback0() {
	// Commented because annoying
	//serialPrintln("<Interruption fallback 0 Programmable Interrupt Timer Interrupt>");
	writePort(PIC1_COMMAND_0x20, PIC_EOI_0x20);
}

void handle_fallback1() {
	serialPrintln("<Interruption fallback 1 Keyboard Interrupt>");
	writePort(PIC1_COMMAND_0x20, PIC_EOI_0x20);
}

void handle_fallback2() {
	serialPrintln("<Interruption fallback 2 Cascade (used internally by the two PICs. never raised)>");
	writePort(PIC1_COMMAND_0x20, PIC_EOI_0x20);
}

void handle_fallback3() {
	serialPrintln("<Interruption fallback 3 COM2 (if enabled)>");
	writePort(PIC1_COMMAND_0x20, PIC_EOI_0x20);
}

void handle_fallback4() {
	serialPrintln("<Interruption fallback 4 COM1 (if enabled)>");
	writePort(PIC1_COMMAND_0x20, PIC_EOI_0x20);
}

void handle_fallback5() {
	serialPrintln("<Interruption fallback 5 LPT2 (if enabled)>");
	writePort(PIC1_COMMAND_0x20, PIC_EOI_0x20);
}

void handle_fallback6() {
	serialPrintln("<Interruption fallback 6 Floppy Disk>");
	writePort(PIC1_COMMAND_0x20, PIC_EOI_0x20);
}

void handle_fallback7() {
	serialPrintln("<Interruption fallback 7 LPT1 / Unreliable spurious interrupt (usually)>");
	writePort(PIC1_COMMAND_0x20, PIC_EOI_0x20);
}

void handle_fallback8() {
	serialPrintln("<Interruption fallback 8 CMOS real-time clock (if enabled)>");
	writePort(0xA0, 0x20);
	writePort(PIC1_COMMAND_0x20, PIC_EOI_0x20);
}

void handle_fallback9() {
	serialPrintln("<Interruption fallback 9 Free for peripherals / legacy SCSI / NIC>");
	writePort(0xA0, 0x20);
	writePort(PIC1_COMMAND_0x20, PIC_EOI_0x20);
}

void handle_fallback10() {
	serialPrintln("<Interruption fallback 10 Free for peripherals / SCSI / NIC>");
	writePort(0xA0, 0x20);
	writePort(PIC1_COMMAND_0x20, PIC_EOI_0x20);
}

void handle_fallback11() {
	serialPrintln("<Interruption fallback 11 Free for peripherals / SCSI / NIC>");
	writePort(0xA0, 0x20);
	writePort(PIC1_COMMAND_0x20, PIC_EOI_0x20);
}

void handle_fallback12() {
	serialPrintln("<Interruption fallback 12 PS2 Mouse>");
	writePort(0xA0, 0x20);
	writePort(PIC1_COMMAND_0x20, PIC_EOI_0x20);
}

void handle_fallback13() {
	serialPrintln("<Interruption fallback 13 FPU / Coprocessor / Inter-processor>");
	writePort(0xA0, 0x20);
	writePort(PIC1_COMMAND_0x20, PIC_EOI_0x20);
}

void handle_fallback14() {
	serialPrintln("<Interruption fallback 14 Primary ATA Hard Disk>");
	writePort(0xA0, 0x20);
	writePort(PIC1_COMMAND_0x20, PIC_EOI_0x20);
}

void handle_fallback15() {
	serialPrintln("<Interruption fallback 15 Secondary ATA Hard Disk >");
	writePort(0xA0, 0x20);
	writePort(PIC1_COMMAND_0x20, PIC_EOI_0x20);
}
