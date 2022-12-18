// The Tofita Kernel
// Copyright (C) 2020 Oleh Petrenko
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

#undef free
#undef malloc
#undef realloc
#undef memset
#undef abs
#undef pow
#undef memcpy

#if 0
Bitmap32 *loadPng32FromBuffer(uint8_t *data, uint32_t size) {
	int32_t width, height, png_bpp;
	uint8_t *colors = stbi_load_from_memory(data, size, &width, &height, &png_bpp, 4);
	Bitmap32 *bitmap = allocateBitmap(width, height);

	uint32_t *rgba = (uint32_t *)colors;
	for (uint16_t y = 0; y < height; y++)
		for (uint16_t x = 0; x < width; x++) {
			bitmap->pixels[y * width + x].color = rgba[y * width + x];
			// This should be fixed in framebuffer? Or PNG really BGRA?
			uint8_t temp = bitmap->pixels[y * width + x].rgba.r;
			bitmap->pixels[y * width + x].rgba.r = bitmap->pixels[y * width + x].rgba.b;
			bitmap->pixels[y * width + x].rgba.b = temp;
		}

	return bitmap;
}

Bitmap32 *loadPng32(const RamDiskAsset *asset) {
	return loadPng32FromBuffer(asset->data, asset->size);
}
#endif
