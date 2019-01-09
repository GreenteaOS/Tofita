static Framebuffer *_framebuffer;

void putPixel(unsigned x, unsigned y, uint32_t px)
{
	int32_t *fbBase = (int32_t *) _framebuffer->base;
	int32_t *pixelAddress = fbBase + y * _framebuffer->pixelsPerScanline + x;
	*pixelAddress = px;
}

void clearScreen() {
	union {
		uint8_t bytes[4];
		uint32_t int32_value;
	} encoder;

	for (unsigned x = 0; x < _framebuffer->width; ++x) {
		for (unsigned y = 0; y < _framebuffer->height; ++y) {
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
	clearScreen();
}

void safePutPixel(unsigned x, unsigned y, uint32_t px) {
	if (x < _framebuffer->width && y < _framebuffer->height) {
		putPixel(x, y, px);
	}
}
