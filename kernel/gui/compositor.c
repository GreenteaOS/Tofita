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

// Performs screen composition of all user interface

// Note: ACCURACY is PREFERRED over SPEED, that's why we do full 2D composition even without GPU

// Differential composition is not yet supported, performance with large amount of apps may be low

Bitmap32* wallpaper; // Size of framebuffer
Bitmap32* vibrance; // Size of framebuffer

typedef enum {
	Center,
	Stretch,
	Fill
	// TODO more options
} WallpaperStyle;

void setWallpaper(Bitmap32* bitmap, WallpaperStyle style) {
	wallpaper = bitmap;

	serialPrintln("[compositor.setWallpaper] downscale 8x");

	uint8_t blurScale = 8;

	Bitmap32* downscale = allocateBitmapFromBuffer(bitmap->width / blurScale - 1, bitmap->height / blurScale - 1 + 8);

	for (uint32_t y = 0; y < downscale->height - 8; y++)
		for (uint32_t x = 0; x < downscale->width; x++)
			downscale->pixels[y * downscale->width + x].color =
				bitmap->pixels[y * bitmap->width * blurScale + x * blurScale].color;

	// Fix
	for (uint32_t y = downscale->height - 8; y < downscale->height; y++)
		for (uint32_t x = 0; x < downscale->width; x++)
			downscale->pixels[y * downscale->width + x].color =
				bitmap->pixels[bitmap->width * (bitmap->height - 1) + x * blurScale].color;

	serialPrintln("[compositor.setWallpaper] blur");

	Bitmap32* blur = gaussBlur(downscale, 4);

	serialPrintln("[compositor.setWallpaper] upscale 8x");

	// Upscale
	Bitmap32* upscale = allocateBitmapFromBuffer(bitmap->width, bitmap->height);

	float hReciprocal = 1.0f / (float)upscale->height;
	float wReciprocal = 1.0f / (float)upscale->width;

	for (uint16_t y = 0; y < upscale->height; y++)
		for (uint16_t x = 0; x < upscale->width; x++) {
			PixelRGBAData rgba = interpolatePixel(downscale,
				((float)x * wReciprocal) * downscale->width,
				((float)y * hReciprocal) * (downscale->height - 8)
			);
			// Apply vibrance (frosted glass)
			// 0.66*255 = 168.3
			if (x < 512) {
				rgba.r = Blend255(rgba.r, 255, 168);
				rgba.g = Blend255(rgba.g, 255, 168);
				rgba.b = Blend255(rgba.b, 255, 168);
			}
			upscale->pixels[y * upscale->width + x].rgba = rgba;
		}

	vibrance = upscale;
}

void drawVibrancedRectangle(int16_t x, int16_t y, uint16_t width, uint16_t height) {
	for (int16_t yy = 0; yy < height; yy++) {
		for (int16_t xx = 0; xx < width; xx++) {
			if (x + xx < 0) continue ;
			if (x + xx > vibrance->width) continue ;
			if (y + yy < 0) continue ;
			if (y + yy > vibrance->height) continue ;
			uint32_t pixel = (y + yy) * vibrance->width + xx + x;
			setPixel(x + xx, y + yy, vibrance->pixels[pixel]);
		}
	}
}

void composite() {
	drawBitmap32(wallpaper, 0, 0);
}
