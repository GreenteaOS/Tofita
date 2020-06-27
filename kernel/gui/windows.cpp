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

// Window frames and some GUI elements

function drawHint(const char8_t *text, int16_t x, int16_t y) {
	let measure = getTextAdvance(text);
	let w = 6 * 2 + measure + 1;
	let h = 20;

	Pixel32 color;
	color.color = 0x57575757;
	drawRectangleOutline(color, x, y, w, h);
	color.color = 0xFFFFFFFF;
	drawRectangle(color, x + 1, y + 1, w - 2, h - 2);
	color.color = 0x57575757;
	drawAsciiText(text, x + 6, y + 5, color);
}
