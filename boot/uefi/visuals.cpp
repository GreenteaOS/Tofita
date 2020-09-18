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

// trampoline.asm -> incbin
extern const uint8_t binFont[];
extern const uint8_t binFontBitmap[];
const uint32_t fontWidth = 512;
const uint32_t fontHeight = 32;

extern const uint8_t binLeavesBitmap[];
const uint32_t leavesWidth = 36;
const uint32_t leavesHeight = 25;

int64_t _fltused = 0;

function drawPixel(Framebuffer *framebuffer, uint32_t x, uint32_t y, uint32_t color) {
	uint32_t *pixels = (uint32_t *)framebuffer->base;
	pixels[(y * framebuffer->pixelsPerScanLine) + x] = color;
}

struct UefiPixel {
	uint8_t b;
	uint8_t g;
	uint8_t r;
	uint8_t a;
} __attribute__((packed));

_Static_assert(sizeof(UefiPixel) == 4, "UefiPixel has to be 32 bit");

uint32_t getBitmapPixel(const uint8_t *bmp, uint32_t x, uint32_t y, uint32_t width, uint32_t height) {
	let colors = bmp + 54;
	UefiPixel pixel;
	pixel.b = colors[((height - y - 1) * width + x) * 3 + 0];
	pixel.g = colors[((height - y - 1) * width + x) * 3 + 1];
	pixel.r = colors[((height - y - 1) * width + x) * 3 + 2];
	pixel.a = 0xFF;
	uint32_t *color = (uint32_t *)&pixel;
	return *color;
}

#pragma pack(1)
struct TextFontList {
	uint16_t x;
	uint16_t y;
	double width;
} __attribute__((packed));
#pragma pack()

_Static_assert(sizeof(TextFontList) == 12, "TextFontList has to be 12 byte");

TextFontList *textFontList = null;

function initText() {
	textFontList = (TextFontList *)binFont;
}

double getCharAdvance(const char8_t c) {
	TextFontList textChar = textFontList[(uint64_t)c];
	return textChar.width;
}

double drawChar(const char8_t c, double x, int16_t y, Framebuffer *framebuffer) {
	TextFontList textChar = textFontList[(uint64_t)c];

	int16_t xx = (int32_t)(x + 0.5);
	uint16_t w = (int32_t)(textChar.width + 0.5);

	for (uint16_t yi = 0; yi < 12; yi++)
		for (uint16_t xi = 0; xi < w; xi++) {
			const uint32_t atx = xx + xi;
			const uint32_t aty = y + yi;

			let font = getBitmapPixel(binFontBitmap, textChar.x + xi, textChar.y + yi, fontWidth, fontHeight);

			drawPixel(framebuffer, atx, aty, font);
		}

	return textChar.width;
}

function drawText(const wchar_t *text, int16_t y, Framebuffer *framebuffer) {
	uint16_t i = 0;

	// Get width to center text on screen
	double width = 0;
	while ((uint16_t)text[i] != 0 && i < 255 * 255) {
		width += getCharAdvance((uint16_t)text[i] & 0xFF);
		i++;
	}
	double xx = framebuffer->width / 2 - width / 2;

	i = 0;
	while ((uint16_t)text[i] != 0 && i < 255 * 255) {
		xx += drawChar((uint16_t)text[i] & 0xFF, xx, y, framebuffer);
		i++;
	}
}
