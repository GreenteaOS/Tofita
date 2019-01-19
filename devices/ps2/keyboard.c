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

uint8_t keyboardPressedState[128];
uint8_t keyboardPressedStates = 0;
uint8_t keyboardMap[128] =
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
	0, // 29 - Control
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
	0, // Right shift
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
	// TODO move this to init
	if (keyboardPressedStates == 0) for (uint8_t i = 0; i < 128; i++) {
		// Cleanup states
		keyboardPressedState[i] = 0;
	}

	uint8_t status = readPort(STATUS_REGISTER);

	// PS2 Mouse
	if (status & 0x20) {
	//    uint8_t keycode = readPort(DATA_PORT);
	//    //if (keycode < 0) {
			return;
	//    //}
	//    //uint8_t buffer[] = {keyboardMap[keycode], 0};
	//    uint8_t buffer[] = {'M','o','u','s','e',':', 0};
	//    serialPrint(buffer);
	//    serialPrint(buffer);
	}

	if (status & 0x1) {
		uint8_t keycode = readPort(DATA_PORT);

		if (keycode < 0) {
			return;
		}

		if (keycode < 128) {
			uint8_t buffer[] = {keyboardMap[keycode], 0};
			if (buffer[0] == 0) buffer[0] = '?';
			serialPrint("[keyboard] [");
			serialPrint(buffer);
			if (keyboardPressedState[keycode] == 0) keyboardPressedStates++;
			serialPrintf(" down] %d keys are down, %d keycode\r\n", keyboardPressedStates, keycode);
			keyboardPressedState[keycode] = 1;
		} else {
			keycode = keycode - 128;
			uint8_t buffer[] = {keyboardMap[keycode], 0};
			if (buffer[0] == 0) buffer[0] = '?';
			serialPrint("[keyboard] [");
			serialPrint(buffer);
			if (keyboardPressedState[keycode] != 0) keyboardPressedStates--;
			serialPrintf(" up] %d keys are down, %d keycode\r\n", keyboardPressedStates, keycode + 128);
			keyboardPressedState[keycode] = 0;
		}

		//if (status & 0x20) {
		//    uint8_t buffer[] = {'M', 0};
		//    serialPrint(buffer);
		//}
	} else {
		serialPrint("handleKeyboard status not 0x1!\r\n");
		uint8_t buffer[] = {'w', 0};
		serialPrint(buffer);
	}

	// EOI
	writePort(PIC1_COMMAND, PIC_EOI);
}
