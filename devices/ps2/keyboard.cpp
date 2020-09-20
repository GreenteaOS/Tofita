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

// Based on http://www.osdever.net/bkerndev/Docs/keyboard.htm

uint8_t keyboardPressedState[128] = {0};
const uint8_t keyboardMap[128] = {
	0,	  27,  '1', '2', '3', '4', '5', '6', '7',
	'8', // 9
	'9',  '0', '-', '=',
	'\b', // Backspace
	'\t', // Tab
	'q',  'w', 'e',
	'r', // 19
	't',  'y', 'u', 'i', 'o', 'p', '[', ']',
	'\n', // Enter key
	0,	  // 29 - Control
	'a',  's', 'd', 'f', 'g', 'h', 'j', 'k', 'l',
	';', // 39
	'\'', '`',
	0, // Left shift
	'\\', 'z', 'x', 'c', 'v', 'b',
	'n', // 49
	'm',  ',', '.', '/',
	0, // Right shift
	'*',
	0,	 // Alt
	' ', // Space bar
	0,	 // Caps lock
	0,	 // 59 - F1 key ... >
	0,	  0,   0,	0,	 0,	  0,   0,	0,
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
	0,	  0,   0,
	0, // F11 Key
	0, // F12 Key
	0, // All other keys are undefined
};

#define STATUS_REGISTER 0x64
#define DATA_PORT 0x60

function handleKeyboardPacket() {


	{
		uint8_t keycode = readPort(DATA_PORT);

		if (keycode < 0) {
			return;
		}

		if (keycode < 128) {
			wchar_t buffer[] = {(char8_t)keyboardMap[keycode], 0};
			if (buffer[0] == 0)
				buffer[0] = '?';
			serialPrint(L"[keyboard] [");
			serialPrint(buffer);
			serialPrintf(L" down] %d keycode\n", keycode);
			keyboardPressedState[keycode] = 1;
			if (haveToQuake)
				quakeHandleButtonDown(keycode);
			else {
			}
		} else {
			keycode = keycode - 128;
			wchar_t buffer[] = {(char8_t)keyboardMap[keycode], 0};
			if (buffer[0] == 0)
				buffer[0] = '?';
			serialPrint(L"[keyboard] [");
			serialPrint(buffer);
			serialPrintf(L" up] %d keycode\n", keycode + 128);
			keyboardPressedState[keycode] = 0;
		}

		// if (status & 0x20) {
		//    uint8_t buffer[] = {'M', 0};
		//    serialPrint(buffer);
		//}
	}

	// Note: Quake terminal has higher priority than kernel itself
	if (keyboardPressedState[41]) {
		haveToQuake = !haveToQuake;
		// TODO DWM
		haveToRender = 1;
	}

	// EOI
	// Disable this line if polling is used
	writePort(PIC1_COMMAND, PIC_EOI);
}

function handleKeyboard() {
}
