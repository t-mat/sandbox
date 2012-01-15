// --------------------------------------------------------------------------
/**
	SetWindowsHookEx
	http://msdn.microsoft.com/library/ja/default.asp?url=/library/ja/jpipc/html/_win32_setwindowshookex.asp

	CallNextHookEx
	http://msdn.microsoft.com/library/ja/default.asp?url=/library/ja/jpipc/html/_win32_callnexthookex.asp

	Keylogging fun for C/C++ Beginners
	http://bothack.wordpress.com/2007/12/12/keylogging-fun-for-cc-beginners/
 */
//---------------------------------------------------------------------------
#define _WIN32_WINNT 0x0400
#include <stdio.h>
#include <windows.h>
#include <tchar.h>



class MouseHook {
	typedef	__int64	Time;

protected:
	MouseHook(HINSTANCE hInstDll)
		: hook(0)
	{
		OutputDebugStringA(__FUNCTION__"\n");
		init(hInstDll);
	}

	~MouseHook() {
		cleanup();
	}

	void init(HINSTANCE hInstDll) {
		OutputDebugStringA(__FUNCTION__"\n");
		hook = SetWindowsHookEx(WH_MOUSE_LL, static_hookMouseLl, hInstDll, 0);

		if(hook == 0) {
			DWORD err = GetLastError();
			LPVOID lpMsgBuf;
			FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
						  0, err, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR) &lpMsgBuf, 0, 0);
			MessageBox(0, (LPCTSTR)lpMsgBuf, _T("Error"), MB_OK | MB_ICONINFORMATION);
			LocalFree(lpMsgBuf);
		} else {
			prevLeftDownTime	= getCurrentTime();
			prevLeftUpTime		= getCurrentTime();
			prevRightDownTime	= getCurrentTime();
			prevRightUpTime		= getCurrentTime();
		}
	}

	void cleanup() {
		if(hook) {
			UnhookWindowsHookEx(hook);
			hook = 0;
		}
	}

	void setChatteringThresholdTime(float f) {
		Time t;
		QueryPerformanceFrequency((LARGE_INTEGER*) &t);

		double d = f;
		d *= t;

		chatteringThreshold = (Time) d;
	}

	static Time getCurrentTime() {
		Time t;
		QueryPerformanceCounter((LARGE_INTEGER*) &t);
		return t;
	}

	bool isChattering(const Time& prevTime, const Time& currentTime) {
		Time d = currentTime - prevTime;
		return d < chatteringThreshold;
	}

	static LRESULT CALLBACK static_hookMouseLl(int nCode, WPARAM wParam, LPARAM lParam) {
		return instance->hookMouseLl(nCode, wParam, lParam);
	}

	LRESULT hookMouseLl(int nCode, WPARAM wParam, LPARAM lParam) {
		bool callNext = true;

		if(nCode >= 0) {
			MSLLHOOKSTRUCT* msll = reinterpret_cast<MSLLHOOKSTRUCT*>(lParam);

			// --------------------------------------------------------------------------
			/**
				押下時のチャタリング

					--u---	d-----------------
					--u---	dud---------------
					--u---	dudud-------------

				離し時のチャタリング

					--d--	u-----------------
					--d--	udu---------------
					--d--	ududu-------------

				ドラッグ時のチャタリング (用語間違い？　接触や通信不良のこと)

					d-------dud-------
			 */
			//---------------------------------------------------------------------------
			switch(wParam) {
			case WM_LBUTTONDOWN:
				{
					Time currentTime = getCurrentTime();
					//	チャタリングが発生しているか？
					if(isChattering(prevLeftUpTime, currentTime) || isChattering(prevLeftDownTime, currentTime)) {
						//	してるっぽいので何もしない
						callNext = false;
					}
					prevLeftDownTime = currentTime;
					break;
				}
			case WM_LBUTTONUP:
				{
					Time currentTime = getCurrentTime();
					//	チャタリングが発生しているか？
					if(isChattering(prevLeftDownTime, currentTime) || isChattering(prevLeftUpTime, currentTime)) {
						//	してるっぽいので何もしない
						callNext = false;
					}
					prevLeftUpTime = currentTime;
					break;
				}

			case WM_RBUTTONDOWN:
				{
					Time currentTime = getCurrentTime();
					//	チャタリングが発生しているか？
					if(isChattering(prevRightUpTime, currentTime)) {
						//	してるっぽいので何もしない
						callNext = false;
					}
					prevRightDownTime = currentTime;
					break;
				}
				break;
			case WM_RBUTTONUP:
				{
					Time currentTime = getCurrentTime();
					//	チャタリングが発生しているか？
					if(isChattering(prevRightDownTime, currentTime)) {
						//	してるっぽいので何もしない
						callNext = false;
					}
					prevRightUpTime = currentTime;
					break;
				}
				break;

			case WM_MOUSEMOVE:
			case WM_MOUSEWHEEL:
			default:
				break;
			}
		}

		if(callNext) {
			return CallNextHookEx(hook, nCode, wParam, lParam);
		} else {
			return 1;
		}
	}

	HHOOK hook;
	static MouseHook* instance;

	Time	timeFrequency;
	Time	chatteringThreshold;
	Time	prevLeftDownTime;
	Time	prevLeftUpTime;
	Time	prevRightDownTime;
	Time	prevRightUpTime;


public:
	static void create(HINSTANCE hInstDll) {
		if(instance == 0) {
			instance = new MouseHook(hInstDll);
		}
	}

	static void setThreshold(float t) {
		if(instance) {
			instance->setChatteringThresholdTime(t);
		}
	}

	static void destroy() {
		if(instance) {
			delete instance;
			instance = 0;
		}
	}
};



MouseHook* MouseHook::instance = 0;



BOOL WINAPI DllMain(HINSTANCE hInstDLL, DWORD fdwReason, LPVOID) {
	switch (fdwReason) {
	case DLL_PROCESS_ATTACH:
		MouseHook::create(hInstDLL);
		MouseHook::setThreshold(1.0 / 32.0f);
		break;
    case DLL_PROCESS_DETACH:
		MouseHook::destroy();
		break;
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	default:
		break;
	}

	return TRUE;
}
