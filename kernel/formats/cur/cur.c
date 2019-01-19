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

struct Cursor {
	uint16_t x;
	uint16_t y;
	uint8_t width;
	uint8_t height;
	uint8_t* bmp;
} Cursor;

uint32_t toInt32(char a, char b, char c, char d) {
	return a + b * 256 + c * 0xFFFF + d * 0xFFFFFF;
}

struct Cursor *loadCursor(const RamDiskAsset* asset) {
	uint8_t* buffer = asset->data;

	// Reserved. Must always be 0.
	if (buffer[0] != 0) return 0;
	// Specifies image type: 1 for icon (.ICO) image, 2 for cursor (.CUR) image.
	// Other values are invalid.
	if (buffer[2] != 2) return 0;
	// Specifies number of images in the file.
	uint16_t count = buffer[4] + buffer[5] * 0xFF;
	if (count == 0) return 0;

	// Search for the largest one with best quality
	uint32_t best = 6; // Offset

	uint32_t offset = 6;
	for (uint16_t i = 0; i < count; i++) {
		// Specifies image width in pixels. Can be any number between 0 and 255. Value 0 means image width is 256 pixels.
		// Specifies image height in pixels. Can be any number between 0 and 255. Value 0 means image height is 256 pixels.
		// Specifies number of colors in the color palette. Should be 0 if the image does not use a color palette.
		serialPrintf("[cur] %d image is %dx%d, palette = %d\r\n", i, buffer[offset], buffer[offset + 1], buffer[offset + 2]);
		if (buffer[offset] + buffer[offset + 1] > buffer[best] + buffer[best + 1]) best = offset;
		offset += 16;
		// Reserved. Should be 0.

		// In ICO format: Specifies color planes. Should be 0 or 1.[Notes 3]
		// In CUR format: Specifies the horizontal coordinates of the hotspot in number of pixels from the left.

		// In ICO format: Specifies bits per pixel. [Notes 4]
		// In CUR format: Specifies the vertical coordinates of the hotspot in number of pixels from the top.

		// Specifies the size of the image's data in bytes
		// Specifies the offset of BMP or PNG data from the beginning of the ICO/CUR file
	}

	serialPrintf("[cur] The best image is %dx%d\r\n", buffer[best], buffer[best + 1]);

	struct Cursor *cur = allocateFromBuffer(sizeof(struct Cursor));
	// Specifies image width in pixels. Can be any number between 0 and 255. Value 0 means image width is 256 pixels.
	cur->width = buffer[best];
	// Specifies image height in pixels. Can be any number between 0 and 255. Value 0 means image height is 256 pixels.
	cur->height = buffer[best + 1];
	// Specifies number of colors in the color palette. Should be 0 if the image does not use a color palette.
	// Reserved. Should be 0.
	// In CUR format: Specifies the horizontal coordinates of the hotspot in number of pixels from the left.
	cur->x = buffer[best + 4] + buffer[best + 5] * 0xFF;
	// In CUR format: Specifies the vertical coordinates of the hotspot in number of pixels from the top.
	cur->y = buffer[best + 6] + buffer[best + 7] * 0xFF;
	// Specifies the size of the image's data in bytes
	uint32_t imageSize = toInt32(buffer[best + 8], buffer[best + 9], buffer[best + 10], buffer[best + 11]);
	// Specifies the offset of BMP or PNG data from the beginning of the ICO/CUR file
	offset = toInt32(buffer[best + 12], buffer[best + 13], buffer[best + 14], buffer[best + 15]);

	serialPrintf("[cur] The cursor is %dx%d, %d:%d, %d bytes at %d\r\n", cur->width, cur->height, cur->x, cur->y, imageSize, offset);
	// A PNG file starts with an 8-byte signature (refer to hex editor image on the right):
	serialPrintf("[cur] Header at %d: %d %d %d %d\r\n", offset, buffer[offset], buffer[offset + 1], buffer[offset + 2], buffer[offset + 3]);
	serialPrintf("[cur] Header at %d %d %d %d\r\n", buffer[best + 12], buffer[best + 13], buffer[best + 14], buffer[best + 15]);

	uint32_t fsize = cur->width * cur->height * 4;
	uint8_t* bmp = allocateFromBuffer(fsize);
	for (int i = 0; i < fsize; i++) {
		bmp[i] = buffer[offset + i + 20 + 16 + 4];
	}
	cur->bmp = bmp;

	return cur;
}

void drawCursor(const struct Cursor *cur, uint16_t x, uint16_t y) {
	Pixel32 pixel;
	for (uint8_t yi = 0; yi < cur->height; yi++)
		for (uint8_t xi = 0; xi < cur->width; xi++) {
			uint32_t yy = cur->height - yi - 1;
			uint32_t* bmp = cur->bmp;
			pixel.color = bmp[xi + yy*cur->width];
			blendPixel(x+xi-cur->x+1, y+yi-cur->y+1, pixel);
		}
}
