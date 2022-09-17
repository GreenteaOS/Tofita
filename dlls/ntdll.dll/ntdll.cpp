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
#include <stddef.h>

#ifdef bit64
	#define NTDLL32_DLL __declspec(dllexport)
#else
	#define NTDLL32_DLL __declspec(dllexport) __stdcall
#endif

#include "ntdll.hpp"

extern "C" {
uint64_t KiFastSystemCall(uint64_t rcx, uint64_t rdx) {
	return tofitaFastSystemCall((TofitaSyscalls)rcx, rdx);
}

const wchar_t stubText[] = L"STUB --> called %S with value %u";
#define STUB(name) uint32_t name(void* value) { tofitaDebugLog(stubText, (uint64_t)L"" #name, (uint64_t)value); return 0; }

STUB(CloseHandle)
STUB(CompareStringW)
STUB(CreateFileW)
STUB(DeleteCriticalSection)
STUB(EncodePointer)
STUB(EnterCriticalSection)
STUB(EnumSystemLocalesW)
STUB(FindClose)
STUB(FindFirstFileExW)
STUB(FindNextFileW)
STUB(FlushFileBuffers)
STUB(FreeEnvironmentStringsW)
STUB(FreeLibrary)
STUB(GetACP)
STUB(GetCommandLineA)
STUB(GetCommandLineW)
STUB(GetConsoleMode)
STUB(GetConsoleOutputCP)
STUB(GetCPInfo)
STUB(GetCurrentProcess)
STUB(GetCurrentProcessId)
STUB(GetCurrentThread)
STUB(GetCurrentThreadId)
STUB(GetDateFormatW)
STUB(GetEnvironmentStringsW)
STUB(GetFileSizeEx)
STUB(GetFileType)
STUB(GetLastError)
STUB(GetLocaleInfoW)
STUB(GetModuleFileNameW)
STUB(GetModuleHandleExW)
STUB(GetModuleHandleW)
STUB(GetOEMCP)
STUB(GetProcAddress)
uint32_t GetProcessHeap(void* value) { tofitaDebugLog(L"STUB --> called %S with value %u", (uint64_t)L"GetProcessHeap", (uint64_t)value); return 0xCAFECAFE; }
STUB(GetStartupInfoW)
STUB(GetStdHandle)
STUB(GetStringTypeW)
STUB(GetSystemTimeAsFileTime)
STUB(GetTimeFormatW)
STUB(GetUserDefaultLCID)
STUB(HeapFree)
STUB(HeapReAlloc)
STUB(HeapSize)
STUB(InitializeSListHead)
STUB(InterlockedFlushSList)
STUB(InterlockedPushEntrySList)
STUB(IsDebuggerPresent)
STUB(IsProcessorFeaturePresent)
STUB(IsValidCodePage)
STUB(IsValidLocale)
STUB(LCMapStringW)
STUB(LeaveCriticalSection)
STUB(MultiByteToWideChar)
STUB(OutputDebugStringW)
STUB(QueryPerformanceCounter)
STUB(RaiseException)
STUB(ReadConsoleW)
STUB(ReadFile)
STUB(RtlCaptureContext)
STUB(RtlLookupFunctionEntry)
STUB(RtlPcToFileHeader)
STUB(RtlUnwind)
STUB(RtlUnwindEx)
STUB(RtlVirtualUnwind)
STUB(SetConsoleCtrlHandler)
STUB(SetEnvironmentVariableW)
STUB(SetFilePointerEx)
STUB(SetLastError)
STUB(SetStdHandle)
STUB(SetUnhandledExceptionFilter)
STUB(TerminateProcess)
STUB(TlsAlloc)
STUB(TlsFree)
STUB(TlsGetValue)
STUB(UnhandledExceptionFilter)
STUB(WideCharToMultiByte)
STUB(WriteConsoleW)
STUB(WriteFile)

STUB(AppendMenuW)
STUB(Beep)
STUB(CallWindowProcW)
STUB(CheckDlgButton)
STUB(CheckMenuItem)
STUB(CheckRadioButton)
STUB(CloseClipboard)
STUB(CreateDialogParamW)
STUB(CreateDirectoryA)
STUB(CreateDirectoryW)
STUB(CreatePipe)
STUB(CreatePopupMenu)
STUB(CreateProcessA)
STUB(CreateProcessW)
STUB(CreateThread)
STUB(DbgPrint)
STUB(DeleteFileA)
STUB(DeleteFileW)
STUB(DestroyIcon)
STUB(DestroyMenu)
STUB(DrawFrameControl)
STUB(DrawStateW)
STUB(DuplicateHandle)
STUB(EmptyClipboard)
STUB(EnableMenuItem)
STUB(EnableWindow)
STUB(EnumChildWindows)
STUB(EnumResourceLanguagesA)
STUB(ExitThread)
STUB(FindFirstFileA)
STUB(FindFirstFileW)
STUB(FindNextFileA)
STUB(FreeEnvironmentStringsA)
STUB(GetAsyncKeyState)
STUB(GetClassNameW)
STUB(GetClipboardData)
STUB(GetCurrentDirectoryA)
STUB(GetCurrentDirectoryW)
STUB(GetDateFormatA)
STUB(GetDiskFreeSpaceA)
STUB(GetDlgItem)
STUB(GetDlgItemTextW)
STUB(GetDriveTypeA)
STUB(GetDriveTypeW)
STUB(GetEnvironmentStrings)
STUB(GetEnvironmentVariableA)
STUB(GetExitCodeProcess)
STUB(GetFileAttributesA)
STUB(GetFileAttributesExA)
STUB(GetFileAttributesExW)
STUB(GetFileAttributesW)
STUB(GetFileInformationByHandle)
STUB(GetFocus)
STUB(GetFullPathNameA)
STUB(GetFullPathNameW)
STUB(GetKeyboardLayout)
STUB(GetKeyboardState)
STUB(GetLocaleInfoA)
STUB(GetLocalTime)
STUB(GetMenu)
STUB(GetModuleFileNameA)
STUB(GetModuleHandleA)
STUB(GetNumberOfConsoleInputEvents)
STUB(GetParent)
STUB(GetProfileIntW)
STUB(GetStartupInfoA)
STUB(GetStockObject)
STUB(GetStringTypeA)
STUB(GetStringTypeExA)
STUB(GetSubMenu)
STUB(GetSystemDefaultLCID)
STUB(GetSystemMetrics)
STUB(GetTempFileNameA)
STUB(GetTempFileNameW)
STUB(GetTextExtentPoint32W)
STUB(GetTickCount)
STUB(GetTimeFormatA)
STUB(GetTimeZoneInformation)
STUB(GetVersionExW)
STUB(GetWindowLongPtrW)
STUB(GetWindowRect)
STUB(GetWindowTextW)
STUB(GlobalLock)
STUB(GlobalUnlock)
STUB(HeapCompact)
STUB(HeapValidate)
STUB(HeapWalk)
STUB(InitCommonControls)
STUB(InitializeCriticalSection)
STUB(InvalidateRect)
STUB(IsClipboardFormatAvailable)
STUB(IsDlgButtonChecked)
STUB(IsWindow)
STUB(IsWindowEnabled)
STUB(MoveFileExA)
STUB(MoveFileExW)
STUB(MoveWindow)
STUB(OpenClipboard)
STUB(PeekConsoleInputA)
STUB(PostMessageW)
STUB(ReadConsoleA)
STUB(RegCloseKey)
STUB(RegCreateKeyExW)
STUB(RegOpenKeyExW)
STUB(RegQueryValueExW)
STUB(RegSetValueExW)
STUB(RemoveDirectoryA)
STUB(RemoveDirectoryW)
STUB(ResumeThread)
STUB(RtlAllocateHeap)
STUB(RtlAreBitsClear)
STUB(RtlAreBitsSet)
STUB(RtlFreeHeap)
STUB(RtlInitializeBitMap)
STUB(RtlRaiseException)
STUB(RtlSecondsSince1970ToTime)
STUB(RtlSetBits)
STUB(RtlTimeToSecondsSince1970)
STUB(SearchPathA)
STUB(SearchPathW)
STUB(SendDlgItemMessageW)
STUB(SendMessageW)
STUB(SetBkMode)
STUB(SetClipboardData)
STUB(SetConsoleMode)
STUB(SetCurrentDirectoryA)
STUB(SetCurrentDirectoryW)
STUB(SetDlgItemTextW)
STUB(SetEndOfFile)
STUB(SetErrorMode)
STUB(SetFileAttributesA)
STUB(SetFileAttributesW)
STUB(SetFilePointer)
STUB(SetFileTime)
STUB(SetFocus)
STUB(SetLocalTime)
STUB(SetMenu)
STUB(SetTextColor)
STUB(SetWindowLongPtrW)
STUB(ShellAboutW)
STUB(Sleep)
STUB(ToAsciiEx)
STUB(TrackMouseEvent)
STUB(TrackPopupMenu)
STUB(TryEnterCriticalSection)
STUB(UnlockFile)
STUB(vDbgPrintExWithPrefix)
STUB(VirtualProtect)
STUB(WaitForSingleObject)
STUB(WriteConsoleA)
STUB(WriteProfileStringW)

uint32_t InitializeCriticalSectionAndSpinCount(void* value) { tofitaDebugLog(stubText, (uint64_t)L"InitializeCriticalSectionAndSpinCount", (uint64_t)value); return 1; }
uint32_t LoadLibraryExW(void* value) { tofitaDebugLog(L"STUB --> called %S with value %S", (uint64_t)L"LoadLibraryExW", 0*(uint64_t)value); return 0; }
uint32_t TlsSetValue(void* value) { tofitaDebugLog(L"STUB --> called %S with value %u", (uint64_t)L"TlsSetValue", (uint64_t)value); return 1; }


STUB(LCMapStringA)
STUB(LoadImageW)
STUB(LoadLibraryA)
STUB(LoadLibraryW)
STUB(LoadMenuW)
STUB(LoadStringW)
STUB(LocalFree)
STUB(LocalReAlloc)
STUB(LockFile)
STUB(lstrcatW)
STUB(lstrcpynA)
STUB(lstrcpyW)
STUB(lstrlenW)
STUB(MapVirtualKeyExW)

STUB(wcschr)
STUB(__lconv_init)
STUB(signal)
STUB(_iob)
STUB(fprintf)
STUB(_commode)
STUB(_fmode)
STUB(__winitenv)
STUB(_initterm)
STUB(_fpreset)
STUB(_cexit)
STUB(exit)
STUB(__set_app_type)
STUB(__wgetmainargs)
STUB(_amsg_exit)
STUB(__setusermatherr)
STUB(_wcsicmp)
STUB(wcslen)
STUB(wcscpy)
STUB(wcscat)
STUB(_strupr)
STUB(_strdup)
STUB(strlen)
STUB(memset)
STUB(memcpy)
STUB(swscanf)
STUB(swprintf)
STUB(cosh)
STUB(memmove)
STUB(wcscmp)
STUB(acos)
STUB(asin)
STUB(atan)
STUB(cos)
STUB(exp)
STUB(fabs)
STUB(fmod)
STUB(log)
STUB(log10)
STUB(pow)
STUB(sin)
STUB(sinh)
STUB(sqrt)
STUB(tan)
STUB(tanh)
STUB(ceil)
STUB(modf)
STUB(_finite)
STUB(_isnan)
STUB(free)

#ifdef bit64
	#define CONV
#else // __cdecl __stdcall
	#define CONV __cdecl
#endif

// DWORD == uint32_t // TODO write down all types sizes NTAPI callbacks etc
typedef bool(CONV *DllEntry)(void* hinstDLL, uint32_t fdwReason, void* lpvReserved);
typedef int32_t(CONV *ExeEntry)(void* hInstance, void* hPrev, void* pCmdLine, int nCmdShow);

// TODO Hehe just use uint32_t for PIDs
void __attribute__((fastcall)) greenteaosIsTheBest(ExeEntry entry, void* pid, void* dllEntries) asm("greenteaosIsTheBest");
void __attribute__((fastcall)) greenteaosIsTheBest(ExeEntry entry, void* pid, void* dllEntries) {
	// TODO entry arguments (argv, argc)
	// TODO init DLLs
	// TODO PEB/TEB
	// TODO TLS callbacks
	// TODO load DLLs in usermode
	_wcmdln = L"_wcmdln";

	#ifdef bit64
		tofitaDebugLog(L"64-bit CRT ready for PID %u", (uint64_t)pid);
	#else
		tofitaDebugLog(L"32-bit CRT ready for PID %u", (uint64_t)pid);
	#endif

	auto count = (size_t*)dllEntries;
	auto dllMains = (DllEntry*)dllEntries;
	size_t i = 0;
	auto DLL_PROCESS_ATTACH = 1;

	while (i < count[0]) {
		i++;
		dllMains[i](nullptr, DLL_PROCESS_ATTACH, nullptr);
	}

	tofitaDebugLog(L"Done DLLs");

	tofitaExitProcess_(entry(nullptr, nullptr, nullptr, 0));
	while (true) {};
}
}
