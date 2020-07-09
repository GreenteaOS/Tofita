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

function shadow(int16_t x, int16_t y, uint8_t darkness) {
	Pixel32 pixel;
	pixel.color = 0;
	pixel.rgba.a = 0xFF - darkness;
	blendPixel(x, y, pixel);
}

auto distance(int16_t x1, int16_t y1, int16_t x2, int16_t y2) {
	// TODO floor?
	return Math::round(Math::sqrt((x1 - x2) * (x1 - x2) + (y1 - y2) * (y1 - y2)));
}

function drawShadowBox(int16_t x, int16_t y, uint16_t w, uint16_t h) {
	for (int16_t iy = 0; iy < 16; ++iy)
		for (int16_t ix = 0; ix < w; ++ix) {
			shadow(x + ix, y - iy, 192 + iy * 4);
			shadow(x + ix, y + iy + h, 192 + iy * 4);
		}
	for (int16_t iy = 0; iy < h; ++iy)
		for (int16_t ix = 0; ix < 16; ++ix) {
			shadow(x - ix, y + iy, 192 + ix * 4);
			shadow(x + ix + w, y + iy, 192 + ix * 4);
		}
	for (int16_t iy = 1; iy < 15; ++iy)
		for (int16_t ix = 1; ix < 15; ++ix) {
			let dist = 192 + distance(x, y, x - ix, y - iy) * 4;
			if (dist > 255)
				continue;
			shadow(x - ix, y - iy, dist);
			shadow(x + ix + w - 1, y - iy, dist);
			shadow(x + ix + w - 1, y - 1 + iy + h, dist);
			shadow(x - ix, y - 1 + iy + h, dist);
		}
}

function drawWindowFrame(const wchar_t *title, int16_t x, int16_t y, uint16_t w, uint16_t h) {
	Pixel32 color;
	color.rgba.r = color.rgba.g = color.rgba.b = 0;

	drawVibrancedRectangle(x + 1, y + 1, w - 1, 32, false);

	drawText(title, x + 8 + 4, y + 9, color);

	// Frame hairline
	color.rgba.r = color.rgba.g = color.rgba.b = 164;
	drawRectangleOutline(color, x, y, w, h);

	// Minimize
	color.rgba.r = color.rgba.g = color.rgba.b = 229;
	color.rgba.a = 128;
	drawRectangleWithAlpha(color, x + w - (45 + 1) * 3, y + 1, 45, 29);

	color.rgba.r = color.rgba.g = color.rgba.b = 0;
	drawRectangle(color, x + w - (45 + 1) * 3 + 23 - 4, y + 20, 8, 1);

	// Maximize
	color.rgba.r = color.rgba.g = color.rgba.b = 229;
	color.rgba.a = 128;
	drawRectangleWithAlpha(color, x + w - (45 + 1) * 2, y + 1, 45, 29);

	color.rgba.r = color.rgba.g = color.rgba.b = 0;
	drawRectangle(color, x + w - (45 + 1) * 2 + 23 - 4, y + 14 - 4, 9, 1);
	drawRectangle(color, x + w - (45 + 1) * 2 + 23 - 4, y + 14 + 5, 10, 1);
	drawRectangle(color, x + w - (45 + 1) * 2 + 23 - 4, y + 14 - 4, 1, 9);
	drawRectangle(color, x + w - (45 + 1) * 2 + 23 + 5, y + 14 - 4, 1, 9);

	// Close
	color.rgba.b = 232;
	color.rgba.g = 17;
	color.rgba.r = 35;
	color.rgba.a = 128;
	drawRectangleWithAlpha(color, x + w - (45 + 1) * 1, y + 1, 45, 29);

	color.rgba.r = color.rgba.g = color.rgba.b = 255;
	line45smooth(color, x + w - (45 + 1) * 1 + 23 - 6, y + 10, 10, 1);
	line45smooth(color, x + w - 18, y + 10, 10, -1);

	// Shadow
	drawShadowBox(x - 1, y - 1, w + 1, h + 1);
}
