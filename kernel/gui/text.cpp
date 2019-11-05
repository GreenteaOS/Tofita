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

// Simple bitmap font drawing
// Non-monospace

Bitmap32* textFontBitmap;

typedef struct {
	uint16_t x;
	uint16_t y;
	double width;
} __attribute__((packed)) TextFontList;

TextFontList* textFontList;

void initText() {
	RamDiskAsset textFont = getRamDiskAsset("ascii.tofita");
	textFontList = (TextFontList*)textFont.data;

	Bitmap32* loadPng32(const RamDiskAsset* asset);
	RamDiskAsset a = getRamDiskAsset("fonts\\default.png");
	textFontBitmap = loadPng32(&a);
}

int8_t getLigatureAdvance(const char left, const char right) {
	if (left == 0) return 0;
	if (right == 0) return 0;
	return 8;
}

double drawChar(const char c, double x, uint16_t y) {
	TextFontList textChar = textFontList[(size_t)c];

	uint16_t xx = (int)(x + 0.5);
	uint8_t w = (int)(textChar.width + 0.5);

	for (uint8_t yi = 0; yi < 12; yi++)
		for (uint8_t xi = 0; xi < w; xi++) {
			Pixel32 color = textFontBitmap->pixels[
				(textChar.y + yi) * textFontBitmap->width + (textChar.x + xi)
			];

			setPixel(xx + xi, y + yi, color);
		}

	return textChar.width;
}

/// Returns advance after last character
uint16_t drawAsciiText(const char* text, double x, uint16_t y) {
	uint16_t i = 0;
	double xx = x;
	while (text[i] != 0 && i < 255*255) {
		xx += drawChar(text[i], xx, y);
		i++;
	}
	return xx - x;
}
