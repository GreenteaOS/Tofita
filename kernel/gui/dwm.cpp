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

// Desktop window manager (i.e. GUI logic)

namespace dwm {

uint8_t nextDefaultWindowPosition = 0;

OverlappedWindow *OverlappedWindow_create(uint64_t pid) {
	uint64_t index = 0;
	while (index < 255) {
		index++;
		if (index == 256)
			return null;
		if (windowsList[index].present != true)
			break;
	}

	auto window = &windowsList[index];
	window->windowId = index;
	window->pid = pid;
	window->present = true;
	window->visible = false;
	window->width = 500;							  // TODO actual wapi default values
	window->height = 300;							  // TODO actual wapi default values
	window->x = 200 + 48 * nextDefaultWindowPosition; // TODO actual wapi default values
	window->y = 100 + 48 * nextDefaultWindowPosition; // TODO actual wapi default values
	nextDefaultWindowPosition++;
	if (nextDefaultWindowPosition > 8)
		nextDefaultWindowPosition = 0;
	window->fbZeta = null;
	window->fbGama = null;
	window->fbCurrentZeta = true;
	window->title = null;
	return window;
}

OverlappedWindow *OverlappedWindow_find(uint64_t pid, uint64_t windowId) {
	uint64_t index = 0;
	while (index < 255) {
		index++;
		if (index == 256)
			return null;
		auto window = &windowsList[index];
		if (window->present == true && window->pid == pid && window->windowId == windowId)
			return window;
	}
	return null;
}
} // namespace dwm
