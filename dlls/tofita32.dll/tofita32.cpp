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
uint32_t tofitaFastStub() {
	return 0;
}

// Return value is placed into RAX
uint64_t tofitaFastSystemCall(TofitaSyscalls rcx, uint64_t rdx, uint64_t r8, uint64_t r9) {
	asm volatile("int $0x80" ::);
	asm volatile("ret" ::);
}

void tofitaExitProcess(uint32_t exitCode) {
	tofitaFastSystemCall(TofitaSyscalls::ExitProcess, exitCode);
}

void tofitaDebugLog(const char8_t *message, uint64_t extra, uint64_t more) {
	tofitaFastSystemCall(TofitaSyscalls::DebugLog, (uint64_t)message, extra, more);
}

void njraaUploadWindowFramebuffer(uint64_t windowId) {
	tofitaFastSystemCall(TofitaSyscalls::UploadWindowFramebuffer, (uint64_t)windowId);
}

nj::WindowFramebuffer njraaGetOrCreateWindowFramebuffer(uint64_t windowId) {
	tofitaFastSystemCall(TofitaSyscalls::GetOrCreateWindowFramebuffer, (uint64_t)windowId);
}

uint64_t tofitaCreateWindowEx(CreateWindowExPayload *payload) {
	tofitaFastSystemCall(TofitaSyscalls::CreateWindowEx, (uint64_t)payload);
}

void tofitaShowWindow(uint64_t windowId, int32_t nCmdShow) {
	tofitaFastSystemCall(TofitaSyscalls::ShowWindow, (uint64_t)windowId, nCmdShow);
}

wapi::Bool tofitaGetMessage(GetMessagePayload *payload) {
	return (wapi::Bool)tofitaFastSystemCall(TofitaSyscalls::GetMessage, (uint64_t)payload);
}

wapi::Bool tofitaPostMessage(PostMessagePayload *payload) {
	return (wapi::Bool)tofitaFastSystemCall(TofitaSyscalls::PostMessage, (uint64_t)payload);
}
}
