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

#define TOFITA32_DLL __declspec(dllexport)
#include "tofita32.hpp"
#include "tofita32Vars.hpp"

extern "C" {
// High performance function, that does completely nothing
uint32_t tofitaFastStub(uint64_t rcx, uint64_t rdx) {
	tofitaDebugLog(L"--> tofitaFastStub --> rcx=%8 rdx=%8", rcx, rdx);
	return 0;
}

uint32_t tofitaStdStub(void* arg) {
	tofitaDebugLog(L"--> tofitaStdStub --> arg=%8", (uint64_t)arg);
	return 0;
}

// External
#ifdef bit64
uint64_t tofitaFastSystemCallWrapper(TofitaSyscalls rcx, uint64_t rdx);
#else
uint32_t __attribute__((fastcall)) tofitaFastSystemCallWrapper(TofitaSyscalls rcx, uint32_t rdx);
#endif

// Return value is placed into RAX
#ifdef bit64
uint64_t tofitaFastSystemCall(TofitaSyscalls rcx, uint64_t rdx)
#else
uint32_t tofitaFastSystemCall(TofitaSyscalls rcx, uint32_t rdx)
#endif
{
	return tofitaFastSystemCallWrapper(rcx, rdx);
}

void tofitaExitProcess(uint32_t exitCode) {
	tofitaFastSystemCall(TofitaSyscalls::ExitProcess, exitCode);
}

void tofitaDebugLog(const wchar_t *message, uint64_t extra, uint64_t more) {
	DebugLogPayload payload;
	payload.message = message;
	payload.extra = extra;
	payload.more = more;
	tofitaFastSystemCall(TofitaSyscalls::DebugLog, (uint64_t)&payload);
}

void njraaSwapWindowFramebuffer(uint64_t windowId, nj::WindowFramebuffer *fb) {
	SwapWindowFramebufferPayload payload;
	payload.windowId = windowId;
	payload.fb = fb;
	tofitaFastSystemCall(TofitaSyscalls::SwapWindowFramebuffer, (uint64_t)&payload);
}

void njraaGetOrCreateWindowFramebuffer(uint64_t windowId, nj::WindowFramebuffer *fb) {
	SwapWindowFramebufferPayload payload;
	payload.windowId = windowId;
	payload.fb = fb;
	tofitaFastSystemCall(TofitaSyscalls::GetOrCreateWindowFramebuffer, (uint64_t)&payload);
}

uint64_t tofitaCreateWindowEx(CreateWindowExPayload *payload) {
	return tofitaFastSystemCall(TofitaSyscalls::CreateWindowEx, (uint64_t)payload);
}

void tofitaShowWindow(uint64_t windowId, int32_t nCmdShow) {
	ShowWindowPayload payload;
	payload.windowId = windowId;
	payload.nCmdShow = nCmdShow;
	tofitaFastSystemCall(TofitaSyscalls::ShowWindow, (uint64_t)&payload);
}

wapi::Bool tofitaGetMessage(GetMessagePayload *payload) {
	// TODO filters
	return (wapi::Bool)tofitaFastSystemCall(TofitaSyscalls::GetMessage, (uint64_t)payload);
}

wapi::Bool tofitaPostMessage(PostMessagePayload *payload) {
	return (wapi::Bool)tofitaFastSystemCall(TofitaSyscalls::PostMessage, (uint64_t)payload);
}
}
