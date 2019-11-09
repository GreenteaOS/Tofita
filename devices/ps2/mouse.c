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

// PS/2 Mouse

uint8_t mouseCycle = 0;
int8_t mouseByte[3];
int16_t mouseX = 256;
int16_t mouseY = 256;

function handleMouseDown(uint8_t key);
function handleMouseUp(uint8_t key);

uint8_t getBit(uint8_t byte, uint8_t bit) {
	return (byte & ( 1 << bit )) >> bit;
}

function handleMouse() {
	switch(mouseCycle) {
	case 0: {
		mouseByte[0] = mouseRead();
		if((mouseByte[0] & 0x08) != 0) {
			mouseCycle++; // Only accept this as the first byte if the "must be 1" bit is set
		}
		//mouseCycle++;
		break;
	};
	case 1: {
		mouseByte[1] = mouseRead();
		mouseCycle++;
		break;
	};
	case 2: {
		mouseByte[2] = mouseRead();
		int8_t mouseXd = mouseByte[1];
		int8_t mouseYd = mouseByte[2];
		mouseCycle = 0;
		mouseX += mouseXd;
		mouseY -= mouseYd;

		if (getBit(mouseByte[0], 0) != 0) serialPrintln("[mouse] left button is down");
		if (getBit(mouseByte[0], 1) != 0) serialPrintln("[mouse] right button is down");
		if (getBit(mouseByte[0], 2) != 0) serialPrintln("[mouse] middle button is down");

		if ((getBit(mouseByte[0], 0) != 0) || (getBit(mouseByte[0], 1) != 0))
			handleMouseDown(0);
		else
			handleMouseUp(0);

		serialPrintf("[mouseByte[0]] 1:%d 2:%d 3:%d 4:%d 5:%d 6:%d 7:%d 8:%d bits are set\n",
			getBit(mouseByte[0], 0),
			getBit(mouseByte[0], 1),
			getBit(mouseByte[0], 2),
			getBit(mouseByte[0], 3),
			getBit(mouseByte[0], 4),
			getBit(mouseByte[0], 5),
			getBit(mouseByte[0], 6),
			getBit(mouseByte[0], 7)
		);

		break;
	};
	}

	// EOI
	// Disabled cause polling is used
	//writePort(0xA0, 0x20);
	//writePort(0x20, 0x20);
}
