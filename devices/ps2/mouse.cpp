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

// PS/2 Mouse

volatile uint8_t mouseCycle = 0;
volatile uint8_t mouseByte[3];

// Note: those are controlled by DWM
volatile int16_t mouseX = 256;
volatile int16_t mouseY = 256;

// Note: those are controlled by driver
volatile bool leftDown = false;
volatile bool rightDown = false;
volatile bool middleDown = false;

enum class MouseActionType : uint8_t {
	Noop = 0,
	LeftDown,
	LeftUp,
	RightDown,
	RightUp,
	MiddleDown,
	MiddleUp,
	// TODO ScrollFromUser,
	// TODO ScrollToUser,
	Moved
};

struct MouseAction {
	int16_t mouseXdiff;
	int16_t mouseYdiff;
	MouseActionType type;
	// TODO time stamp
};

constexpr uint8_t mouseActionsLimit = 255;
volatile MouseAction mouseActionsZeta[mouseActionsLimit];
volatile MouseAction mouseActionsGama[mouseActionsLimit];
volatile bool mouseActionsUseZeta = false;
volatile uint8_t mouseActionsZetaAmount = 0;
volatile uint8_t mouseActionsGamaAmount = 0;

function pushMouseAction(MouseActionType type, int16_t mouseXdiff = 0, int16_t mouseYdiff = 0) {
	if (mouseActionsUseZeta) {
		if (mouseActionsZetaAmount == mouseActionsLimit)
			return;

		mouseActionsZeta[mouseActionsZetaAmount].mouseXdiff = mouseXdiff;
		mouseActionsZeta[mouseActionsZetaAmount].mouseYdiff = mouseYdiff;
		mouseActionsZeta[mouseActionsZetaAmount].type = type;

		mouseActionsZetaAmount++;
	} else {
		if (mouseActionsGamaAmount == mouseActionsLimit)
			return;

		mouseActionsGama[mouseActionsGamaAmount].mouseXdiff = mouseXdiff;
		mouseActionsGama[mouseActionsGamaAmount].mouseYdiff = mouseYdiff;
		mouseActionsGama[mouseActionsGamaAmount].type = type;

		mouseActionsGamaAmount++;
	}
}

uint8_t getBit(uint8_t byte, uint8_t bit) {
	return (byte & (1 << bit)) >> bit;
}

volatile bool lockMouse = false;
function handleMousePacket() {
	lockMouse = true;

	switch (mouseCycle) {
	case 0: {
		mouseByte[0] = mouseRead();
		if ((mouseByte[0] & 0x08) != 0) {
			mouseCycle++; // Only accept this as the first byte if the "must be 1" bit is set
		}
		// mouseCycle++;
		break;
	};
	case 1: {
		mouseByte[1] = mouseRead();
		mouseCycle++;
		break;
	};
	case 2: {
		mouseByte[2] = mouseRead();

		int16_t mouseXd = mouseByte[1];
		int16_t mouseYd = mouseByte[2];
		mouseYd = -mouseYd;

		// Overflow
		if (getBit(mouseByte[0], 6) != 0)
			mouseXd = 0;
		else if (getBit(mouseByte[0], 4) != 0)
			mouseXd -= (int16_t)0x100;

		if (getBit(mouseByte[0], 7) != 0)
			mouseYd = 0;
		else if (getBit(mouseByte[0], 5) != 0)
			mouseYd += (int16_t)0x100;

		mouseCycle = 0;

		// Ignore all state changes until message pool is ready
		// Otherwise we will have situaion like DOWN-UP-UP instead of D-U-D-U
		if (mouseActionsUseZeta && mouseActionsZetaAmount == mouseActionsLimit)
			break;
		if (!mouseActionsUseZeta && mouseActionsGamaAmount == mouseActionsLimit)
			break;

		if (mouseXd != 0 || mouseYd != 0)
			pushMouseAction(MouseActionType::Moved, mouseXd, mouseYd);

		const bool left = getBit(mouseByte[0], 0) != 0;
		const bool right = getBit(mouseByte[0], 1) != 0;
		const bool middle = getBit(mouseByte[0], 2) != 0;

		if (left == false && leftDown == true)
			pushMouseAction(MouseActionType::LeftUp);
		if (left == true && leftDown == false)
			pushMouseAction(MouseActionType::LeftDown);

		if (right == false && rightDown == true)
			pushMouseAction(MouseActionType::RightUp);
		if (right == true && rightDown == false)
			pushMouseAction(MouseActionType::RightDown);

		if (middle == false && middleDown == true)
			pushMouseAction(MouseActionType::MiddleUp);
		if (middle == true && middleDown == false)
			pushMouseAction(MouseActionType::MiddleDown);

		leftDown = left;
		rightDown = right;
		middleDown = middle;

		break;
	};
	}

	// EOI
	// Disable those lines if polling is used
	lockMouse = false;
}

function handleMouse() {
	handleMousePacket();
	uint8_t poll = readPort(0x64);
	while (getBit(poll, 0) == 1 && getBit(poll, 5) == 1) {
		handleMousePacket();
		poll = readPort(0x64);
	}
	writePort(0xA0, 0x20);
	writePort(0x20, 0x20);
}
