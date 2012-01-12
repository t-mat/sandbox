#define _WIN32_WINNT 0x0502
#include <stdio.h>
#include <windows.h>
#include <tchar.h>

#if !defined(_DEBUG)
#define	DLL64_NAME	"x64_kbdacc_dll64.dll"
static const TCHAR* appName = _T("x64_kbdacc");
#else
#define	DLL64_NAME	"x64_kbdacc_dll64D.dll"
static const TCHAR* appName = _T("x64_kbdaccD");
#endif

#if !defined(_WIN64)
#error This code must be compiled at 'x64' project environment.
#endif

enum {
	WM_MY_TASKTRAY	= WM_USER + 1,

	CM_MY_EXIT		= 1,
};

///////////////////////////////////////////////////////////////
void debugOut(const TCHAR* fmt, ...) {
	va_list args;
	va_start(args, fmt);

	TCHAR buf[1024];
	wvsprintf(buf, fmt, args);
	OutputDebugString(buf);

	va_end(args);
}



///////////////////////////////////////////////////////////////
struct Menu {
	HMENU	hMenu;
	HMENU	hPopupMenu;
	HWND	hWnd;

	Menu() : hMenu(0), hPopupMenu(0), hWnd(0) {
	}

	~Menu() {
		cleanup();
	}

	void init(HWND hWnd) {
		this->hWnd = hWnd;

		hMenu = CreateMenu();
		hPopupMenu = CreatePopupMenu();

		AppendMenu(hPopupMenu, MF_STRING, CM_MY_EXIT, _T("Quit"));
		AppendMenu(hMenu, MF_STRING | MF_POPUP, (UINT) hPopupMenu, _T(""));

		SetMenu(hWnd, hMenu);
	}

	void cleanup() {
	}

	BOOL trackPopupMenu(const POINT& p) {
		if(! hWnd) {
			return FALSE;
		} else {
			return TrackPopupMenu(hPopupMenu, TPM_RIGHTALIGN | TPM_BOTTOMALIGN, p.x, p.y, 0, hWnd, 0);
		}
	}
};

static Menu menu;



static void notifyIcon(HWND hwnd, HICON icon, const TCHAR* tipText, bool add = true) {
	NOTIFYICONDATA	 nid = { 0 };

	nid.cbSize				= sizeof(nid);
	nid.uID					= 1;
	nid.hWnd				= hwnd;
	nid.uFlags				= NIF_MESSAGE | NIF_ICON | NIF_TIP;
	nid.hIcon				= icon;
	nid.uCallbackMessage	= WM_MY_TASKTRAY;

	if(tipText) {
		lstrcpy(nid.szTip, tipText);
	}

	if(add) {
		Shell_NotifyIcon(NIM_ADD, &nid);
	} else {
		Shell_NotifyIcon(NIM_DELETE, &nid);
	}
}


static LRESULT CALLBACK WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	switch(uMsg) {
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	case WM_SYSDEADCHAR:
	case WM_SYSCHAR:
	case WM_SYSKEYDOWN:
	case WM_SYSKEYUP:
	case WM_KEYDOWN:
		break;
	case WM_COMMAND:
		switch(LOWORD(wParam)) {
		default:
			break;
		case CM_MY_EXIT:
			SendMessage(hwnd, WM_CLOSE, 0, 0);
			break;
		}
		break;
	case WM_MY_TASKTRAY:
		switch(lParam) {
		case WM_RBUTTONUP:
			{
				SetForegroundWindow(hwnd);
				SetFocus(hwnd);
				POINT p;
				GetCursorPos(&p);
				menu.trackPopupMenu(p);
			}
			break;

		case WM_LBUTTONDOWN:
		case WM_LBUTTONUP:
		case WM_LBUTTONDBLCLK:
		case WM_RBUTTONDOWN:
		default:
			break;
		}
		return 0;
		break;
	default:
		return DefWindowProc(hwnd, uMsg, wParam, lParam);
	}
	return 0;
}



int __stdcall WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int) {
	DWORD wndstyle = WS_OVERLAPPED;
	DWORD exstyle  = WS_EX_TOOLWINDOW;

	HWND hwnd;
	{
		WNDCLASS c = { 0 };
		c.lpfnWndProc	= WndProc;
		c.hInstance		= hInstance;
		c.lpszClassName	= appName;
		RegisterClass(&c);
		hwnd = CreateWindowEx(exstyle, c.lpszClassName, 0, wndstyle, 0, 0, 0, 0, 0, 0, hInstance, 0);
	}

	HINSTANCE hInstDLL = LoadLibrary(_T(DLL64_NAME));
	if(! hInstDLL) {
		MessageBox(hwnd, _T("ERROR\nCan't load DLL : ") _T(DLL64_NAME), appName, MB_OK | MB_ICONINFORMATION);
		return 0;
	}

	menu.init(hwnd);

	HICON hicon = LoadIcon(0, IDI_APPLICATION);
	notifyIcon(hwnd, hicon, appName);

	MSG msg;
	while(GetMessage(&msg, 0, 0, 0)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	notifyIcon(hwnd, 0, 0, false);
	menu.cleanup();
	Sleep(1000);
	FreeLibrary(hInstDLL);
	return (int) msg.wParam;
}
