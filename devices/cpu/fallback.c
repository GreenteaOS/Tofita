void handle_fallback0()  { /*serialPrint("........................................................<handle_fallback   0 Programmable Interrupt Timer Interrupt >\r\n");/**/ writePort(PIC1_COMMAND_0x20, PIC_EOI_0x20); }
void handle_fallback1()  { serialPrintln("<<<<<<<<<<<<<<handle_fallback   1 Keyboard Interrupt >>>>>>>>>>>>>>>\r\n"); writePort(PIC1_COMMAND_0x20, PIC_EOI_0x20); }
void handle_fallback2()  { serialPrintln("<<<<<<<<<<<<<<handle_fallback   2 Cascade (used internally by the two PICs. never raised) >>>>>>>>>>>>>>>\r\n"); writePort(PIC1_COMMAND_0x20, PIC_EOI_0x20); }
void handle_fallback3()  { serialPrintln("<<<<<<<<<<<<<<handle_fallback   3 COM2 (if enabled) >>>>>>>>>>>>>>>\r\n"); writePort(PIC1_COMMAND_0x20, PIC_EOI_0x20); }
void handle_fallback4()  { serialPrintln("<<<<<<<<<<<<<<handle_fallback   4 COM1 (if enabled) >>>>>>>>>>>>>>>\r\n"); writePort(PIC1_COMMAND_0x20, PIC_EOI_0x20); }
void handle_fallback5()  { serialPrintln("<<<<<<<<<<<<<<handle_fallback   5 LPT2 (if enabled) >>>>>>>>>>>>>>>\r\n"); writePort(PIC1_COMMAND_0x20, PIC_EOI_0x20); }
void handle_fallback6()  { serialPrintln("<<<<<<<<<<<<<<handle_fallback   6 Floppy Disk >>>>>>>>>>>>>>>\r\n"); writePort(PIC1_COMMAND_0x20, PIC_EOI_0x20); }
void handle_fallback7()  { serialPrintln("<<<<<<<<<<<<<<handle_fallback   7 LPT1 / Unreliable spurious interrupt (usually) >>>>>>>>>>>>>>>\r\n"); writePort(PIC1_COMMAND_0x20, PIC_EOI_0x20); }
void handle_fallback8()  { serialPrintln("<<<<<<<<<<<<<<handle_fallback   8 CMOS real-time clock (if enabled) >>>>>>>>>>>>>>>\r\n"); writePort(0xA0, 0x20); writePort(PIC1_COMMAND_0x20, PIC_EOI_0x20); }
void handle_fallback9()  { serialPrintln("<<<<<<<<<<<<<<handle_fallback   9 Free for peripherals / legacy SCSI / NIC >>>>>>>>>>>>>>>\r\n"); writePort(0xA0, 0x20); writePort(PIC1_COMMAND_0x20, PIC_EOI_0x20); }
void handle_fallback10() { serialPrintln("<<<<<<<<<<<<<<handle_fallback 10 Free for peripherals / SCSI / NIC >>>>>>>>>>>>>>>\r\n"); writePort(0xA0, 0x20); writePort(PIC1_COMMAND_0x20, PIC_EOI_0x20); }
void handle_fallback11() { serialPrintln("<<<<<<<<<<<<<<handle_fallback 11 Free for peripherals / SCSI / NIC >>>>>>>>>>>>>>>\r\n"); writePort(0xA0, 0x20); writePort(PIC1_COMMAND_0x20, PIC_EOI_0x20); }
void handle_fallback12() { serialPrintln("<<<<<<<<<<<<<<handle_fallback 12 PS2 Mouse >>>>>>>>>>>>>>>\r\n"); writePort(0xA0, 0x20); writePort(PIC1_COMMAND_0x20, PIC_EOI_0x20); }
void handle_fallback13() { serialPrintln("<<<<<<<<<<<<<<handle_fallback 13 FPU / Coprocessor / Inter-processor >>>>>>>>>>>>>>>\r\n"); writePort(0xA0, 0x20); writePort(PIC1_COMMAND_0x20, PIC_EOI_0x20); }
void handle_fallback14() { serialPrintln("<<<<<<<<<<<<<<handle_fallback 14 Primary ATA Hard Disk >>>>>>>>>>>>>>>\r\n"); writePort(0xA0, 0x20); writePort(PIC1_COMMAND_0x20, PIC_EOI_0x20); }
void handle_fallback15() { serialPrintln("<<<<<<<<<<<<<<handle_fallback 15 Secondary ATA Hard Disk  >>>>>>>>>>>>>>>\r\n"); writePort(0xA0, 0x20); writePort(PIC1_COMMAND_0x20, PIC_EOI_0x20); }
