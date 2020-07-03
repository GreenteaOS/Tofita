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

// Simple bitmap font drawing
// Non-monospace

Bitmap32 *textFontBitmap;
extern Pixel32 *_pixels;

typedef struct {
	uint16_t x;
	uint16_t y;
	double width;
} __attribute__((packed)) TextFontList;

TextFontList *textFontList = null;

function initText() {
	RamDiskAsset textFont = getRamDiskAsset(u8"ascii.tofita");
	textFontList = (TextFontList *)textFont.data;

	Bitmap32 *loadPng32(const RamDiskAsset *asset);
	RamDiskAsset a = getRamDiskAsset(u8"root/Windows/Fonts/default.png");
	textFontBitmap = loadPng32(&a);
}

int8_t getLigatureAdvance(const char8_t left, const char8_t right) {
	if (left == 0)
		return 0;
	if (right == 0)
		return 0;
	return 8;
}

double getCharAdvance(const char8_t c) {
	if (textFontList == null)
		return 8;
	TextFontList textChar = textFontList[(uint64_t)c];
	return textChar.width;
}

uint16_t getTextAdvance(const char8_t *text) {
	uint16_t i = 0;
	double xx = 0;
	while (text[i] != 0 && i < 255 * 255) {
		xx += textFontList[text[i]].width;
		i++;
	}
	return xx;
}

double drawChar(const char8_t c, double x, int16_t y, Pixel32 color) {
	TextFontList textChar = textFontList[(uint64_t)c];

	int16_t xx = (int32_t)(x + 0.5);
	uint8_t w = (int32_t)(textChar.width + 0.5);

	for (uint8_t yi = 0; yi < 12; yi++)
		for (uint8_t xi = 0; xi < w; xi++) {
			const int32_t atx = xx + xi;
			const int32_t aty = y + yi;

			if (atx >= _framebuffer->width)
				continue;
			if (atx < 0)
				continue;

			if (aty >= _framebuffer->height)
				continue;
			if (aty < 0)
				continue;

			Pixel32 font =
				textFontBitmap->pixels[(textChar.y + yi) * textFontBitmap->width + (textChar.x + xi)];

			Pixel32 p = _pixels[aty * _framebuffer->width + atx];
			p.rgba.r = Blend255(p.rgba.r, color.rgba.r, font.rgba.r);
			p.rgba.g = Blend255(p.rgba.g, color.rgba.g, font.rgba.g);
			p.rgba.b = Blend255(p.rgba.b, color.rgba.b, font.rgba.b);
			_pixels[aty * _framebuffer->width + atx] = p;
		}

	return textChar.width;
}

double drawAlphaChar(const char8_t c, double x, int16_t y, Pixel32 color) {
	TextFontList textChar = textFontList[(uint64_t)c];

	int16_t xx = (int16_t)(x + 0.5);
	uint8_t w = (uint8_t)(textChar.width + 0.5);

	for (uint8_t yi = 0; yi < 12; yi++)
		for (uint8_t xi = 0; xi < w; xi++) {
			const int16_t atx = (int16_t)xx + (int16_t)xi;
			const int16_t aty = (int16_t)y + (int16_t)yi;

			if (atx >= _framebuffer->width)
				continue;
			if (atx < 0)
				continue;

			if (aty >= _framebuffer->height)
				continue;
			if (aty < 0)
				continue;

			Pixel32 font =
				textFontBitmap->pixels[(textChar.y + yi) * textFontBitmap->width + (textChar.x + xi)];

			Pixel32 p = _pixels[aty * _framebuffer->width + atx];
			p.rgba.r = Blend255(p.rgba.r, color.rgba.r, font.rgba.r);
			p.rgba.g = Blend255(p.rgba.g, color.rgba.g, font.rgba.g);
			p.rgba.b = Blend255(p.rgba.b, color.rgba.b, font.rgba.b);

			// Grayscale
			// TODO probably just blend color.rgb all with p.g?
			uint16_t sum = (uint16_t)p.rgba.r + (uint16_t)p.rgba.g + (uint16_t)p.rgba.b;
			sum = sum * 0.33;

			p.rgba.r = sum;
			p.rgba.g = sum;
			p.rgba.b = sum;

			p.rgba.a = color.rgba.a;
			blendPixel(atx, aty, p);
		}

	return textChar.width;
}

/// Returns advance after last character
uint16_t drawAsciiText(const char8_t *text, double x, int16_t y, Pixel32 color) {
	uint16_t i = 0;
	double xx = x;
	while (text[i] != 0 && i < 255 * 255) {
		xx += drawChar(text[i], xx, y, color);
		i++;
	}
	return xx - x;
}

uint16_t drawIntegerText(int64_t value, double x, int16_t y, Pixel32 color) {
	double xx = x;
	if (value < 0) {
		xx += drawAsciiText(u8"-", xx, y, color);
		value = -value;
	}
	const char8_t *digits[] = {u8"0", u8"1", u8"2", u8"3", u8"4", u8"5", u8"6", u8"7", u8"8", u8"9"};
	const char8_t *chars[24] = {0};
	uint8_t count = 0;
	do {
		chars[23 - count] = digits[value % 10];
		value = (int64_t)(value / 10);
		count++;
	} while (value > 0);
	for (uint8_t i = 0; i < 24; ++i) {
		if (chars[i] != 0)
			xx += drawAsciiText(chars[i], xx, y, color);
	}
	return xx - x;
}

function drawShadowText(const char8_t *text, double x, int16_t y) {
	uint16_t i = 0;
	double xx = x;
	Pixel32 color;
	color.color = 0x00000000;
	color.rgba.a = 70;
	while (text[i] != 0 && i < 255 * 255) {
		char8_t ch = text[i];

		drawAlphaChar(ch, xx + 1, y + 1, color);
		drawAlphaChar(ch, xx + 1, y + 2, color);

		drawAlphaChar(ch, xx - 1, y + 1, color);
		xx += drawAlphaChar(ch, xx - 1, y + 2, color);

		i++;
	}
}
