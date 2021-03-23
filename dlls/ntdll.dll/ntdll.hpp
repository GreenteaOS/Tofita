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

#include <stdint.h>
#include "../wrappers.hpp"
#include "../types.hpp"

#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)

#ifndef NTDLL32_DLL
	#ifdef bit64
		#define NTDLL32_DLL __declspec(dllimport)
	#else
		#define NTDLL32_DLL __declspec(dllimport) __stdcall
	#endif
#endif

// TODO https://en.cppreference.com/w/cpp/numeric/bit_cast
extern "C" {
NTDLL32_DLL uint64_t KiFastSystemCall(uint64_t rcx, uint64_t rdx) asm("KiFastSystemCall");

#define STUB(name) NTDLL32_DLL uint32_t name(void* value) asm(#name);
#define LSTUB(name, link) NTDLL32_DLL uint32_t name(void* value) asm("*" #link);

LSTUB(LeaveCriticalSection, eaveCriticalSection)
LSTUB(LoadLibraryExW, oadLibraryExW)
LSTUB(LCMapStringW, CMapStringW)

STUB(GetCurrentProcess)
STUB(WriteConsoleW)
STUB(CloseHandle)
STUB(CreateFileW)
STUB(ReadConsoleW)
STUB(ReadFile)
STUB(SetFilePointerEx)
STUB(GetFileSizeEx)
STUB(GetConsoleMode)
STUB(GetConsoleOutputCP)
STUB(FlushFileBuffers)
STUB(HeapReAlloc)
STUB(QueryPerformanceCounter)
STUB(GetCurrentProcessId)
STUB(GetCurrentThreadId)
STUB(GetSystemTimeAsFileTime)
STUB(InitializeSListHead)
STUB(RtlCaptureContext)
STUB(RtlLookupFunctionEntry)
STUB(RtlVirtualUnwind)
STUB(IsDebuggerPresent)
STUB(UnhandledExceptionFilter)
STUB(SetUnhandledExceptionFilter)
STUB(GetStartupInfoW)
STUB(IsProcessorFeaturePresent)
STUB(GetModuleHandleW)
STUB(RtlUnwindEx)
STUB(InterlockedPushEntrySList)
STUB(InterlockedFlushSList)
STUB(GetLastError)
STUB(SetLastError)
STUB(EnterCriticalSection)
STUB(DeleteCriticalSection)
STUB(InitializeCriticalSectionAndSpinCount)
STUB(TlsAlloc)
STUB(TlsGetValue)
STUB(TlsSetValue)
STUB(TlsFree)
STUB(FreeLibrary)
STUB(GetProcAddress)
STUB(EncodePointer)
STUB(RaiseException)
STUB(RtlPcToFileHeader)
STUB(GetStdHandle)
STUB(WriteFile)
STUB(GetModuleFileNameW)
STUB(RtlUnwind)
STUB(TerminateProcess)
STUB(GetModuleHandleExW)
STUB(GetCurrentThread)
STUB(OutputDebugStringW)
STUB(HeapFree)
STUB(FindClose)
STUB(FindFirstFileExW)
STUB(FindNextFileW)
STUB(IsValidCodePage)
STUB(GetACP)
STUB(GetOEMCP)
STUB(GetCPInfo)
STUB(GetCommandLineA)
STUB(GetCommandLineW)
STUB(MultiByteToWideChar)
STUB(WideCharToMultiByte)
STUB(GetEnvironmentStringsW)
STUB(FreeEnvironmentStringsW)
STUB(SetEnvironmentVariableW)
STUB(SetStdHandle)
STUB(GetFileType)
STUB(GetStringTypeW)
STUB(GetLocaleInfoW)
STUB(IsValidLocale)
STUB(GetUserDefaultLCID)
STUB(EnumSystemLocalesW)
STUB(GetDateFormatW)
STUB(GetTimeFormatW)
STUB(CompareStringW)
STUB(GetProcessHeap)
STUB(SetConsoleCtrlHandler)
STUB(HeapSize)

}
