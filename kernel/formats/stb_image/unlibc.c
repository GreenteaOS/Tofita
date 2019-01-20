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

#undef free libc_free
#undef malloc libc_malloc
#undef realloc libc_realloc
#undef memset libc_memset
#undef abs libc_abs
#undef pow libc_pow
#undef memcpy libc_memcpy

Bitmap32* loadPng32(const RamDiskAsset* asset) {
	int width, height, png_bpp;
	uint8_t* colors = stbi_load_from_memory(asset->data, asset->size, &width, &height, &png_bpp, 4);
	Bitmap32* bitmap = allocateBitmapFromBuffer(width, height);

	uint32_t* rgba = colors;
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
