#define _WIN32_WINNT 0x0400
#include <stdio.h>
#include <windows.h>
#include <tchar.h>

#ifdef _DEBUG
#define	DLL_NAME	"MouseChatteringKillerDllD.dll"
#else
#define	DLL_NAME	"MouseChatteringKillerDll.dll"
#endif

enum {
	WM_MY_TASKTRAY	= WM_USER + 1,

	CM_MY_EXIT		= 1,
};

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

		AppendMenu(hPopupMenu, MF_STRING, CM_MY_EXIT, _T("終了"));
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



static void notifyIcon(HWND hwnd, HICON icon, TCHAR* tipText, bool add = true) {
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
		case WM_RBUTTONUP:			// メニューを表示する
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
	DWORD wndstyle = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU;
	DWORD exstyle  = WS_EX_TOOLWINDOW;
	HWND hwnd = CreateWindowEx(exstyle, _T("edit"), 0, wndstyle, 0, 0, 0, 0, 0, 0, hInstance, 0);
	SetWindowLong(hwnd, GWL_WNDPROC, PtrToLong(WndProc));

	HINSTANCE hInstDLL = LoadLibrary(_T(DLL_NAME));
	if(hInstDLL == 0) {
		SendMessage(hwnd, WM_SETTEXT, 0, (LPARAM) _T(DLL_NAME) _T("が読み込めません\r\n"));
		return 0;
	}

	HICON hicon = LoadIcon(0, IDI_APPLICATION);
	menu.init(hwnd);
	notifyIcon(hwnd, hicon, _T("MouseChatteringKiller"));

	MSG msg;
	while(GetMessage(&msg, 0, 0, 0)) {
		TranslateMessage(&msg);
		DispatchMessageA(&msg);
	}

	notifyIcon(hwnd, 0, 0, false);
	menu.cleanup();
	if(hInstDLL) {
		FreeLibrary(hInstDLL);
	}
	return (int) msg.wParam;
}
