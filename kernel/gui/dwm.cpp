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
	while (index < windowsLimit - 1) {
		index++; // Null-window not used directly
		if (index == windowsLimit)
			return null;
		if (windowsList[index].present != true)
			break;
	}

	var window = &windowsList[index];
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
	window->hWnd = null;
	window->prevId = 0;
	window->nextId = 0;
	return window;
}

OverlappedWindow *OverlappedWindow_find(uint64_t pid, uint64_t windowId) {
	uint64_t index = 0;
	while (index < windowsLimit - 1) {
		index++;
		if (index == windowsLimit)
			return null;
		var window = &windowsList[index];
		if (window->present == true && window->pid == pid && window->windowId == windowId)
			return window;
	}
	return null;
}

OverlappedWindow *OverlappedWindow_findAnyProcessWindow(uint64_t pid) {
	uint64_t index = 0;
	while (index < windowsLimit - 1) {
		index++;
		if (index == windowsLimit)
			return null;
		var window = &windowsList[index];
		if (window->present == true && window->pid == pid)
			return window;
	}
	return null;
}

function OverlappedWindow_detach(uint64_t windowId) {
	var window = &windowsList[windowId];

	windowsList[window->prevId].nextId = window->nextId;
	windowsList[window->nextId].prevId = window->prevId;

	if (topmostWindow == windowId)
		topmostWindow = window->prevId;

	if (firstResponder == windowId) {
		firstResponder = window->prevId;

		while (windowsList[firstResponder].visible == false && firstResponder != 0) {
			firstResponder = windowsList[firstResponder].prevId;
		}
	}

	if (rootWindow == windowId)
		rootWindow = window->nextId;

	window->prevId = 0;
	window->nextId = 0;
}

function OverlappedWindow_attach(uint64_t windowId) {
	windowsList[topmostWindow].nextId = windowId;

	windowsList[windowId].prevId = topmostWindow;
	windowsList[windowId].nextId = 0;
	topmostWindow = windowId;

	if (windowsList[windowId].prevId == 0)
		rootWindow = 0;

	if (windowsList[windowId].visible)
		firstResponder = windowId;
}

function OverlappedWindow_destroy(uint64_t windowId) {
	var window = &windowsList[windowId];
	OverlappedWindow_detach(windowId);

	// TODO deallocate
	window->present = false;
	window->visible = false;
	window->fbZeta = null;
	window->fbGama = null;
	window->title = null;
}

function initDwm() {
	mouseX = _framebuffer->width / 2;
	mouseY = _framebuffer->height / 2;
}

FrameHover getFrameButton(int16_t mouseX, int16_t mouseY, uint64_t windowId) {
	if (windowId == 0)
		return FrameHover::Noop;

	var window = &windowsList[windowId];
	const uint16_t frameButtonsWidth = 137;

	let y = window->y;
	let x = window->x + window->width - frameButtonsWidth;

	let frameHeight = 30 + 3;

	let xx = x + window->width;
	let yy = y + frameHeight;

	if (mouseX >= x && mouseX <= xx && mouseY >= y && mouseY <= yy) {
		const uint8_t diff = (mouseX - x) / 46;
		if (diff == 0)
			return FrameHover::Min;
		if (diff == 1)
			return FrameHover::Max;
		return FrameHover::Close;
	}

	return FrameHover::Noop;
}

function handleMouseMove(int16_t mouseX, int16_t mouseY) {
	var window = &windowsList[mouseDragWindow];

	if (mouseDragCapturedWindow && window->present && window->visible) {
		let diffX = mouseX - mouseDragLastX;
		let diffY = mouseY - mouseDragLastY;

		window->x += diffX;
		window->y += diffY;
		if (window->y < 0)
			window->y = 0;

		mouseDragLastX = mouseX;
		mouseDragLastY = mouseY;

		return;
	}

	if (!frameHoverWindowDown) {
		frameHoverWindow = findWindowUnderCursor(mouseX, mouseY);
		frameHoverState = getFrameButton(mouseX, mouseY, frameHoverWindow);
	}
}

uint64_t findWindowUnderCursor(int16_t mouseX, int16_t mouseY) {
	var windowId = topmostWindow;

	do {
		var window = &windowsList[windowId];

		// TODO depend on style
		let frameHeight = 30 + 3;
		let frameWidth = 1;

		let x = window->x - 1;
		let y = window->y - 1;

		let xx = x + window->width + frameWidth;
		let yy = y + window->height + frameHeight;

		if (mouseX >= x && mouseX <= xx && mouseY >= y && mouseY <= yy) {
			return windowId;
		}

		windowId = window->prevId;
	} while (windowId != 0);

	return 0;
}

function selectMouseNextResponder(int16_t mouseX, int16_t mouseY) {
	// TODO hexa unconditional recursion firstResponder = selectMouseNextResponder(mouseX, mouseY);
	firstResponder = findWindowUnderCursor(mouseX, mouseY);
}

bool isDraggableFrame(int16_t mouseX, int16_t mouseY, uint64_t windowId) {
	if (windowId == 0)
		return false;

	var window = &windowsList[windowId];

	let y = window->y;
	let x = window->x;

	let frameHeight = 30 + 3;
	const uint16_t frameButtonsWidth = 137;

	let xx = x + window->width - frameButtonsWidth;
	let yy = y + frameHeight;

	if (mouseX >= x && mouseX <= xx && mouseY >= y && mouseY <= yy) {
		return true;
	}

	return false;
}

function handleMouseDown(MouseActionType type, int16_t mouseX, int16_t mouseY) {
	selectMouseNextResponder(mouseX, mouseY);

	if (firstResponder != 0 && firstResponder != topmostWindow) {
		let id = firstResponder;
		dwm::OverlappedWindow_detach(id);
		dwm::OverlappedWindow_attach(id);
	}

	if (type == MouseActionType::LeftDown && isDraggableFrame(mouseX, mouseY, firstResponder)) {
		mouseDragCapturedWindow = true;
		mouseDragWindow = firstResponder;
		mouseDragLastX = mouseX;
		mouseDragLastY = mouseY;
	}

	if (type == MouseActionType::LeftDown && frameHoverState != FrameHover::Noop) {
		frameHoverWindowDown = true;
	}

	if (firstResponder == 0)
		desktop::handleMouseDownDesktop(type, mouseX, mouseY);
}

function handleMouseUp(MouseActionType type, int16_t mouseX, int16_t mouseY) {
	if (type == MouseActionType::LeftUp)
		mouseDragCapturedWindow = false;

	if (frameHoverWindowDown) {
		frameHoverWindow = findWindowUnderCursor(mouseX, mouseY);
		let hover = getFrameButton(mouseX, mouseY, frameHoverWindow);
		var window = &windowsList[frameHoverWindow];

		if (hover == frameHoverState && window->present) {
			if (frameHoverState == FrameHover::Min) {}
			if (frameHoverState == FrameHover::Max) {}
			if (frameHoverState == FrameHover::Close) {
				PostMessagePayload payload;

				payload.hWnd = window->hWnd;
				payload.msg = wapi::Message::WM_CLOSE;
				payload.wParam = null;
				payload.lParam = null;

				volatile process::Process *process = &process::processes[window->pid];
				process::postMessage(process, &payload);
			}
		}

		frameHoverState = hover;
	}

	frameHoverWindowDown = false;
	haveToRender = 1;
	desktop::handleMouseUpDesktop(type, mouseX, mouseY);
}

function handleMouseActivity() {
	pollPS2Devices();
	if (lockMouse) {
		return;
	}
	// Swap buffers
	mouseActionsUseZeta = !mouseActionsUseZeta;

	volatile MouseAction(*mouseActions)[255] = (!mouseActionsUseZeta) ? &mouseActionsZeta : &mouseActionsGama;

	const uint8_t mouseActionsAmount =
		(!mouseActionsUseZeta) ? mouseActionsZetaAmount : mouseActionsGamaAmount;

	var mouseXtemp = mouseX;
	var mouseYtemp = mouseY;

	for (uint8_t i = 0; i < mouseActionsAmount; ++i) {
		let type = mouseActions[0][i].type;

		if (type == MouseActionType::Moved) {

			mouseXtemp += (*mouseActions)[i].mouseXdiff;
			mouseYtemp += (*mouseActions)[i].mouseYdiff;

			if (mouseYtemp < 0)
				mouseYtemp = 0;
			if (mouseXtemp < 0)
				mouseXtemp = 0;

			if (mouseXtemp > _framebuffer->width)
				mouseXtemp = _framebuffer->width;
			if (mouseYtemp > _framebuffer->height)
				mouseYtemp = _framebuffer->height;

			handleMouseMove(mouseXtemp, mouseYtemp);
		} else if (type == MouseActionType::LeftUp || type == MouseActionType::RightUp ||
				   type == MouseActionType::MiddleUp) {
			handleMouseUp(type, mouseXtemp, mouseYtemp);
		} else if (type == MouseActionType::LeftDown || type == MouseActionType::RightDown ||
				   type == MouseActionType::MiddleDown) {
			handleMouseDown(type, mouseXtemp, mouseYtemp);
		}

		(*mouseActions)[i].type = MouseActionType::Noop;
	}

	if (!mouseActionsUseZeta)
		mouseActionsZetaAmount = 0;
	if (mouseActionsUseZeta)
		mouseActionsGamaAmount = 0;

	// Atomically update
	if (mouseX != mouseXtemp || mouseY != mouseYtemp)
		haveToRender = 1;

	mouseX = mouseXtemp;
	mouseY = mouseYtemp;
}

function handleKeyboardActivity() {
	// TODO
}
} // namespace dwm
