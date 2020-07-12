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

// Zero for system GUI (like taskbar)
uint64_t firstResponder = 0; // Active window to catch keyboard input

struct OverlappedWindow {
	bool present;
	uint64_t pid; // Owner
	// TODO tid - thread id
	uint64_t windowId;
	uint64_t nextId;
	uint64_t prevId;
	wapi::HWnd hWnd;

	bool visible;
	const wchar_t *title;

	int16_t x;
	int16_t y;

	uint16_t width;
	uint16_t height;

	// fb.pixels == null if framebuffer not present
	// Note: fb size may be != current window size
	Bitmap32 *fbZeta;
	Bitmap32 *fbGama;
	bool fbCurrentZeta; // Use to swap chain
};

constexpr uint64_t windowsLimit = 256;

// TODO allocate dynamically (just use Hexa Array or something)
OverlappedWindow windowsList[windowsLimit];

uint64_t rootWindow = 0;
uint64_t topmostWindow = 0;

bool mouseDragCapturedWindow = false;
uint64_t mouseDragWindow = 0;
int16_t mouseDragLastX = 0;
int16_t mouseDragLastY = 0;

// Note: those are controlled by DWM
volatile int16_t mouseX = 256;
volatile int16_t mouseY = 256;

enum class FrameHover : uint8_t { Noop = 0, Min, Max, Close };

uint64_t frameHoverWindow = 0;
bool frameHoverWindowDown = false;
FrameHover frameHoverState = FrameHover::Noop;

OverlappedWindow *OverlappedWindow_create(uint64_t pid);
OverlappedWindow *OverlappedWindow_findAnyProcessWindow(uint64_t pid);
OverlappedWindow *OverlappedWindow_find(uint64_t pid, uint64_t windowId);
function OverlappedWindow_detach(uint64_t windowId);
function OverlappedWindow_attach(uint64_t windowId);
function OverlappedWindow_destroy(uint64_t windowId);
function initDwm();
function handleMouseActivity();
function handleKeyboardActivity();
uint64_t findWindowUnderCursor(int16_t mouseX, int16_t mouseY);
} // namespace dwm
