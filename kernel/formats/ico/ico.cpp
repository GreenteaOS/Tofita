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

namespace icon {

Bitmap32 *loadIcon(const RamDiskAsset *asset, uint8_t widthToExtract) {
	uint8_t *buffer = asset->data;

	// Reserved. Must always be 0.
	if (buffer[0] != 0)
		return 0;
	// Specifies image type: 1 for icon (.ICO) image, 2 for cursor (.CUR) image.
	// Other values are invalid.
	if (buffer[2] != 1)
		return 0;
	// Specifies number of images in the file.
	uint16_t count = buffer[4] + buffer[5] * 0xFF;
	if (count == 0)
		return 0;

	// Search for the largest one with best quality
	uint32_t best = 6; // Offset
	uint32_t bestId = 0;
	uint32_t offset = 6;
	if (widthToExtract == 256)
		widthToExtract = 0; // Fix search
	for (uint16_t i = 0; i < count; i++) {
		// Specifies image width in pixels. Can be any number between 0 and 255. Value 0 means image width is
		// 256 pixels. Specifies image height in pixels. Can be any number between 0 and 255. Value 0 means
		// image height is 256 pixels. Specifies number of colors in the color palette. Should be 0 if the
		// image does not use a color palette.
		serialPrintf(u8"[ico] %d image is %dx%d, palette = %d, bits = %d\n", i, buffer[offset],
					 buffer[offset + 1], buffer[offset + 2], buffer[offset + 6] + buffer[offset + 7] * 0xFF);
		if (buffer[offset] == widthToExtract) {
			// TODO accept by bits per pixel == 32
			best = offset;
			bestId = i;
		}
		offset += 16;
		// Reserved. Should be 0.

		// In ICO format: Specifies color planes. Should be 0 or 1.[Notes 3]
		// In CUR format: Specifies the horizontal coordinates of the hotspot in number of pixels from the
		// left.

		// In ICO format: Specifies bits per pixel. [Notes 4]
		// In CUR format: Specifies the vertical coordinates of the hotspot in number of pixels from the top.

		// Specifies the size of the image's data in bytes
		// Specifies the offset of BMP or PNG data from the beginning of the ICO/CUR file
	}

	serialPrintf(u8"[ico] The best image is %dx%d\n", buffer[best], buffer[best + 1]);

	// Specifies image width in pixels. Can be any number between 0 and 255. Value 0 means image width is 256
	// pixels.
	var width = buffer[best];
	// Specifies image height in pixels. Can be any number between 0 and 255. Value 0 means image height is
	// 256 pixels.
	var height = buffer[best + 1];
	// Specifies number of colors in the color palette. Should be 0 if the image does not use a color palette.
	// Reserved. Should be 0.
	// In CUR format: Specifies the horizontal coordinates of the hotspot in number of pixels from the left.
	// In ICO format: Specifies color planes. Should be 0 or 1
	let planes = buffer[best + 4] + buffer[best + 5] * 0xFF;
	// In CUR format: Specifies the vertical coordinates of the hotspot in number of pixels from the top.
	// In ICO format: Specifies bits per pixel.
	let bits = buffer[best + 6] + buffer[best + 7] * 0xFF;
	// Specifies the size of the image's data in bytes
	uint32_t imageSize =
		cursor::toInt32(buffer[best + 8], buffer[best + 9], buffer[best + 10], buffer[best + 11]);
	// Specifies the offset of BMP or PNG data from the beginning of the ICO/CUR file
	offset = cursor::toInt32(buffer[best + 12], buffer[best + 13], buffer[best + 14], buffer[best + 15]);

	// TODO handle proper height
	if (width == 0)
		width = 256;
	if (height == 0)
		height = 256;
	if (height == width * 2) {
		height = width;
	}

	serialPrintf(u8"[ico] The icon is %dx%d, %d bits %d planes, %d bytes at %d\n", width, height, bits,
				 planes, imageSize, offset);
	// A PNG file starts with an 8-byte signature (refer to hex editor image on the right):
	serialPrintf(u8"[ico] Header at %d: %d %d %d %d\n", offset, buffer[offset], buffer[offset + 1],
				 buffer[offset + 2], buffer[offset + 3]);
	serialPrintf(u8"[ico] Header at %d %d %d %d\n", buffer[best + 12], buffer[best + 13], buffer[best + 14],
				 buffer[best + 15]);

	// ?PNG
	if (buffer[offset] == 0x89 && buffer[offset + 1] == 0x50 && buffer[offset + 2] == 0x4E &&
		buffer[offset + 3] == 0x47) {
		serialPrintf(u8"[ico] Decoding as PNG\n");
		return loadPng32FromBuffer((uint8_t *)&buffer[offset], imageSize);
	}

	uint32_t fsize = width * height * 4;
	let start = offset + 20 + 16 + 4;
	uint8_t *bmp = (uint8_t *)&buffer[start];

	Bitmap32 *result = allocateBitmap(width, height);

	for (uint16_t y = 0; y < height; y++)
		for (uint16_t x = 0; x < width; x++) {
			result->pixels[y * width + x].rgba.r = bmp[((height - y - 1) * width + x) * 4 + 0];
			result->pixels[y * width + x].rgba.g = bmp[((height - y - 1) * width + x) * 4 + 1];
			result->pixels[y * width + x].rgba.b = bmp[((height - y - 1) * width + x) * 4 + 2];
			result->pixels[y * width + x].rgba.a = bmp[((height - y - 1) * width + x) * 4 + 3];
		}

	return result;
}
} // namespace icon
