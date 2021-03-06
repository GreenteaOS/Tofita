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
	var dragX = 255;
	var dragY = 255;
	var drag = false;

	function compositeDesktop(int16_t mouseX, int16_t mouseY);
	function handleMouseDownDesktop(MouseActionType type, int16_t mouseX, int16_t mouseY);
	function handleMouseUpDesktop(MouseActionType type, int16_t mouseX, int16_t mouseY);
}
