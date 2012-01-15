// --------------------------------------------------------------------------
/**
	SetWindowsHookEx
	http://msdn.microsoft.com/library/ja/default.asp?url=/library/ja/jpipc/html/_win32_setwindowshookex.asp

	CallNextHookEx
	http://msdn.microsoft.com/library/ja/default.asp?url=/library/ja/jpipc/html/_win32_callnexthookex.asp

	http://bothack.wordpress.com/2007/12/12/keylogging-fun-for-cc-beginners/
 */
//---------------------------------------------------------------------------
#define _WIN32_WINNT 0x0501
#include <new>
#include <stdio.h>
#include <windows.h>
#include <tchar.h>
#pragma comment(lib, "winmm.lib")

#if !defined(_USRDLL)
#error This code must be compiled at 'Windows Dynamic Library (.dll)' mode.
#endif

#if !defined(_WIN64)
#error This code must be compiled at 'x64' project environment.
#endif

HHOOK	hook = 0;



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
class Entry {
protected:
	UINT	uTimerId;
	MSG		msg;
	DWORD	threadId;

	enum {
		TIMER_ID_UNUSED	= 0,
	};

public:
	void init() {
		uTimerId	= TIMER_ID_UNUSED;
	}

	UINT set(const MSG* pMsg, UINT wDelay, UINT wResolution, LPTIMECALLBACK lptc, DWORD dwUser, UINT fuCallbackType) {
		UINT ret = TIMER_ID_UNUSED;

		if(! isUsed()) {
			msg			= *pMsg;
			msg.lParam	|= 0x40000000;
			threadId	= GetCurrentThreadId();
			uTimerId	=
			ret			= timeSetEvent(wDelay, wResolution, lptc, dwUser, fuCallbackType);
		}

		return ret;
	}

	void unset() {
		if(isUsed()) {
			timeKillEvent(uTimerId);
			uTimerId = TIMER_ID_UNUSED;
		}
	}

	void forceUnset() {
		uTimerId = TIMER_ID_UNUSED;
	}

	BOOL postMessage() const {
		return PostMessage(msg.hwnd, msg.message, msg.wParam, msg.lParam);
	}

	bool isUsed() const {
		return uTimerId != TIMER_ID_UNUSED;
	}

	bool checkThread(DWORD currentThreadId) const {
		return currentThreadId == threadId;
	}

	DWORD getVirtualKey() const {
		return static_cast<DWORD>(msg.wParam);
	}
};



///////////////////////////////////////////////////////////////
class EntryPool {
	enum {
		ENTRY_MAX	= 100,
		BAD_INDEX	= -1,
	};

public:
	EntryPool() {
		initAll();
	}

	~EntryPool() {
		releaseAll();
	}

	void initAll() {
		for(int i = 0; i < ENTRY_MAX; ++i) {
			Entry* e = &entryArray[i];
			e->init();
		}
	}

	void releaseAll() {
		for(int i = 0; i < ENTRY_MAX; ++i) {
			Entry* e = &entryArray[i];
			e->unset();
		}
	}

	Entry* setEntry(const MSG* pMsg, UINT wDelay, LPTIMECALLBACK lptc) {
		Entry* p = 0;
		if(wDelay) {
			for(int i = 0; i < ENTRY_MAX; ++i) {
				Entry* e = &entryArray[i];
				if(! e->isUsed()) {
					p = e;
					p->set(pMsg, wDelay, wDelay, lptc, i, TIME_ONESHOT);
					break;
				}
			}
		}
		return p;
	}

	bool isValidIndex(int index) const {
		return index >= 0 && index < ENTRY_MAX;
	}

	int getEntryIndex(const Entry* p) const {
		int index = static_cast<int>(p - &entryArray[0]);
		if(! isValidIndex(index)) {
			index = BAD_INDEX;
		}
		return index;
	}

	Entry* getEntryByIndex(int index) {
		Entry* p = 0;
		if(isValidIndex(index)) {
			p = &entryArray[index];
		}
		return p;
	}

protected:
	Entry	entryArray[ENTRY_MAX];
};



///////////////////////////////////////////////////////////////
class CriticalSection {
public:
	CriticalSection() {
		InitializeCriticalSection(&cs);
	}

	~CriticalSection() {
		DeleteCriticalSection(&cs);
	}

	void enter() {
		EnterCriticalSection(&cs);
	}

	void leave() {
		LeaveCriticalSection(&cs);
	}

protected:
	CRITICAL_SECTION	cs;
};



///////////////////////////////////////////////////////////////
struct KbdAccConfig {
	int	uDelay;
	int	uDelay2;
	int	uRepeat;
};



///////////////////////////////////////////////////////////////
class KbdAcc {
	void init(HINSTANCE hInstDll) {
		{
			cfg.uDelay		= 200;
			cfg.uDelay2		= cfg.uDelay * 2;
			cfg.uRepeat		= 10;
			lastMsgTime		= 0;
			entryPool.initAll();

			if(hook == 0) {
				hook = SetWindowsHookEx(WH_GETMESSAGE, static_GetMsgProc, hInstDll, 0);
				if(hook == 0) {
					DWORD err = GetLastError();
					LPVOID lpMsgBuf;
					FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
								  0, err, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR) &lpMsgBuf, 0, 0);
					MessageBox(0, (LPCTSTR)lpMsgBuf, _T("x64_kbdacc : SetWindowsHookEx() Install Error"), MB_OK | MB_ICONINFORMATION);
					LocalFree(lpMsgBuf);
				}
			}
		}
	}

	void cleanup() {
		releaseAllTimer();
		if(hook != 0) {
			UnhookWindowsHookEx(hook);
			hook = 0;
		}
		PostMessage(HWND_BROADCAST, 0, 0, 0);
	}

	void timerCallback(UINT uTimerID, UINT uMsg, DWORD_PTR dwUser, DWORD_PTR dw1, DWORD_PTR dw2) {
		int index = static_cast<int>(dwUser);
		Entry* p = entryPool.getEntryByIndex(index);
		if(p) {
			p->forceUnset();
			timerCs.enter();
			HWND foregroundWindow = GetForegroundWindow();
			if(foregroundWindow != 0) {
				DWORD threadId = GetWindowThreadProcessId(foregroundWindow, 0);
				if(p->checkThread(threadId)) {
					short ks = GetAsyncKeyState(p->getVirtualKey());
					if(ks < 0) {
						if(GetQueueStatus(QS_KEY) == 0) {
							p->postMessage();
						}
					}
				}
			}

			timerCs.leave();
		}
	}

	static void CALLBACK lptc(UINT uTimerID, UINT uMsg, DWORD_PTR dwUser, DWORD_PTR dw1, DWORD_PTR dw2) {
		if(instance) {
			return instance->timerCallback(uTimerID, uMsg, dwUser, dw1, dw2);
		}
	}

	void onKeyDown(const MSG* msg) {
		keyDownCs.enter();
		releaseAllTimer();

		//	
		//	lParam
		//	The repeat count, scan code, extended-key flag, context code,
		//	previous key-state flag, and transition-state flag, as shown following.
		//
		//	Bits	Meaning
		//	0-15	The repeat count for the current message. The value is the
		//			number of times the keystroke is autorepeated as a result
		//			of the user holding down the key. If the keystroke is held
		//			long enough, multiple messages are sent.
		//			However, the repeat count is not cumulative.
		//	16-23	The scan code. The value depends on the OEM.
		//	24		Indicates whether the key is an extended key, such as the
		//			right-hand ALT and CTRL keys that appear on an enhanced
		//			101- or 102-key keyboard.
		//			The value is 1 if it is an extended key; otherwise, it is 0.
		//	25-28	Reserved; do not use.
		//	29		The context code. The value is always 0 for a WM_KEYDOWN message.
		//	30		The previous key state. The value is 1 if the key is down
		//			before the message is sent, or it is zero if the key is up.
		//	31		The transition state. The value is always 0 for a WM_KEYDOWN message.
		switch(msg->wParam) {
		case VK_SHIFT:			//	0x10
		case VK_CONTROL:		//	0x11
		case VK_MENU:			//	0x12
		case VK_OEM_COPY:		//	0xf2	Japanese keyboard "KATAKANA, HIRAGANA, ROMAJI" key (When IME is OFF)
			//	do not auto repeat
			break;
		default:
			{
				bool firsttime = false;
				if((msg->lParam & 0x40000000) == 0) {
					//	up -> down state
				//	if((int) (msg->time - lastMsgTime) <= cfg.uDelay2)
					{
						firsttime = true;
					}
				}
				if(firsttime) {
					//	first stroke
					entryPool.setEntry(msg, cfg.uDelay, lptc);
				} else {
					//	repeat
					entryPool.setEntry(msg, cfg.uRepeat, lptc);
				}
			}
			break;
		}

		lastMsgTime = msg->time;
		keyDownCs.leave();
	}

	void releaseAllTimer() {
		entryPool.releaseAll();
	}

	static LRESULT CALLBACK static_GetMsgProc(int code, WPARAM wParam, LPARAM lParam) {
		if(code < 0) {
			CallNextHookEx(0, code, wParam, lParam);
			return 0;
		} else {
			if(code == HC_ACTION && wParam == PM_REMOVE) {
				const MSG* msg = reinterpret_cast<const MSG*>(lParam);

				if(instance) {
					switch(msg->message) {
					default:
						break;
					case WM_KEYUP:
					case WM_SYSKEYUP:
						instance->releaseAllTimer();
						break;
					case WM_KEYDOWN:
					case WM_SYSKEYDOWN:
						instance->onKeyDown(msg);
						break;
					}
				}
			}
			return CallNextHookEx(0, code, wParam, lParam);
		}
	}

public:
	static void attach(HINSTANCE hInstDll) {
		if(instance == 0) {
			instance = new KbdAcc;
			instance->init(hInstDll);
		}
	}

	static void detach() {
		if(instance) {
			instance->cleanup();
			delete instance;
			instance = 0;
		}
	}

protected:
	EntryPool		entryPool;
	CriticalSection	keyDownCs;
	CriticalSection timerCs;
	CriticalSection	detachCs;
	DWORD			lastMsgTime;
	KbdAccConfig	cfg;
	static KbdAcc*	instance;
};

KbdAcc*	KbdAcc::instance	= 0;



///////////////////////////////////////////////////////////////
int checkTarget() {
#if 1
	return 0;
#else
	int ret = -1;

	static const TCHAR* exceptionNames[] = {
		_T("xyz"),
		_T("note"),
		_T("afx"),
		_T("x64_kbdacc"),
	};

	TCHAR	name[512] = { 0 };
	DWORD	e = GetModuleFileName(0, &name[0], sizeof(name)/sizeof(name[0]));
	if(e != 0) {
		for(int i = 0; i < sizeof(exceptionNames)/sizeof(exceptionNames[0]); ++i) {
			const TCHAR* t = exceptionNames[i];
			TCHAR* p = _tcsstr(name, t);
			if(p) {
				ret = i;
				break;
			}
		}
	}

	return ret;
#endif
}



///////////////////////////////////////////////////////////////
BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID) {
	int ti = checkTarget();
	if(ti < 0) {
		return FALSE;
	} else {
		switch(fdwReason) {
		case DLL_PROCESS_ATTACH:
			{
				TCHAR	name[512] = { 0 };
				DWORD	e = GetModuleFileName(0, &name[0], sizeof(name)/sizeof(name[0]));
				debugOut(_T("x64 kbdacc - attach : [%s]\n"), name);
			}
			KbdAcc::attach(hinstDLL);
			break;

		case DLL_PROCESS_DETACH:
			{
				TCHAR	name[512] = { 0 };
				DWORD	e = GetModuleFileName(0, &name[0], sizeof(name)/sizeof(name[0]));
				debugOut(_T("x64 kbdacc - detach : [%s]\n"), name);
			}
			KbdAcc::detach();
			break;

		case DLL_THREAD_ATTACH:
		case DLL_THREAD_DETACH:
		default:
			break;
		}
		return TRUE;
	}
}
