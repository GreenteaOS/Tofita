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

#include <stdint.h>
#include <stddef.h>
#include "../../kernel/syscalls/syscalls.hpp"

#ifndef TOFITA32_DLL
#define TOFITA32_DLL __declspec(dllimport)
#endif

extern "C" {
// Tofita
// Only ecx/edx available in x86-32 fastcall, so use rcx/rdx pair only

#ifdef bit64
TOFITA32_DLL uint64_t tofitaFastSystemCall(TofitaSyscalls rcx, uint64_t rdx = 0);
#else
__attribute__((fastcall)) TOFITA32_DLL uint32_t tofitaFastSystemCall(TofitaSyscalls rcx, uint32_t rdx = 0);
#endif

__attribute__((fastcall)) TOFITA32_DLL uint32_t tofitaFastStub(uint64_t rcx, uint64_t rdx) asm("tofitaFastStub");
__stdcall TOFITA32_DLL uint32_t tofitaStdStub(void* arg) asm("tofitaStdStub");

TOFITA32_DLL void tofitaDebugLog(const wchar_t *message, uint64_t extra = 0, uint64_t more = 0) asm("tofitaDebugLog");

// Kernel32
TOFITA32_DLL void tofitaExitProcess(uint32_t exitCode);

// User32
TOFITA32_DLL uint64_t tofitaCreateWindowEx(CreateWindowExPayload *payload);
TOFITA32_DLL void tofitaShowWindow(uint64_t windowId, int32_t nCmdShow);
TOFITA32_DLL wapi::Bool tofitaGetMessage(GetMessagePayload *payload);
TOFITA32_DLL wapi::Bool tofitaPostMessage(PostMessagePayload *payload);

// NjRAA
TOFITA32_DLL void njraaGetOrCreateWindowFramebuffer(uint64_t windowId, nj::WindowFramebuffer *fb);
TOFITA32_DLL void njraaSwapWindowFramebuffer(uint64_t windowId, nj::WindowFramebuffer *fb);
}
