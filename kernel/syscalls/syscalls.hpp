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

#pragma once

#define USER_SYSCALLS 123000

enum class TofitaSyscalls : uint64_t {
	// System
	Noop = 0,
	Cpu = 1,

	// User
	DebugLog = USER_SYSCALLS,
	CreateWindowEx,
	ShowWindow,
	GetMessage,
	PostMessage,
	GetOrCreateWindowFramebuffer,
	SwapWindowFramebuffer,
	ExitProcess
};

struct CreateWindowExPayload {
	const wapi::WindowClass *wc;
	wapi::HWnd hWnd;

	// CreateWindowEx
	const wchar_t *lpClassName;
	const wchar_t *lpWindowName;
	uint32_t dwStyle;
	int32_t x;
	int32_t y;
	int32_t nWidth;
	int32_t nHeight;
	wapi::HWnd hWndParent;
	wapi::HMenu hMenu;
	wapi::HInstance hInstance;
	void *lpParam;
};

_Static_assert(sizeof(CreateWindowExPayload) == 8 * 8 + 5 * 4 + 4, "bad sizeof");

struct PostMessagePayload {
	wapi::HWnd hWnd;
	wapi::Message msg;
	void *wParam;
	void *lParam;
};

_Static_assert(sizeof(PostMessagePayload) == 8 + 4 + 8 * 2 + 4, "bad sizeof");

struct GetMessagePayload {
	wapi::Msg *msg;
	wapi::HWnd hWnd;
	uint32_t wMsgFilterMin;
	uint32_t wMsgFilterMax;
};

_Static_assert(sizeof(GetMessagePayload) == 8 * 2 + 4 * 2, "bad sizeof");
