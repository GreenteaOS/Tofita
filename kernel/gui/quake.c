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

// Quake-style console with ~

uint8_t haveToQuake = 0;
uint8_t quakeHeight = 255;
char quakeCommand[256] = {0};
char quakeLines[17][256] = {0};
uint8_t quakeLine = 0;
uint8_t quakeRow = 0;
double quakeAdvance = 0.0;
uint8_t quakeCommandSize = 0;

void quakePrintf(const char *c, ...);

void quake() {
	if (haveToQuake == 0) {
		return ;
	}

	void drawVibrancedRectangle(int16_t x, int16_t y, uint16_t width, uint16_t height);
	drawVibrancedRectangle(0, 0, _framebuffer->width, quakeHeight);
	uint16_t drawAsciiText(const char* text, double x, uint16_t y, Pixel32 color);
	Pixel32 color;
	color.rgba.r = color.rgba.g = color.rgba.b = 48;
	auto x = drawAsciiText(quakeCommand, 2, quakeHeight - 14, color);
	drawAsciiText("|", x + 2, quakeHeight - 15, color);

	for (auto i = 0; i < 17; i++) {
		drawAsciiText(quakeLines[i], 2, i * 14 + 2, color);
	}
}

void quakeHandleButtonDown(uint8_t key) {
	serialPrintf("quakeHandleButtonDown %d\n", key);

    if (keyboardMap[key] == '\b' && quakeCommandSize > 0) {
    	quakeCommand[quakeCommandSize - 1] = 0;
    	quakeCommandSize--;
    }

    if (keyboardMap[key] == '\n' && quakeCommandSize > 0) {
    		quakePrintf("Enter 'help' for commands\n");
    		quakePrintf("Command '%s' not supported\n", quakeCommand);
    	for (uint8_t i = 0; i < 255; i++) quakeCommand[i] = 0;
    	quakeCommandSize = 0;
    }

    // TODO 123 etc
    if (
    	(keyboardMap[key] >= 'a' && keyboardMap[key] <= 'z')
    	||
    	(keyboardMap[key] == ' ')
    )
    if (quakeCommandSize < 255) {
    	quakeCommand[quakeCommandSize] = keyboardMap[key];
    	quakeCommandSize++;
    }

	serialPrintf("quake command is %s\n", quakeCommand);
}

uint8_t* _quakeItoA(int i, uint8_t b[]){
	uint8_t const digit[] = "0123456789";
	uint8_t* p = b;
	if (i<0) {
		*p++ = '-';
		i *= -1;
	}
	int shifter = i;
	do { //Move to where representation ends
		++p;
		shifter = shifter/10;
	} while(shifter);
	*p = '\0';
	do { //Move back, inserting digits as u go
		*--p = digit[i%10];
		i = i/10;
	} while(i);
	return b;
}

void _quake_newline() {
	quakeLine++;
	quakeRow = 0;
	quakeAdvance = 0.0;
	if (quakeLine == 17) {
		// Move all lines upper
		quakeLine = 16;
		for (auto i = 0; i < 16; i++) {
			for (auto k = 0; k < sizeof(quakeLines[i]); k++) quakeLines[i][k] = 0;
			for (auto k = 0; k < sizeof(quakeLines[i]); k++) quakeLines[i][k] = quakeLines[i + 1][k];
		}
		for (auto i = 0; i < sizeof(quakeLines[16]); i++) quakeLines[16][i] = 0;
	}
}

uint8_t __cdecl _quake_putchar(const uint8_t c) {
	double getCharAdvance(const char c);

	if (c != 0) {
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
		if (quakeRow > 255) _quake_newline();
		return c;
	}
	return EOF;
}

int _quake_puts(const uint8_t *string)
{
	int i = 0;
	while (string[i] != 0)
	{
		if (_quake_putchar(string[i]) == EOF)
		{
			return EOF;
		}
		i++;
	}
	return 1;
}

void quakePrintf(const char *c, ...) {
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

		switch (*c)
		{
			case 's': _quake_puts(va_arg(lst, uint8_t *)); break;
			case 'c': _quake_putchar(va_arg(lst, int)); break;
			case 'd': {
				int value = va_arg(lst, int);
				uint8_t buffer[16];
				for (uint8_t i = 0; i < 16; i++) buffer[i] = 0;
				_quakeItoA(value, buffer);
				uint8_t *c = buffer;
				while (*c != '\0') {
					_quake_putchar(*c);
					c++;
				}
				break;
			}
		}
		c++;
	}
}
