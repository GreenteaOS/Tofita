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

#define USER32_DLL __declspec(dllexport)
#include "user32.hpp"
#include "user32Vars.hpp"
#include "../kernel32.dll/kernel32.hpp"

// Utils

user32::Window *handleToWindow(wapi::HWnd hWnd) {
	if (hWnd == nullptr)
		return nullptr; // TODO return desktop
	if (hWnd == INVALID_HANDLE_VALUE)
		return nullptr;
	if (((user32::Window *)hWnd)->windowIsWindow != user32::windowIsWindow)
		return nullptr;
	return (user32::Window *)hWnd;
}

wapi::HWnd windowToHandle(user32::Window *window) {
	// TODO offset ptr 8-byte aligned + reserved space
	// os it fits into upper half of virtual space
	return (wapi::HWnd)window;
}

// WAPI
extern "C" {

wapi::Atom RegisterClassW(const wapi::WindowClass *wc) {
	tofitaDebugLog(u8"RegisterClassW called");

	// TODO use libc func utf16le
	auto len = 0;
	// TODO lpszClassName takes atoms?
	auto str = (const uint16_t *)wc->lpszClassName;
	for (uint16_t i = 0; i < 256; ++i) {
		// Increment upfront to rezerve for zeros
		len += 2;
		// Null-termination of utf16le is \0\0
		if (str[i] == 0)
			break;
	}

	auto tail = &user32::rootClass;
	while (tail->next != nullptr)
		tail = tail->next;

	auto list = (user32::ClassesLinkedList *)LocalAlloc(0, sizeof(user32::ClassesLinkedList));
	auto name = (uint16_t *)LocalAlloc(0, len);
	list->next = nullptr;

	for (uint16_t i = 0; i < len; ++i) {
		name[i] = str[i];
	}

	list->name = name;
	list->wc = *wc;

	tail->next = list;
	tail = tail->next;

	tofitaDebugLog(u8"RegisterClassW done");
	return (wapi::Atom)0;
}

wapi::HWnd CreateWindowExW(uint32_t dwExStyle, const wchar_t *lpClassName, const wchar_t *lpWindowName,
						   uint32_t dwStyle, int32_t x, int32_t y, int32_t nWidth, int32_t nHeight,
						   wapi::HWnd hWndParent, wapi::HMenu hMenu, wapi::HInstance hInstance,
						   void *lpParam) {
	tofitaDebugLog(u8"CreateWindowExW called");

	// Find class
	auto *list = &user32::rootClass;
	auto name = lpClassName;
	auto found = false;
	const wapi::WindowClass *wc = nullptr;

	while (list->next != nullptr) {
		// Step upfront, to ignore empty root
		list = list->next;

		uint16_t i = 0;
		while (true) {
			if ((list->name[i] == name[i]) && (name[i] == 0)) {
				tofitaDebugLog(u8"CreateWindowExW found class {%S}\n", (uint64_t)name);
				found = true;
				wc = &list->wc;
				break;
			}
			if (list->name[i] == name[i]) {
				i++;
				continue;
			}
			break;
		}
	}

	if (!found) {
		tofitaDebugLog(u8"CreateWindowExW not found return");
		return (wapi::HWnd) nullptr;
	}

	// Prepare for syscall
	CreateWindowExPayload payload;

	payload.wc = wc;

	payload.lpClassName = lpClassName;
	payload.lpWindowName = lpWindowName;
	payload.dwStyle = dwStyle;
	payload.x = x;
	payload.y = y;
	payload.nWidth = nWidth;
	payload.nHeight = nHeight;
	payload.hWndParent = hWndParent;
	payload.hMenu = hMenu;
	payload.hInstance = hInstance;
	payload.lpParam = lpParam;

	// Create local window counterpart
	auto window = (user32::Window *)LocalAlloc(0, sizeof(user32::Window));

	const auto hWnd = (wapi::HWnd)window;
	payload.hWnd = hWnd;

	// Create native window counterpart
	uint64_t windowId = tofitaCreateWindowEx(&payload);

	window->windowId = windowId;
	window->windowIsWindow = user32::windowIsWindow;
	window->proc = wc->lpfnWndProc;

	// Initial paint
	// TODO should be called somewhere else, in syscall of windows creation?
	PostMessage(hWnd, wapi::Message::WM_PAINT, nullptr, nullptr);

	tofitaDebugLog(u8"CreateWindowExW done");
	return hWnd;
}

wapi::Bool ShowWindow(wapi::HWnd hWnd, int32_t nCmdShow) {
	tofitaDebugLog(u8"ShowWindow called");
	auto window = handleToWindow(hWnd);
	wapi::Bool result = 0;
	if (window != nullptr) {
		switch (nCmdShow) {
		case SW_HIDE: {
			tofitaShowWindow(window->windowId, nCmdShow);
			result = 1;
			break;
		}
		case SW_SHOW:
		case SW_SHOWNORMAL:
		case SW_SHOWDEFAULT: {
			tofitaShowWindow(window->windowId, nCmdShow);
			result = 1;
			break;
		}
		default: {
			// TODO
		}
		}
	}
	tofitaDebugLog(u8"ShowWindow done");
	return result;
}

// TODO take 2 or more messages at a time for better perf (this is pretty common)
wapi::Bool GetMessageW(wapi::Msg *msg, wapi::HWnd hWnd, uint32_t wMsgFilterMin, uint32_t wMsgFilterMax) {
	tofitaDebugLog(u8"GetMessageW called");
	wapi::Bool result = 1;

	GetMessagePayload payload;
	payload.msg = msg;
	payload.hWnd = hWnd;
	payload.wMsgFilterMin = wMsgFilterMin;
	payload.wMsgFilterMax = wMsgFilterMax;
	tofitaGetMessage(&payload);

	// Stop iteration
	if (msg->message == wapi::Message::WM_QUIT)
		result = 0;

	tofitaDebugLog(u8"GetMessageW done");
	return result;
}

wapi::Bool TranslateMessage(wapi::Msg *msg) {
	tofitaDebugLog(u8"TranslateMessage called");
	// TODO
	tofitaDebugLog(u8"TranslateMessage done");
	return 0;
}

wapi::LResult DispatchMessageW(wapi::Msg *msg) {
	tofitaDebugLog(u8"DispatchMessageW called");
	wapi::LResult result = 0;
	if (msg->hwnd != nullptr) {
		auto window = handleToWindow(msg->hwnd);
		if (window == nullptr)
			return 0;
		result = window->proc(msg->hwnd, msg->message, msg->wParam, msg->lParam);
	} else {
		// TODO
	}
	tofitaDebugLog(u8"DispatchMessageW done");
	return result;
}

wapi::Bool PostMessage(wapi::HWnd hWnd, wapi::Message msg, void *wParam, void *lParam) {
	PostMessagePayload payload;
	payload.hWnd = hWnd;
	payload.msg = msg;
	payload.wParam = wParam;
	payload.lParam = lParam;
	return tofitaPostMessage(&payload);
}

void PostQuitMessage(int32_t nExitCode) {
	tofitaDebugLog(u8"PostQuitMessage called");
	PostMessage(nullptr, wapi::Message::WM_QUIT, (void *)nExitCode, nullptr);
	tofitaDebugLog(u8"PostQuitMessage done");
	// TODO ERROR_SUCCESS
}

wapi::HDc BeginPaint(wapi::HWnd hWnd, wapi::PaintStruct *ps) {
	tofitaDebugLog(u8"BeginPaint called");
	auto window = handleToWindow(hWnd);
	if (window == nullptr)
		return nullptr;
	// We do this every time, cause window may be resized
	njraaGetOrCreateWindowFramebuffer(window->windowId, &window->fb);
	tofitaDebugLog(u8"BeginPaint done");
	return (wapi::HDc)&window->fb; // TODO
}

int32_t FillRect(wapi::HDc dc, const wapi::Rect *lprc, wapi::HBrush brush) {
	tofitaDebugLog(u8"FillRect called");
	// TODO
	tofitaDebugLog(u8"FillRect done");
	return 1;
}

wapi::Bool EndPaint(wapi::HWnd hWnd, wapi::PaintStruct *ps) {
	tofitaDebugLog(u8"EndPaint called");
	auto window = handleToWindow(hWnd);
	if (window == nullptr)
		return 0;
	njraaSwapWindowFramebuffer(window->windowId, &window->fb);
	tofitaDebugLog(u8"EndPaint done");
	return 1;
}

wapi::LResult DefWindowProcW(wapi::HWnd hWnd, wapi::Message uMsg, void *wParam, void *lParam) {
	tofitaDebugLog(u8"DefWindowProcW called");
	auto window = handleToWindow(hWnd);
	if (window == nullptr)
		return 0;
	if (uMsg == wapi::Message::WM_CLOSE) {
		// TODO DestroyWindow(hWnd);
		// TODO probably should be async post?
		window->proc(hWnd, wapi::Message::WM_DESTROY, 0, 0);
		window->proc(hWnd, wapi::Message::WM_NCDESTROY, 0, 0);
		return 0;
	}
	tofitaDebugLog(u8"DefWindowProcW done");
	return 0;
}
}

extern "C" __attribute__((fastcall)) void _DllMainCRTStartup() {
	user32::rootClass.next = nullptr;
}
