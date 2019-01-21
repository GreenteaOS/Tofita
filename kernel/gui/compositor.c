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

typedef enum {
	Center,
	Stretch,
	Fill
	// TODO more options
} WallpaperStyle;

void setWallpaper(Bitmap32* bitmap, WallpaperStyle style) {
	wallpaper = bitmap;
}

void composite() {
	drawBitmap32(wallpaper, 0, 0);
}
