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

// wchar_t is UTF-16LE with -fshort-wchar
_Static_assert(sizeof(wchar_t) == 2, "bad sizeof");

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

struct DebugLogPayload {
	const wchar_t *message;
	// TODO count chars at userspace, and transfer as length for SEH probing
	uint64_t extra = 0;
	uint64_t more = 0;
};

_Static_assert(sizeof(DebugLogPayload) == 8 * 3, "bad sizeof");

struct SwapWindowFramebufferPayload {
	uint64_t windowId;
	nj::WindowFramebuffer *fb;
};

_Static_assert(sizeof(SwapWindowFramebufferPayload) == 8 * 2, "bad sizeof");

struct ShowWindowPayload {
	int32_t nCmdShow;
	uint64_t windowId;
};

_Static_assert(sizeof(ShowWindowPayload) == 8 * 2, "bad sizeof");

// TODO use pointer wrappers instead of raw pointers
struct CreateWindowExPayload {
	const wapi::WindowClass *wc;
	wapi::HWnd hWnd;

	// CreateWindowEx
	const wchar_t *lpClassName;
	// TODO count chars at userspace, and transfer as length for SEH probing
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
