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

#include "../ntdll.dll/ntdll.hpp"
#include "../tofita32.dll/tofita32.hpp"

#ifndef USER32_DLL
	#ifdef bit64
		#define USER32_DLL __declspec(dllimport)
	#else
		#define USER32_DLL __declspec(dllimport) __stdcall
	#endif
#endif

extern "C" {
USER32_DLL wapi::Bool DestroyWindow(wapi::HWnd hWnd) asm("DestroyWindow");
USER32_DLL wapi::Atom RegisterClassW(const wapi::WindowClass *wc) asm("RegisterClassW");
USER32_DLL wapi::HWnd CreateWindowExW(uint32_t dwExStyle, const wchar_t *lpClassName,
									  const wchar_t *lpWindowName, uint32_t dwStyle, int32_t x, int32_t y,
									  int32_t nWidth, int32_t nHeight, wapi::HWnd hWndParent,
									  wapi::HMenu hMenu, wapi::HInstance hInstance, void *lpParam) asm("CreateWindowExW");
USER32_DLL wapi::Bool ShowWindow(wapi::HWnd hWnd, int32_t nCmdShow) asm("ShowWindow");
USER32_DLL wapi::Bool GetMessageW(wapi::Msg *msg, wapi::HWnd hWnd, uint32_t wMsgFilterMin,
								  uint32_t wMsgFilterMax) asm("GetMessageW");
USER32_DLL wapi::Bool TranslateMessage(wapi::Msg *msg) asm("TranslateMessage");
USER32_DLL wapi::LResult DispatchMessageW(wapi::Msg *msg) asm("DispatchMessageW");
USER32_DLL void PostQuitMessage(int32_t nExitCode) asm("PostQuitMessage");
USER32_DLL wapi::HDc BeginPaint(wapi::HWnd hWnd, wapi::PaintStruct *ps) asm("BeginPaint");
USER32_DLL int32_t FillRect(wapi::HDc dc, const wapi::Rect *lprc, wapi::HBrush brush) asm("FillRect");
USER32_DLL wapi::Bool EndPaint(wapi::HWnd hWnd, wapi::PaintStruct *ps) asm("EndPaint");
USER32_DLL wapi::LResult DefWindowProcW(wapi::HWnd hWnd, wapi::Message uMsg, void *wParam, void *lParam) asm("DefWindowProcW");
USER32_DLL wapi::Bool PostMessage(wapi::HWnd hWnd, wapi::Message msg, void *wParam, void *lParam) asm("PostMessage");
}
