uint8_t mouseCycle = 0;
int8_t mouseByte[3];
int8_t mouseX = 0;
int8_t mouseY = 0;
uint16_t mxx = 256;
uint16_t myy = 256;

void handleMouse() {
	switch(mouseCycle) {
	case 0:
		mouseByte[0] = mouseRead(0x60);
		if((mouseByte[0] & 0x08) != 0) {
			mouseCycle++; // Only accept this as the first byte if the "must be 1" bit is set
		}
		//mouseCycle++;
		break;
	case 1:
		mouseByte[1] = mouseRead(0x60);
		mouseCycle++;
		break;
	case 2:
		mouseByte[2] = mouseRead(0x60);
		mouseX = mouseByte[1];
		mouseY = mouseByte[2];
		mouseCycle = 0;

		mxx += mouseX;
		myy -= mouseY;

		Pixel32 color;
		color.color = 0x44FFFFFF;
		drawRectangle(color, mxx, myy, 20, 40);
		color.color = 0x44FF33FF;
		drawRectangleWithAlpha(color, mxx + 30, myy, 20, 40);
		break;
	}

	writePort(0xA0, 0x20);
	writePort(0x20, 0x20);
}
