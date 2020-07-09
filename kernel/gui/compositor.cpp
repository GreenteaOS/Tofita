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

// Performs screen composition of all user interface

// Note: ACCURACY is PREFERRED over SPEED, that's why we do full 2D composition even without GPU

// Differential composition is not yet supported, performance with large amount of apps may be low

Bitmap32 *wallpaper;	 // Size of framebuffer
Bitmap32 *vibranceLight; // Size of framebuffer
Bitmap32 *vibranceDark;	 // Size of framebuffer
Bitmap32 *leaves;
Bitmap32 *trashCan;
Bitmap32 *notepad16;
Bitmap32 *notepad48;
Bitmap32 *link;
cursor::Cursor *cur = null;

uint16_t startupAnimation = 0;

typedef enum {
	Center,
	Stretch,
	Fill
	// TODO more options
} WallpaperStyle;

function setWallpaper(Bitmap32 *bitmap, WallpaperStyle style) {

	{
		serialPrintln(u8"[compositor.setWallpaper] upscale wallpaper to screen size");
		Bitmap32 *upscale = allocateBitmap(_framebuffer->width, _framebuffer->height);

		float hReciprocal = 1.0f / (float)_framebuffer->height;
		float wReciprocal = 1.0f / (float)_framebuffer->width;

		for (uint16_t y = 0; y < upscale->height; y++)
			for (uint16_t x = 0; x < upscale->width; x++) {
				PixelRGBAData rgba = interpolatePixel(bitmap, ((float)x * wReciprocal) * bitmap->width,
													  ((float)y * hReciprocal) * (bitmap->height - 8));
				upscale->pixels[y * upscale->width + x].rgba = rgba;
			}

		wallpaper = upscale;
		bitmap = wallpaper;
	}

	serialPrintln(u8"[compositor.setWallpaper] downscale 8x");

	uint8_t blurScale = 8;

	Bitmap32 *downscale = allocateBitmap(bitmap->width / blurScale - 1, bitmap->height / blurScale - 1 + 8);

	for (uint32_t y = 0; y < downscale->height - 8; y++)
		for (uint32_t x = 0; x < downscale->width; x++)
			downscale->pixels[y * downscale->width + x].color =
				bitmap->pixels[y * bitmap->width * blurScale + x * blurScale].color;

	// Fix
	for (uint32_t y = downscale->height - 8; y < downscale->height; y++)
		for (uint32_t x = 0; x < downscale->width; x++)
			downscale->pixels[y * downscale->width + x].color =
				bitmap->pixels[bitmap->width * (bitmap->height - 1) + x * blurScale].color;

	serialPrintln(u8"[compositor.setWallpaper] blur");

	Bitmap32 *blur = gaussBlur(downscale, 4);

	serialPrintln(u8"[compositor.setWallpaper] upscale 8x");

	// Upscale
	Bitmap32 *upscaleLight = allocateBitmap(bitmap->width, bitmap->height);
	Bitmap32 *upscaleDark = allocateBitmap(bitmap->width, bitmap->height);
	Bitmap32 *upscale = upscaleDark;

	float hReciprocal = 1.0f / (float)upscale->height;
	float wReciprocal = 1.0f / (float)upscale->width;

	for (uint16_t y = 0; y < upscale->height; y++)
		for (uint16_t x = 0; x < upscale->width; x++) {
			PixelRGBAData rgbaSource =
				interpolatePixel(downscale, ((float)x * wReciprocal) * downscale->width,
								 ((float)y * hReciprocal) * (downscale->height - 8));
			// Apply vibrance (frosted glass)
			// 0.66*255 = 168.3

			{
				PixelRGBAData rgba = rgbaSource;
				rgba.r = Blend255(rgba.r, 0, 168);
				rgba.g = Blend255(rgba.g, 0, 168);
				rgba.b = Blend255(rgba.b, 0, 168);
				upscaleDark->pixels[y * upscale->width + x].rgba = rgba;
			}

			{
				PixelRGBAData rgba = rgbaSource;
				rgba.r = Blend255(rgba.r, 255, 168);
				rgba.g = Blend255(rgba.g, 255, 168);
				rgba.b = Blend255(rgba.b, 255, 168);
				upscaleLight->pixels[y * upscale->width + x].rgba = rgba;
			}
		}

	vibranceDark = upscaleDark;
	vibranceLight = upscaleLight;
}

function drawVibrancedRectangle(int16_t x, int16_t y, uint16_t width, uint16_t height, bool dark) {
	let vibrance = dark ? vibranceDark : vibranceLight; // Avoit non-register global access
	for (int16_t yy = 0; yy < height; yy++) {
		for (int16_t xx = 0; xx < width; xx++) {
			if (x + xx < 0)
				continue;
			if (x + xx > vibrance->width)
				continue;
			if (y + yy < 0)
				continue;
			if (y + yy > vibrance->height)
				continue;
			uint32_t pixel = (y + yy) * vibrance->width + xx + x;
			setPixel(x + xx, y + yy, vibrance->pixels[pixel]);
		}
	}
}

Bitmap32 *doublebuffer;
function initializeCompositor() {
	serialPrintln(u8"[compositor.initializeCompositor] begin");
	doublebuffer = allocateBitmap(_framebuffer->width, _framebuffer->height);
	_pixels = doublebuffer->pixels;

	Bitmap32 *loadPng32(const RamDiskAsset *asset);

	let a = getRamDiskAsset(u8"root/Windows/Resources/Icons/leaves.png");
	leaves = loadPng32(&a);

	let b = getRamDiskAsset(u8"root/Windows/Resources/Icons/trash-empty48.png");
	trashCan = loadPng32(&b);

	let c = getRamDiskAsset(u8"root/Windows/Resources/Icons/notepad.ico_48x48.png");
	notepad48 = loadPng32(&c);

	let d = getRamDiskAsset(u8"root/Windows/Resources/Icons/notepad.ico_16x16.png");
	notepad16 = loadPng32(&d);

	let e = getRamDiskAsset(u8"root/Windows/Resources/Icons/link.png");
	link = loadPng32(&e);

	let asset = getRamDiskAsset(u8"root/Windows/Cursors/aero_arrow.cur");
	cur = cursor::loadCursor(&asset);

	serialPrintln(u8"[compositor.initializeCompositor] done");
}

var dragX = 255;
var dragY = 255;
var drag = false;

function handleMouseDown(uint8_t key) {
	if (drag == false) {
		dragX = mouseX;
		dragY = mouseY;
	}
	drag = true;
}

function handleMouseUp(uint8_t key) {
	drag = false;
}

function composite() {
	var _framebuffer = ::_framebuffer; // Faster access

	// Startup animation
	if (startupAnimation < 2000) {
		haveToRender = true;
		startupAnimation += 9 * 2;
	}

	drawBitmap32(wallpaper, 0, 0);
	drawBitmap32WithAlpha(trashCan, 12, 10);
	Pixel32 color;
	color.color = 0x00000000;
	drawShadowText(u8"Recycle Bin", 8, 62);
	color.rgba.r = color.rgba.g = color.rgba.b = 0xFF;
	drawAsciiText(u8"Recycle Bin", 8, 62, color);

	var outlineX = mouseX < dragX ? mouseX : dragX;
	var outlineY = mouseY < dragY ? mouseY : dragY;
	var outlineW = mouseX < dragX ? dragX - mouseX : mouseX - dragX;
	var outlineH = mouseY < dragY ? dragY - mouseY : mouseY - dragY;
	color.rgba.a = 64;
	if ((mouseX > 20 && mouseX < (20 + 32) && mouseY > 12 && mouseY < (12 + 60))) {
		let outlineX = 1;
		let outlineY = 10;
		let outlineW = 72;
		let outlineH = 64;
		drawRectangleWithAlpha(color, outlineX, outlineY, outlineW, outlineH);
		drawRectangleOutline(color, outlineX, outlineY, outlineW, outlineH);
	};
	if (drag) {
		drawRectangleWithAlpha(color, outlineX, outlineY, outlineW, outlineH);
		drawRectangleOutline(color, outlineX, outlineY, outlineW, outlineH);
	}

	// Taskbar
	let animationTaskbarY = Math::min(startupAnimation / 1555.0, 1.0) * 30;
	drawVibrancedRectangle(0, _framebuffer->height - animationTaskbarY, _framebuffer->width, 30);

	let taskbarY = _framebuffer->height + 30 - animationTaskbarY * 2.0;

	// Start button
	color.rgba.a = 128;
	color.rgba.a = 100;
	color.rgba.r = color.rgba.g = color.rgba.b = 0xFF;
	color.rgba.r = color.rgba.g = color.rgba.b = 0x61;
	if (mouseX < 40 && mouseY > (_framebuffer->height - 30))
		drawRectangleWithAlpha(color, 0, taskbarY, 40, 30);
	drawBitmap32WithAlpha(leaves, 2, taskbarY + 2);

	// Taskbar shortcuts
	let shortcutsStart = 5 + 36 + 4;
	drawBitmap32WithAlpha(notepad16, shortcutsStart, taskbarY + 7);

	// Tray | line
	color.color = 0x00000000;
	color.rgba.r = color.rgba.g = color.rgba.b = 0x66;
	drawRectangle(color, _framebuffer->width - 4, taskbarY, 1, 30);

	var trayButtonX = _framebuffer->width - 20 - 16;
	trayButtonX = _framebuffer->width - 80;
	color.rgba.r = color.rgba.g = color.rgba.b = 0x11;
	color.rgba.r = color.rgba.g = color.rgba.b = 0xFF;

	let uptimeHours = (uint8_t)(uptimeMilliseconds / (60 * 60 * 1000));
	let uptimeMinutes = (uint8_t)((uptimeMilliseconds - (uptimeHours * (60 * 60 * 1000))) / (60 * 1000));

	var trayTimeX = trayButtonX + 20;
	trayTimeX += drawIntegerText(uptimeHours, trayTimeX, taskbarY + 10, color);
	trayTimeX += drawAsciiText(u8":", trayTimeX, taskbarY + 10, color);
	if (uptimeMinutes < 10)
		trayTimeX += drawAsciiText(u8"0", trayTimeX, taskbarY + 10, color);
	trayTimeX += drawIntegerText(uptimeMinutes, trayTimeX, taskbarY + 10, color);
	trayTimeX += drawAsciiText(u8" AM", trayTimeX, taskbarY + 10, color);

	line45smooth(color, trayButtonX, taskbarY + 10 + 2, 6, 1);
	line45smooth(color, trayButtonX + 1, taskbarY + 10 + 2, 6, -1);

	for (uint16_t i = 0; i < dwm::windowsLimit; ++i)
		if (dwm::windowsList[i].present) {
			let window = &dwm::windowsList[i];
			if (!window->visible)
				continue;

			drawWindowFrame(window->title ? window->title : L"Greentea OS", window->x, window->y,
							window->width, window->height);

			if (window->fbZeta != null && window->fbGama != null) {
				let frameHeight = 30;
				let frameWidth = 1;

				drawBitmap32(window->fbCurrentZeta ? window->fbZeta : window->fbGama, window->x + frameWidth,
							 window->y + frameHeight);
			}
		}

	drawCursor(cur, mouseX, mouseY);
	quake();
}

function copyToScreen() {
	// On 64-bit platform registers are 64-bit,
	// so lets copy two pixels at a time
	const uint64_t *source = (uint64_t *)_pixels;
	uint64_t *destination = (uint64_t *)_framebuffer->base;
	const uint32_t count = (_framebuffer->width * _framebuffer->height) / 2;

	for (uint32_t i = 0; i < count; i += 4) { // note +4
		// All screen resolutions are guaranteed to be divisible by 8 pixels
		destination[i + 0] = source[i + 0]; // each step is 2 pixels
		destination[i + 1] = source[i + 1]; // thus 4 * 2 = 8 pixels
		destination[i + 2] = source[i + 2]; // so CPU applies pipelining optimization
		destination[i + 3] = source[i + 3];
	}
}
