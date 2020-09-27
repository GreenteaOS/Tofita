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

uint8_t pollPS2Devices() {

	uint8_t poll = readPort(0x64);

	while (getBit(poll, 0) == 1) {
		// Mouse has higher priority
		while (getBit(poll, 0) == 1 && getBit(poll, 5) == 1) {
			handleMousePacket();
			poll = readPort(0x64);
		}

		if (getBit(poll, 0) == 1 && getBit(poll, 5) == 0) {
			handleKeyboardPacket();
			poll = readPort(0x64);
		}
	}

	return 0;
}
