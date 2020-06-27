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

// Passively check for PS/2 updates

// TODO set xchng flag in IDT PS/2 interrupt, for avoiding polling in this case
// ^ clean that flag at the pollPS2Devices end to avoid racing

// ^ this may draw polling mechanism useless (poll for flag only in scheduler)
// ^ use hlt in scheduler

extern bool haveToRender;
extern uint8_t haveToQuake;
function handleKeyDown(uint8_t key);
extern function (*keyDownHandler)(uint8_t);
function quakeHandleButtonDown(uint8_t key);

// Returns 1 if something happened
const uint8_t PollingPS2SomethingHappened = 1;
const uint8_t PollingPS2NothingHappened = 0;
uint8_t pollPS2Devices() {
	uint8_t result = PollingPS2NothingHappened;
	uint8_t poll = readPort(0x64);
	while (getBit(poll, 0) == 1) {
		result = PollingPS2SomethingHappened;
		if (getBit(poll, 5) == 0) {
			handleKeyboard();
			if (keyboardPressedState[72])
				mouseY -= 4;
			if (keyboardPressedState[80])
				mouseY += 4;
			if (keyboardPressedState[77])
				mouseX += 4;
			if (keyboardPressedState[75])
				mouseX -= 4;
			if (keyboardPressedState[41])
				haveToQuake = !haveToQuake;
			keyDownHandler = null;
			if (haveToQuake)
				keyDownHandler = quakeHandleButtonDown;
			haveToRender = 1;
		} else if (getBit(poll, 5) == 1) {
			handleMouse();
			haveToRender = 1;
		}
		poll = readPort(0x64);
	}

	return result;
}
