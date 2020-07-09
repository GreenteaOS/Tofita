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

// Performs visualization onto the screen

// Speed of rendering mostly depends on cache-locality
// Remember: top-down, left-to-right: for(y) for(x) {}, not other way!

Framebuffer *_framebuffer;

// Avoid one level of pointer indirection
Pixel32 *_pixels;

Bitmap32 *allocateBitmap(uint16_t width, uint16_t height) {
	var result = (Bitmap32Interim *)PhysicalAllocator::allocateBytes(sizeof(uint16_t) * 2 +
																	 sizeof(Pixel32) * width * height);
	result->width = width;
	result->height = height;
	return (Bitmap32 *)result;
}

function setFramebuffer(const Framebuffer *framebuffer) {
	_framebuffer = const_cast<Framebuffer *>(framebuffer);
	_pixels = (Pixel32 *)_framebuffer->base;
}

// Very fast, but not precise, alpha multiply
#define Mul255(a255, c255) (((uint32_t)a255 + 1) * (uint32_t)c255 >> 8)
#define Blend255(target, color, alpha) (Mul255(alpha, color) + Mul255(255 - alpha, target))

function __attribute__((fastcall)) blendPixel(int16_t x, int16_t y, Pixel32 pixel) {
	var _framebuffer = ::_framebuffer; // Faster access
	if ((x > _framebuffer->width - 1) || (y > _framebuffer->height - 1))
		return;
	Pixel32 p = _pixels[y * _framebuffer->width + x];

	p.rgba.r = Mul255(pixel.rgba.a, pixel.rgba.r) + Mul255(255 - pixel.rgba.a, p.rgba.r);
	p.rgba.g = Mul255(pixel.rgba.a, pixel.rgba.g) + Mul255(255 - pixel.rgba.a, p.rgba.g);
	p.rgba.b = Mul255(pixel.rgba.a, pixel.rgba.b) + Mul255(255 - pixel.rgba.a, p.rgba.b);

	_pixels[y * _framebuffer->width + x] = p;
}

function __attribute__((fastcall)) setPixel(uint16_t x, uint16_t y, Pixel32 pixel) {
	var _framebuffer = ::_framebuffer; // Faster access
	if ((x > _framebuffer->width - 1) || (y > _framebuffer->height - 1))
		return;
	_pixels[y * _framebuffer->width + x] = pixel;
}

function drawBitmap32WithAlpha(Bitmap32 *bitmap, uint16_t x, uint16_t y) {
	for (int16_t yy = 0; yy < bitmap->height; yy++) {
		for (int16_t xx = 0; xx < bitmap->width; xx++) {
			blendPixel(x + xx, y + yy, bitmap->pixels[yy * bitmap->width + xx]);
		}
	}
}

function drawBitmap32(Bitmap32 *bitmap, uint16_t x, uint16_t y) {
	for (int16_t yy = 0; yy < bitmap->height; yy++) {
		for (int16_t xx = 0; xx < bitmap->width; xx++) {
			setPixel(x + xx, y + yy, bitmap->pixels[yy * bitmap->width + xx]);
		}
	}
}

function drawRectangleWithAlpha(Pixel32 color, uint16_t x, uint16_t y, uint16_t width, uint16_t height) {
	for (int16_t yy = 0; yy < height; yy++) {
		for (int16_t xx = 0; xx < width; xx++) {
			blendPixel(x + xx, y + yy, color);
		}
	}
}

function drawRectangle(Pixel32 color, uint16_t x, uint16_t y, uint16_t width, uint16_t height) {
	for (int16_t yy = 0; yy < height; yy++) {
		for (int16_t xx = 0; xx < width; xx++) {
			setPixel(x + xx, y + yy, color);
		}
	}
}

function drawRectangleOutline(Pixel32 color, uint16_t x, uint16_t y, uint16_t width, uint16_t height) {
	for (int16_t yy = 0; yy < height; yy++) {
		for (int16_t xx = 0; xx < width; xx++) {
			// Rendering left and far right points sequentally should be
			// better for cache-locality than vertical lines
			// At least this is true for small rectangles (like buttons)
			if (yy == 0 || xx == 0 || xx == width - 1 || yy == height - 1)
				setPixel(x + xx, y + yy, color);
		}
	}
}

function line45smooth(Pixel32 color, int16_t x, int16_t y, int16_t width, int16_t mod) {
	color.rgba.a = 98;
	int16_t xx = 0;
	for (int16_t xi = 0; xi < width - 1; xi++) {
		xx += mod;
		setPixel(xx + x, y + xi, color);
		blendPixel(xx + x, y + xi + 1, color);
		blendPixel(xx + x + mod, y + xi, color);
	}
	xx += mod;
	setPixel(xx + x, y + width - 1, color);
}

function lineHorizontal(Pixel32 color, int16_t x, int16_t y, uint16_t rightLength) {
	for (uint16_t i = 0; i < rightLength; ++i) {
		setPixel(x + i, y, color);
	}
}

function lineHorizontalWithAlpha(Pixel32 color, int16_t x, int16_t y, uint16_t rightLength) {
	for (uint16_t i = 0; i < rightLength; ++i) {
		blendPixel(x + i, y, color);
	}
}

function lineVertical(Pixel32 color, int16_t x, int16_t y, uint16_t bottomLength) {
	for (uint16_t i = 0; i < bottomLength; ++i) {
		setPixel(x, y + i, color);
	}
}

function lineVerticalWithAlpha(Pixel32 color, int16_t x, int16_t y, uint16_t bottomLength) {
	for (uint16_t i = 0; i < bottomLength; ++i) {
		blendPixel(x, y + i, color);
	}
}
