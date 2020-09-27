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

// Desktop (i.e. icons)

extern "C++" namespace desktop {
	function compositeDesktop(int16_t mouseX, int16_t mouseY) {
		Pixel32 color;
		color.rgba.r = color.rgba.g = color.rgba.b = 0xFF;
		color.rgba.a = 64;

		// Keep origin left-top
		var outlineX = mouseX < dragX ? mouseX : dragX;
		var outlineY = mouseY < dragY ? mouseY : dragY;
		var outlineW = mouseX < dragX ? dragX - mouseX : mouseX - dragX;
		var outlineH = mouseY < dragY ? dragY - mouseY : mouseY - dragY;

		if (drag) {
			drawRectangleWithAlpha(color, outlineX, outlineY, outlineW, outlineH);
			drawRectangleOutline(color, outlineX, outlineY, outlineW, outlineH);
		}
	}

	function handleMouseDownDesktop(MouseActionType type, int16_t mouseX, int16_t mouseY) {
		if (drag == false) {
			dragX = mouseX;
			dragY = mouseY;
		}
		drag = true;
	}

	function handleMouseUpDesktop(MouseActionType type, int16_t mouseX, int16_t mouseY) {
		drag = false;
	}
}
