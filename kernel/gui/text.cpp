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

Bitmap32* textFontBitmap;
RamDiskAsset textFontList;

void initText() {

}

int8_t getLigatureAdvance(const char left, const char right) {
	if (left == 0) return 0;
	if (right == 0) return 0;
	return 8;
}

uint8_t drawChar(const char c, uint16_t x, uint16_t y) {
	return 8;
}

/// Returns advance after last character
uint16_t drawAsciiText(const char* text, uint16_t x, uint16_t y) {
	uint16_t i = 0;
	uint16_t xx = x;
	while (text[i] != 0 && i < 255*255) {
		xx += drawChar(text[i], xx, y);
		i++;
	}
	return xx - x;
}
