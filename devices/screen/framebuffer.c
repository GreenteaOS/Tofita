Framebuffer *_framebuffer;

typedef struct {
	uint8_t r;
	uint8_t g;
	uint8_t b;
	uint8_t a;
} __attribute__((packed)) PixelRGBAData;

typedef struct {
	union {
		PixelRGBAData rgba;
		uint32_t color;
	};
} Pixel32;

Pixel32 *_pixels;

void putPixel(uint16_t x, uint16_t y, uint32_t px)
{
	int32_t *fbBase = (int32_t *) _framebuffer->base;
	int32_t *pixelAddress = fbBase + y * _framebuffer->width + x;
	*pixelAddress = px;
}

void clearScreen() {
	union {
		uint8_t bytes[4];
		uint32_t int32_value;
	} encoder;

	for (uint16_t y = 0; y < _framebuffer->height; ++y) {
		for (uint16_t x = 0; x < _framebuffer->width; ++x) {
			encoder.bytes[0] = x / 3;
			encoder.bytes[1] = y / 3;
			encoder.bytes[2] = x+y;
			encoder.bytes[3] = 0xFF;
			putPixel(x, y, encoder.int32_value);

			if (x < 5) putPixel(x, y, 0xA0A0A0FF);
			if (x > 795) putPixel(x, y, 0xA0A0A0FF);
			if (y > 595) putPixel(x, y, 0xA0A0A0FF);
			if (y < 5) putPixel(x, y, 0xA0A0A0FF);
		}
	}
}

void setFramebuffer(Framebuffer *framebuffer) {
	_framebuffer = framebuffer;
	_pixels = (Pixel32 *)_framebuffer->base;
	clearScreen();
}

void safePutPixel(uint16_t x, uint16_t y, uint32_t px) {
	if (x < _framebuffer->width && y < _framebuffer->height) {
		putPixel(x, y, px);
	}
}

typedef struct {
	uint16_t width;
	uint16_t height;
	Pixel32 pixels[];
} Bitmap32;

// Very fast, but not precise, alpha multiply
#define Mul255(a255, c255) (((uint32_t)a255 + 1) * (uint32_t)c255 >> 8)

void blendPixel(uint16_t x, uint16_t y, Pixel32 pixel) {
	if ((x > _framebuffer->width - 1) || (y > _framebuffer->height - 1)) return ;
	Pixel32 p = _pixels[y * _framebuffer->width + x];

	p.rgba.r = Mul255(pixel.rgba.a, pixel.rgba.r) + Mul255(255 - pixel.rgba.a, p.rgba.r);
	p.rgba.g = Mul255(pixel.rgba.a, pixel.rgba.g) + Mul255(255 - pixel.rgba.a, p.rgba.g);
	p.rgba.b = Mul255(pixel.rgba.a, pixel.rgba.b) + Mul255(255 - pixel.rgba.a, p.rgba.b);

	_pixels[y * _framebuffer->width + x] = p;
}

void setPixel(uint16_t x, uint16_t y, Pixel32 pixel) {
	if ((x > _framebuffer->width - 1) || (y > _framebuffer->height - 1)) return ;
	_pixels[y * _framebuffer->width + x] = pixel;
}

void drawBitmap32WithAlpha(Bitmap32* bitmap, uint16_t x, uint16_t y) {
	for (int yy = 0; yy < bitmap->height; yy++) {
		for (int xx = 0; xx < bitmap->width; xx++) {
			blendPixel(x + xx, y + yy, bitmap->pixels[y * bitmap->width]);
		}
	}
}

void drawRectangleWithAlpha(Pixel32 color, uint16_t x, uint16_t y, uint16_t width, uint16_t height) {
	for (int yy = 0; yy < height; yy++) {
		for (int xx = 0; xx < width; xx++) {
			blendPixel(x + xx, y + yy, color);
		}
	}
}

void drawRectangle(Pixel32 color, uint16_t x, uint16_t y, uint16_t width, uint16_t height) {
	for (int yy = 0; yy < height; yy++) {
		for (int xx = 0; xx < width; xx++) {
			setPixel(x + xx, y + yy, color);
		}
	}
}
