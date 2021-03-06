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

namespace wapi {

typedef void *Handle;
// TODO ifdef 64-bit
// (HandleSize)handle & ~(HandleSize)0b11
typedef uint64_t HandleSize;
typedef Handle HInstance;
typedef Handle HIcon;
typedef Handle HIcon;
typedef Handle HBrush;
typedef Handle HCursor;
typedef Handle HLocal;
typedef Handle HGlobal;
typedef Handle HWnd;
typedef Handle HMenu;
typedef Handle HDc;

typedef void *LResult;

// TODO LResult as enum?
#define ERROR_SUCCESS ((wapi::LResult)0)
#define ERROR_INVALID_HANDLE ((wapi::LResult)6)
#define INVALID_HANDLE_VALUE ((wapi::Handle)-1)

typedef int32_t Bool;

// LONG is signed 32-bit int32_t

// Similiar to handle, has nothing to do with async atomic memory access
typedef uint16_t Atom;

struct Rect {
	int32_t left;
	int32_t top;
	int32_t right;
	int32_t bottom;
};

struct PaintStruct {
	HDc hdc;
	Bool fErase;
	Rect rcPaint;
	Bool fRestore;
	Bool fIncUpdate;
	uint8_t rgbReserved[32];
};

enum class Message : uint32_t {
	WM_NULL = 0x0000,
	WM_CREATE = 0x0001,
	WM_DESTROY = 0x0002,
	WM_MOVE = 0x0003,
	WM_SIZE = 0x0005,
	WM_ACTIVATE = 0x0006,
	WM_SETFOCUS = 0x0007,
	WM_KILLFOCUS = 0x0008,

	WM_ENABLE = 0x000a,
	WM_SETREDRAW = 0x000b,
	WM_SETTEXT = 0x000c,
	WM_GETTEXT = 0x000d,
	WM_GETTEXTLENGTH = 0x000e,
	WM_PAINT = 0x000f,
	WM_CLOSE = 0x0010,
	WM_QUERYENDSESSION = 0x0011,
	WM_QUIT = 0x0012,
	WM_QUERYOPEN = 0x0013,
	WM_ERASEBKGND = 0x0014,
	WM_SYSCOLORCHANGE = 0x0015,
	WM_ENDSESSION = 0x0016,

	WM_SHOWWINDOW = 0x0018,
	WM_ACTIVATEAPP = 0x001c,
	WM_SETCURSOR = 0x0020,
	WM_MOUSEACTIVATE = 0x0021,
	WM_CHILDACTIVATE = 0x0022,
	WM_QUEUESYNC = 0x0023,
	WM_GETMINMAXINFO = 0x0024,
	WM_CONTEXTMENU = 0x007b,
	WM_STYLECHANGING = 0x007c,
	WM_STYLECHANGED = 0x007d,
	WM_DISPLAYCHANGE = 0x007e,
	WM_GETICON = 0x007f,
	WM_SETICON = 0x0080,
	WM_NCCREATE = 0x0081,
	WM_NCDESTROY = 0x0082,
	WM_NCCALCSIZE = 0x0083,
	WM_NCHITTEST = 0x0084,
	WM_NCPAINT = 0x0085,
	WM_NCACTIVATE = 0x0086,
	WM_GETDLGCODE = 0x0087,
	WM_SYNCPAINT = 0x0088,
	WM_NCMOUSEMOVE = 0x00a0,
	WM_NCLBUTTONDOWN = 0x00a1,
	WM_NCLBUTTONUP = 0x00a2,
	WM_NCLBUTTONDBLCLK = 0x00a3,
	WM_NCRBUTTONDOWN = 0x00a4,
	WM_NCRBUTTONUP = 0x00a5,
	WM_NCRBUTTONDBLCLK = 0x00a6,
	WM_NCMBUTTONDOWN = 0x00a7,
	WM_NCMBUTTONUP = 0x00a8,
	WM_NCMBUTTONDBLCLK = 0x00a9,
	BM_GETCHECK = 0x00f0,
	BM_SETCHECK = 0x00f1,
	BM_GETSTATE = 0x00f2,
	BM_SETSTATE = 0x00f3,
	BM_SETSTYLE = 0x00f4,
	BM_CLICK = 0x00f5,
	BM_GETIMAGE = 0x00f6,
	BM_SETIMAGE = 0x00f7,
	WM_KEYFIRST = 0x0100,
	WM_KEYDOWN = 0x0100,
	WM_KEYUP = 0x0101,
	WM_CHAR = 0x0102,
	WM_DEADCHAR = 0x0103,
	WM_SYSKEYDOWN = 0x0104,
	WM_SYSKEYUP = 0x0105,
	WM_SYSCHAR = 0x0106,
	WM_SYSDEADCHAR = 0x0107,
	WM_KEYLAST = 0x0108,
	WM_INITDIALOG = 0x0110,
	WM_COMMAND = 0x0111,
	WM_SYSCOMMAND = 0x0112,
	WM_TIMER = 0x0113,
	WM_MOUSEMOVE = 0x0200,
	WM_LBUTTONDOWN = 0x0201,
	WM_LBUTTONUP = 0x0202,
	WM_RBUTTONDOWN = 0x0204,
	WM_RBUTTONUP = 0x0205,
	WM_MBUTTONDOWN = 0x0207,
	WM_MBUTTONUP = 0x0208,
	WM_SIZING = 0x0214
};

#ifndef SIZEOF
extern "C++"
template<typename T, uint16_t s, uint16_t t> struct checkSize {
    static_assert(s == t, "wrong size");
};
#define SIZEOF(T, SIZE) namespace sizeofChecks { wapi::checkSize<T, sizeof(T), SIZE> __SIZEOF__##T; }
#endif

SIZEOF(Message, 4)

struct Point {
	int32_t x;
	int32_t y;
};
SIZEOF(Point, 8)

struct Msg {
	HWnd hwnd;
	Message message;
	void *wParam;
	void *lParam;
	uint32_t time;
	Point pt;

#ifdef bit64
	// Hidden under _MAC, safe to use? Why it is than sizeof(48) in MSVC
	// Documentation explicitly assumes it anyway
	uint32_t lPrivate;
#endif
};

#ifdef bit64
SIZEOF(Msg, 48)
SIZEOF(HWnd, 8)
#else
SIZEOF(Msg, 28)
SIZEOF(HWnd, 4)
#endif

#ifdef bit64
typedef LResult (*WindowProcedure)(HWnd hWnd, Message uMsg, void *wParam, void *lParam);
#else
// TODO MUST support both cdecl and stdcall (or fastcall?) cause some apps do it
// ^ use Linux convention to pass all args + func pointer, and build frame from asm & jmp
typedef LResult (__stdcall *WindowProcedure)(HWnd hWnd, Message uMsg, void *wParam, void *lParam);
#endif

struct WindowClass {
	uint32_t style;
	WindowProcedure lpfnWndProc;
	int32_t cbClsExtra;
	int32_t cbWndExtra;
	HInstance hInstance;
	HIcon hIcon;
	HCursor hCursor;
	HBrush hbrBackground;
	const wchar_t *lpszMenuName;
	const wchar_t *lpszClassName;
};

#ifdef bit64
_Static_assert(sizeof(WindowClass) == 72, "bad sizeof");
#else
_Static_assert(sizeof(WindowClass) == 40, "bad sizeof");
#endif

#define SW_HIDE 0
#define SW_SHOWNORMAL 1
#define SW_NORMAL 1
#define SW_SHOWMINIMIZED 2
#define SW_SHOWMAXIMIZED 3
#define SW_MAXIMIZE 3
#define SW_SHOWNOACTIVATE 4
#define SW_SHOW 5
#define SW_MINIMIZE 6
#define SW_SHOWMINNOACTIVE 7
#define SW_SHOWNA 8
#define SW_SHOWRESTORE 9
#define SW_SHOWDEFAULT 10
#define SW_FORCEMINIMIZE 11
#define SW_MAX 11

} // namespace wapi

namespace tofi {} // namespace tofi

namespace nj {

typedef struct {
	uint8_t r;
	uint8_t g;
	uint8_t b;
	uint8_t a;
} __attribute__((packed)) PixelRGBAData;

struct Pixel32 {
	union {
		PixelRGBAData rgba;
		uint32_t color;
	};
};

SIZEOF(Pixel32, 4)

struct WindowFramebuffer {
	Pixel32 *pixels;
	uint16_t width;
	uint16_t height;
};

} // namespace nj
