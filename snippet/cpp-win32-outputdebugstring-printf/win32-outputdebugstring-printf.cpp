#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <tchar.h>
#include <stdio.h>
#include <locale.h>

// extern "C" __declspec(dllimport) void __stdcall OutputDebugStringA(const char*);
// extern "C" __declspec(dllimport) void __stdcall OutputDebugStringW(const wchar_t*);

void dbgPuts(const char* str) {
	OutputDebugStringA(str);
}

void dbgPrintf(const char* fmt, ...) {
	va_list args;
	va_start(args, fmt);
	char buf[4096];
	vsnprintf_s(buf, sizeof(buf), fmt, args);
	dbgPuts(buf);
	va_end(args);
}


void dbgPuts(const wchar_t* str) {
	OutputDebugStringW(str);
}

void dbgPrintf(const wchar_t* fmt, ...) {
	va_list args;
	va_start(args, fmt);
	wchar_t buf[4096];
	_vsnwprintf_s(buf, sizeof(buf)/sizeof(buf[0]), fmt, args);
	dbgPuts(buf);
	va_end(args);
}


int main() {
	dbgPrintf(_T("%s(%d)"), _T("TCHAR"), __LINE__);
	dbgPrintf(L"%s(%d)", L"wchar_t", __LINE__);
	dbgPrintf("%s(%d)", "char", __LINE__);
}
