// The Tofita Kernel
// Copyright (C) 2020-2023 Oleh Petrenko
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

// TODO describe the purpose
// extern const wchar_t* _wcmdln asm("_wcmdln");
__declspec(dllexport) const wchar_t* _wcmdln asm("_wcmdln") = L"_wcmdln TODO";

// TODO
NTDLL32_DLL uint32_t free(void* value) asm("free");

// TODO on 32-bit, stub should be std call
uint64_t KiFastSystemCall(uint64_t rcx, uint64_t rdx) {
	// TODO return tofitaFastSystemCall((TofitaSyscalls)rcx, rdx);
	return 0;
}

//const wchar_t stubText[] = L"STUB --> called %S with value %u";
#define STUB(name) uint32_t name(void* value) { /*tofitaDebugLog(stubText, (uint64_t)L"" #name, (uint64_t)value);*/ return 0; }
/*
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
uint32_t GetProcessHeap(void* value = nullptr) { tofitaDebugLog(L"STUB --> called %S with value %u", (uint64_t)L"GetProcessHeap", (uint64_t)value); return 0xCAFECAFE; }
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
//STUB(memset)
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
*/
STUB(free)

//#ifdef bit64
//	#define CONV
//#else // __cdecl __stdcall
//	#define CONV __cdecl
//#endif

// DWORD == uint32_t // TODO write down all types sizes NT API callbacks etc
// typedef int32_t(CONV *DllEntry)(void* hinstDLL, uint32_t fdwReason, void* lpvReserved);
// typedef int32_t(CONV *ExeEntry)(void* hInstance, void* hPrev, void* pCmdLine, int nCmdShow);

#define HEAP_C 4096 * 16
static volatile uint8_t heap[HEAP_C] = {0};
static volatile uint64_t heapOffset = 0;
static void* HeapAlloc(volatile int8_t x,volatile void* u, volatile /*uint64_t*/size_t size) {
	// size = ((size - 1) | 7) + 1; // Align by 8
	size = ((size - 1) | 15) + 1; // Align by 16
	if (size < 16) size = 16;
	heapOffset += 8;
	heapOffset += size;
	if (heapOffset >= HEAP_C) {
		//tofitaDebugLog_(L"!!! Heap overflow !!!\n",0,0);
		while (1) {};
	}
	void* result = (void *)&heap[heapOffset - size];
	//tofitaDebugLog_(L"HeapAlloc %u bytes at %8", size, (uint64_t)result);
	return result;
}
static void* HeapAllocAt(size_t lineNumber, char const* filename, char const* functionName, volatile int8_t x,volatile void* u, volatile uint64_t size) {
	//tofitaDebugLog_(L"NTHeapAllocAt %s:%d\n", (uint64_t)functionName, lineNumber);
	return HeapAlloc(x, u, size);
}
//#define HeapAlloc(a, b, c) HeapAllocAt(__LINE__, __FILE__, __func__, a, b, c)
//#define HEXA_NEW(z) HeapAllocAt(__LINE__, __FILE__, __func__, 0,nullptr,z)

// TODO replace impl!
#define strlen(z) 0
void *memcpy(void *dest, const void *src, size_t count) {
	uint8_t *dst8 = (uint8_t *)dest;
	const uint8_t *src8 = (const uint8_t *)src;

	while (count--) {
		*dst8++ = *src8++;
	}

	return dest;
}
#define wprintf(z,...) {}
#define HEAP_ZERO_MEMORY ((void*)0)
#define GetProcessHeap() 0
#define fflush(z) {}
#define HEXA_UNREACHABLE(z) {}
#define stdout ((void*)0)
int64_t _fltused = 0;

#ifdef bit64
#else
int64_t __alldiv() asm("__alldiv");
int64_t __alldiv() { return 0; } // TODO

int64_t __allrem() asm("__allrem");
int64_t __allrem() { return 0; } // TODO

uint64_t __aulldiv() asm("__aulldiv");
uint64_t __aulldiv() { return 0; } // TODO

uint64_t __aullrem() asm("__aullrem");
uint64_t __aullrem() { return 0; } // TODO

void _memset() asm("_memset");
void _memset() { } // TODO TODO TODO
#endif

//#define TRACER() tofitaDebugLog_(L"~TRACER~ [%s:%d]\n", __func__, __LINE__);

#ifdef bit64
	#include "ntdll.64.c"
#else
	#include "ntdll.32.c"
#endif
