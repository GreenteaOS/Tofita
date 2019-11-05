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
uint8_t quakeCommandSize = 0;

void quake() {
	if (haveToQuake == 0) {
		return ;
	}

	void drawVibrancedRectangle(int16_t x, int16_t y, uint16_t width, uint16_t height);
	drawVibrancedRectangle(0, 0, _framebuffer->width, quakeHeight);
	uint16_t drawAsciiText(const char* text, double x, uint16_t y);
	drawAsciiText(quakeCommand, 2, quakeHeight - 11);
}

void quakeHandleButtonDown(uint8_t key) {
	serialPrintf("quakeHandleButtonDown %d\n", key);

    if (keyboardMap[key] == '\b' && quakeCommandSize > 0) {
    	quakeCommand[quakeCommandSize - 1] = 0;
    	quakeCommandSize--;
    }

    if (keyboardMap[key] == '\n' && quakeCommandSize > 0) {
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

int __cdecl _quake_putchar(int c) {
	if (c) {

		return c;
	}
	return EOF;
}

int _quake_puts(const uint8_t *string)
{
	int i = 0;
	while (string[i]) //standard c idiom for looping through a null-terminated string
	{
		if (_quake_putchar(string[i]) == EOF) //if we got the EOF value from writing the uint8_t
		{
			return EOF;
		}
		i++;
	}
	if (_quake_putchar('\n') == EOF) //this will occur right after we quit due to the null terminated character.
	{
		return EOF;
	}
	return 1; //to meet spec.
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
