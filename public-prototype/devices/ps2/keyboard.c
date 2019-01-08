/* http://www.osdever.net/bkerndev/Docs/keyboard.htm */
unsigned char keyboardMap[128] =
{
	0,
	27,
	'1',
	'2',
	'3',
	'4',
	'5',
	'6',
	'7',
	'8', // 9
	'9',
	'0',
	'-',
	'=',
	'\b', // Backspace
	'\t', // Tab
	'q',
	'w',
	'e',
	'r', // 19
	't',
	'y',
	'u',
	'i',
	'o',
	'p',
	'[',
	']',
	'\n', // Enter key
	0, 		// 29   - Control
	'a',
	's',
	'd',
	'f',
	'g',
	'h',
	'j',
	'k',
	'l',
	';', // 39
	'\'',
	'`',
	0, // Left shift
	'\\',
	'z',
	'x',
	'c',
	'v',
	'b',
	'n', // 49
	'm',
	',',
	'.',
	'/',
	0, 			// Right shift
	'*',
	0, // Alt
	' ', // Space bar
	0, // Caps lock
	0, // 59 - F1 key ... >
	0, 0, 0, 0, 0, 0, 0, 0,
	0, // < ... F10
	0, // 69 - Num lock
	0, // Scroll Lock
	0, // Home key
	0, // Up Arrow
	0, // Page Up
	'-',
	0, // Left Arrow
	0,
	0, // Right Arrow
	'+',
	0, // 79 - End key
	0, // Down Arrow
	0, // Page Down
	0, // Insert Key
	0, // Delete Key
	0, 0, 0,
	0, // F11 Key
	0, // F12 Key
	0, // All other keys are undefined
};

#define STATUS_REGISTER 0x64
#define DATA_PORT       0x60

void handleKeyboard() {
	/* EOI */
	serialPrint("handleKeyboard 0\r\n");
	writePort(PIC1_COMMAND, PIC_EOI);
	serialPrint("handleKeyboard 1\r\n");

	uint8_t status = readPort(STATUS_REGISTER);
	serialPrint("handleKeyboard 2\r\n");

	// PS2 Mouse
	if (status & 0x20) {
	//    uint8_t keycode = readPort(DATA_PORT);
	//    //if (keycode < 0) {
			return;
	//    //}
	//    //char buffer[] = {keyboardMap[keycode], 0};
	//    char buffer[] = {'M','o','u','s','e',':', 0};
	//    serialPrint(buffer);
	//    serialPrint(buffer);
	}

	if (status & 0x1) {
		serialPrint("handleKeyboard 3\r\n");
		uint8_t keycode = readPort(DATA_PORT);
		serialPrint("handleKeyboard 4\r\n");

		if (keycode < 0) {
			return;
		}
		serialPrint("handleKeyboard 5\r\n");

		if (keycode < 128 && keyboardMap[keycode]) {
			char buffer[] = {keyboardMap[keycode], 0};
			serialPrint("handleKeyboard 6\r\n");
			serialPrint(buffer);
			serialPrint("handleKeyboard 7\r\n");
			serialPrint(buffer);
		} else serialPrintf("[%d]", keycode);
		serialPrint("handleKeyboard 8\r\n");

		//if (status & 0x20) {
		//    char buffer[] = {'M', 0};
		//    serialPrint(buffer);
		//}
	} else {
		serialPrint("handleKeyboard status not 0x1!\r\n");
		char buffer[] = {'w', 0};
		serialPrint(buffer);
	}
}
