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

// Quake-style console with ~

uint8_t quakeHeight = 255;
char8_t quakeCommand[256] = {0};
char8_t quakeLines[17][256] = {0};
uint8_t quakeLine = 0;
uint8_t quakeRow = 0;
double quakeAdvance = 0.0;
uint8_t quakeCommandSize = 0;

function quakePrintf(const char8_t *c, ...);

function quake() {
	if (haveToQuake == 0) {
		return;
	}

	drawVibrancedRectangle(0, 0, _framebuffer->width, quakeHeight);
	Pixel32 color;
	// color.rgba.r = color.rgba.g = color.rgba.b = 48;
	color.rgba.r = color.rgba.g = color.rgba.b = 0xDD;
	var x = drawAsciiText(quakeCommand, 2, quakeHeight - 14, color);
	drawAsciiText(u8"|", x + 2, quakeHeight - 15, color);

	for (var i = 0; i < 17; i++) {
		drawAsciiText(quakeLines[i], 2, i * 14 + 2, color);
	}
}

function quakeHandleButtonDown(uint8_t key) {
	serialPrintf(u8"quakeHandleButtonDown %d\n", key);

	if (keyboardMap[key] == '\b' && quakeCommandSize > 0) {
		quakeCommand[quakeCommandSize - 1] = 0;
		quakeCommandSize--;
	}

	if (keyboardMap[key] == '\n' && quakeCommandSize > 0) {
		if (quakeCommand[0] == 'r' && quakeCommand[1] == 'e') {
			extern const KernelParams *paramsCache;
			quakePrintf(u8"Doing hot reload. Note: this log may be persistent between hot reloads\n");
			InitKernel start = (InitKernel)KernelVirtualBase;
			start(paramsCache); // TODO pml4, stack
		} else if (quakeCommand[0] == 'h' && quakeCommand[1] == 'e') {
			quakePrintf(u8"Hit `~` to show/hide this terminal\n", quakeCommand);
			quakePrintf(u8"Command 'reload' does quick kernel restart (without actual reboot) Note: this "
						u8"destroys all data!\n",
						quakeCommand);
		} else {
			quakePrintf(u8"Command '%s' not supported\n", quakeCommand);
			quakePrintf(u8"Enter 'help' for commands\n");
		}
		for (uint8_t i = 0; i < 255; i++)
			quakeCommand[i] = 0;
		quakeCommandSize = 0;
	}

	// TODO 123 etc
	if ((keyboardMap[key] >= 'a' && keyboardMap[key] <= 'z') || (keyboardMap[key] == ' '))
		if (quakeCommandSize < 255) {
			quakeCommand[quakeCommandSize] = keyboardMap[key];
			quakeCommandSize++;
		}

	serialPrintf(u8"quake command is %s\n", quakeCommand);
}

function qsod(const char8_t *format, const uint64_t extra, const uint64_t more) {
	haveToQuake = 1;
	quakePrintf(u8"Kernel stopped working. Please, reboot.\n");
	quakePrintf(format, extra, more);
}

uint8_t *_quakeItoA(int32_t i, uint8_t b[]) {
	uint8_t const digit[] = "0123456789";
	uint8_t *p = b;
	if (i < 0) {
		*p++ = '-';
		i *= -1;
	}
	int32_t shifter = i;
	do { // Move to where representation ends
		++p;
		shifter = shifter / 10;
	} while (shifter);
	*p = '\0';
	do { // Move back, inserting digits as u go
		*--p = digit[i % 10];
		i = i / 10;
	} while (i);
	return b;
}

function _quake_newline() {
	quakeLine++;
	quakeRow = 0;
	quakeAdvance = 0.0;
	if (quakeLine == 17) {
		// Move all lines upper
		quakeLine = 16;
		for (var i = 0; i < 16; i++) {
			for (var k = 0; k < sizeof(quakeLines[i]); k++)
				quakeLines[i][k] = 0;
			for (var k = 0; k < sizeof(quakeLines[i]); k++)
				quakeLines[i][k] = quakeLines[i + 1][k];
		}
		for (var i = 0; i < sizeof(quakeLines[16]); i++)
			quakeLines[16][i] = 0;
	}
}

uint8_t __cdecl _quake_putchar(const uint8_t c) {
	double getCharAdvance(const char8_t c);

	if (c != 0) {
		putchar(c); // Copy to serial log
		if (c == '\n') {
			_quake_newline();
			return c;
		}
		quakeAdvance += getCharAdvance(c);
		if (quakeAdvance > _framebuffer->width) {
			_quake_newline();
			quakeAdvance = getCharAdvance(c);
		}
		quakeLines[quakeLine][quakeRow] = c;
		quakeRow++;
		if (quakeRow > 255)
			_quake_newline();
		return c;
	}
	return EOF;
}

int32_t _quake_puts(const uint8_t *string) {
	int32_t i = 0;
	while (string[i] != 0) {
		if (_quake_putchar(string[i]) == EOF) {
			return EOF;
		}
		i++;
	}
	return 1;
}

function quakePrintHex(uint64_t n) {
	_quake_puts((const uint8_t *)"0x");
	uint8_t buf[16], *bp = buf + 16;
	for (int32_t i = 0; i < 16; i++)
		buf[i] = '0';
	do {
		bp--;
		uint8_t mod = n % 16;
		if (mod < 10) {
			*bp = '0' + mod;
		} else {
			*bp = 'A' - 10 + mod;
		}
		n /= 16;
	} while (n != 0);
	for (int32_t i = 0; i < 16; i++)
		_quake_putchar(buf[i]);
}

function quakePrintf(const char8_t *c, ...) {
	uint8_t *s;
	va_list lst;
	va_start(lst, c);
	while (*c != '\0') {
		if (*c != '%') {
			_quake_putchar(*c);
			c++;
			continue;
		}

		c++;

		if (*c == '\0') {
			break;
		}

		switch (*c) {
		case 's':
			_quake_puts(va_arg(lst, uint8_t *));
			break;
		case 'c':
			_quake_putchar(va_arg(lst, int32_t));
			break;
		case 'd': {
			int32_t value = va_arg(lst, int32_t);
			uint8_t buffer[16];
			for (uint8_t i = 0; i < 16; i++)
				buffer[i] = 0;
			_quakeItoA(value, buffer);
			uint8_t *c = buffer;
			while (*c != '\0') {
				_quake_putchar(*c);
				c++;
			}
			break;
		}
		case 'u': {
			uint32_t value = va_arg(lst, uint32_t);
			uint8_t buffer[16];
			for (uint8_t i = 0; i < 16; i++)
				buffer[i] = 0;
			_quakeItoA(value, buffer);
			uint8_t *c = buffer;
			while (*c != '\0') {
				_quake_putchar(*c);
				c++;
			}
			break;
		}
		case '8': {
			uint64_t value = va_arg(lst, uint64_t);
			quakePrintHex(value);
			break;
		}
		}
		c++;
	}
}
