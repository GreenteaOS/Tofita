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

// Supports only 24-bit .bmp

namespace bmp {

Bitmap32 *loadBmp24(const RamDiskAsset *asset) {
	uint8_t *buffer = asset->data;
	uint8_t info[54];

	for (uint8_t i = 0; i < 54; i++) {
		info[i] = buffer[i];
	}

	// TODO optimization: just load whole bmp, validate, fix, and just use as-is without reallocs
	uint16_t width = *(uint32_t *)&info[18] & 0xFFFF;
	uint16_t height = *(uint32_t *)&info[22] & 0xFFFF;

	Bitmap32 *result = allocateBitmap(width, height);

	serialPrintf(L"[bmp] The bitmap image is %dx%d at %d\n", width, height, result);

	uint8_t *colors = (uint8_t *)(buffer + 54);
	for (uint16_t y = 0; y < height; y++)
		for (uint16_t x = 0; x < width; x++) {
			result->pixels[y * width + x].rgba.a = 255; // transparency
			result->pixels[y * width + x].rgba.r = colors[((height - y - 1) * width + x) * 3 + 0];
			result->pixels[y * width + x].rgba.g = colors[((height - y - 1) * width + x) * 3 + 1];
			result->pixels[y * width + x].rgba.b = colors[((height - y - 1) * width + x) * 3 + 2];
		}

	serialPrintf(L"[bmp] Pixels filled\n");

	return result;
}
} // namespace bmp
