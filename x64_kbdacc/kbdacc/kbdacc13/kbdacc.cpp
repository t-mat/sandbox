
使用している API 一覧

	GetCurrentThreadId
	InterlockedIncrement
	CallNextHookEx
	PostMessageA
	GetQueueStatus
	SetWindowsHookExA
	GetAsyncKeyState
	GeyWindowThreadProcessId
	GetForegroundWindow
	UnhookWindowsHookEx
	timeKillEvent
	timeSetEvent

struct MSG {
	HWND	hwnd;		// +0	u32		0x1000301c
	UINT	message;	// +4	u32		0x10003020
	WPARAM	wParam;		// +8	u32		0x10003024
	LPARAM	lParam;		// +12	u32		0x10003028
	DWORD	time;		// +16	u32		0x1000302c
	POINT	pt;			// +20	u64		0x10003030
};						// +28	

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
struct Entry {
	UINT	uTimerID;	// 0x10003018
	MSG		msg;		// 0x1000301c
	DWORD	threadId;	// 0x10003038
};

static Entry	entryArray[100];		// 0x10003018


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
BOOL __stdcall DllEntryPoint(HINSTANCE hintDLL, DWORD fdwReason, LPVOID lpReserved) {
	switch(fdwReason) {
	case DLL_PROCESS_DETACH: // (0)
		if(ds:hhk == 0) {
			sub_100012f0();
		}
		break;
	case DLL_PROCESS_ATTACH: // (1)
		hmod = hinstDLL;
		break;
	default:
	case DLL_THREAD_ATTACH:  // (2)
	case DLL_THREAD_DETACH:  // (3)
	}
	return TRUE;
	break;
}

void sub_100012f0() {
	char* esi = (char*) &dword_10003018;
	for(;;) {
		int* p = (int*) esi;
		if(*p != 0) {
			timeKillEvent(*p);
			*p = 0;
		}
		esi += 0x24;
		if(esi >= &dword_10003e28) {
			// 100 回ループしたら終わる
			break;
		}
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static int		Addend			= 0;
static HWND*	dword_1000501c	= 0;
static HHOOK	ds:hhk			= 0;

int __stdcall install_hook(HWND* arg_0, HWND** arg_4) {
	int ret = 0;

	if(InterlockedIncrement(&Addend) == 0) {
		ds:dword_1000501c	= arg_0;

		DWORD		arg_dwThreadId	= 0;
		HINSTANCE	arg_hmod		= hmod;
		HOOKPROC	arg_lpfn		= fn;
		int			arg_idHook		= WH_GETMESSAGE; // 3
		HHOOK		hh				= SetWindowsHookExA(arg_idHook, arg_lpfn, arg_hmod, arg_dwThreadId);
		ds::hhk	= hh;
		ret	= hh;
	} else {
		*arg_4 = ds:dword_1000501c
	}

	return ret;
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void __stdcall uninstall_hook() {
	if(ds:hhk != 0) {
		DWORD		arg_hhook	= ds:hhk;
		UnhookWindowsHookEx(arg_hhook);
		ds:hhk = 0;

		DWORD		arg_lParam	= 0;
		DWORD		arg_wParam	= 0;
		DWORD		arg_Msg		= 0;
		DWORD		arg_hWnd	= HWND_BROADCAST;	//0x0000ffff;

		PostMesasgeA(arg_hWnd, arg_Msg, arg_wParam, arg_lParam);
	}
}


struct kbdacc_conf {
	int	d0;	// 10005010
	int	d1;	// 10005014
	int	d2;	// 10005018
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void __stdcall setconf(struct kbdacc_conf const& kc) {
	dword_10005010	= kc;
}

void __stdcall getconf(struct kbdacc_conf& kc) {
	kc = dword_10005010;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static DWORD	lastMsgTime	= 0;		// dword_10003008

LRESULT __stdcall fn(int nCode, WPARAM wParam, LPARAM lParam) {
//	DWORD	dwUser;
	int	var_8;

	if(nCode != HC_ACTION) {	// HC_ACTION = 0
		goto loc_10001288;
	}

	if(wParam != PM_REMOVE) {	// PM_REMOVE = 1
		goto loc_10001288;
	}

	cosnt MSG* msg = (const MSG*) lParam;

	ebx = lParam;

	switch(msg->message) {
	default:
		break;
	case WM_KEYDOWN:	// 0x0100	-> loc_1000108d
	case WM_SYSKEYDOWN:	// 0x0104	-> loc_1000108d
		//	loc_1000108d
		edi				= dword_10005008;
		dword_10005008	= GetCurrentThreadId();

		if((dword_10005010 & 0xff) == 1) {	//	mov ecx, dword_10005010; test	cl, 1
			break;							//	jz		loc_10001288
		}

		if(dword_10003010 == 0) {	// eax = dword_10003010; test eax, eax; jnz loc_100010bd
			sub_100012f0();			// 全タイマリリース
		}

		// loc_100010bd
		{
			int eax = msg->wParam;	// mov eax, [ebx+8]
			if(eax >= 0x10) {
				if(eax <= 0x12) {	// cmp	eax, 0x12
					//	0x10	VK_SHIFT	(SHIFT)
					//	0x11	VK_CONTROL	(CTRL)
					//	0x12	VK_MENU		(ALT)
					break;			// jbe	loc_10001288
				}
				if(eax == 0xf2) {	// cmp	eax, 0xf2
					//	0xf2	VK_OEM_COPY	Japanese keyboard "KATAKANA, HIRAGANA, ROMAJI" (When IME is OFF)
					break;			// jz	loc_10001288
				}
			}
		}

		// loc_100010d9
		if(edi == ds:dword_10005008				// mov eax, dword_10005008; cmp edi, eax; jnz loc_10001200
		&& (msg->lParam & 0x40000000) != 0		// mov eax, [ebx+0x0c]; test eax, 0x40000000; jz loc_10001200
		) {
			{
				int eax = uDelay;
				if(eax != 0) {
					eax += eax;
				} else {
					//	loc_10001101
					eax = 0xfa;
				}
				//	loc_10001106
				if(msg->time - lastMsgTime > eax) {
					goto loc_10001200;
				}
			}
			int dwUser = sub_100012d0();			// エントリから空きを探す
			if(dwUser >= 0) {
				// 空きがあった
#if 1
				Entry*	e	= &entryArray[dwUser];
				e->msg		= *msg;
				e->threadId	= GetCurrentThreadId();

				...

				unsigned int esi = (dword_10005010 & 0x000007f0) >> 4;
				unsigned int edi = (dword_10005010 & 0x0003f800) >> 0x0b;
				unsigned int ecx = (dword_10005010 & 0x01fc0000) >> 0x12;
				unsigned int eax = (dword_10005010             ) >> 0x19;
				unsigned int edx = 0;
				unsigned int var_8 = (eax * esi) / 100;
				if(edi != 0) {
					eax = dword_1000300c;
					edx:eax = eax;			// cdq
					edx:eax /= edi;
					eax *= ecx;
					eax *= esi;
					edx:eax = eax;			// cdq
					ecx = 0x0ffffff9c;
					edx:eax /= ecx;
					esi += eax;
					eax = var_8;
					if(eax >= esi) {
						esi = eax;
					}
				}
				//	loc_100011b8
				if(esi < 0) {
					eax = dword_10005010;
					esi = 1;
					if((eax & 0xff) != 2) {
						dword_10003010 = 1;
						esi = 0x0a;
					}
				}

				//	loc_100011d9
				eax = dwUser;
				UINT			arg_wDelay			= esi;
				UINT			arg_wResolution		= esi;
				LPTIMECALLBACK	arg_lptc			= fptc;
				DWORD			arg_dwUser			= eax;
				UINT			arg_fuCallbackType	= TIME_ONESHOT;	// 0

				e->uTimerID	= timeSetEvent(arg_wDelay, arg_wResolution, arg_lptc, arg_dwUser, arg_fuCallbackType);
#else
				int eax = dwUser * 4;
				int ebp = eax * 9;
				edi = dword_1000301c + ebp;
				eax = GetCurrentThreadId();
				esi = lParam;
				ecx = 7;
				* (DWORD*) (ss:dword_10003038+ebp) = eax;
				while(ecx != 0) {					// rep movsd
					*(DWORD*) edi = *(DWORD*) esi;
					edi += 4;
					esi += 4;
					ecx -= 1;
				}

				esi = dword_10005010;
				ecx = dword_10005010;
				esi &= 0x07f0;
				ecx &= 0x01fc0000;
				esi >>= 4;
				edi = dword_10005010;
				ecx >= 0x12;
				edi &= 0x0003f800;
				edi >>= 0x0b;
				eax = dword_10005010;
				eax >>= 0x19;
				edx = 0;
				var_8 = 0x64;
				eax *= esi;
				var_8 = eax / var_8;
				if(edi != 0) {
					eax = dword_1000300c;
					edx:eax = eax;			// cdq
					edx:eax /= edi;
					eax *= ecx;
					eax *= esi;
					edx:eax = eax;			// cdq
					ecx = 0x0ffffff9c;
					edx:eax /= ecx;
					esi += eax;
					eax = var_8;
					if(eax >= esi) {
						esi = eax;
					}
				}
				if(esi < 0) {
					eax = dword_10005010;
					esi = 1;
					if(eax != 2) {
						dword_10003010 = 1;
						es = 0x0a;
					}
				}
				eax = dwUser;
				UINT			arg_wDelay			= esi;
				UINT			arg_wResolution		= esi;
				LPTIMECALLBACK	arg_lptc			= fptc;
				DWORD			arg_dwUser			= eax;
				UINT			arg_fuCallbackType	= TIME_ONESHOT;	// 0
				eax = timeSetEvent(arg_wDelay, arg_wResolution, arg_lptc, arg_dwUser, arg_fuCallbackType);
				ss:dword_10003018 = eax;
#endif
			}
			// loc_100011f3
			eax	= dword_1000300c;
			eax += 1;
			dword_1000300c = eax;
			goto loc_10001279;
		} else {
loc_10001200:
			dword_1000300c	= 0;
			dword_10003010	= 0;
			if(uDelay != 0) {				// test eax, eax; jz loc_10001279
				var_8 = sub_100012d0();
				if(var_8 >= 0) {			// test eax, eax; jl loc_10001279
					eax = var_8 * 4;
					ebp = eax * 9;
					edi = dword_1000301c + eax * 9;
					eax = GetCurrentThreadId();
					esi = lParam;
					ecx = 7;
					dword_10003038+ebp = eax;
					while(ecx != 0) {					// rep movsd
						*(DWORD*) edi = *(DWORD*) esi;
						edi += 4;
						esi += 4;
						ecx -= 1;
					}

					ecx = dword_10003028 + ebp;
					ecx |= 0x40000000;
					dword_10003028 + ebp = ecx;

					eax = uDelay;
					edx = var_8;

					UINT			arg_wDelay			= eax;
					UINT			arg_wResolution		= eax;
					LPTIMECALLBACK	arg_lptc			= fptc;
					DWORD			arg_dwUser			= edx;
					UINT			arg_fuCallbackType	= TIME_ONESHOT;	// 0
					eax = timeSetEvent(arg_wDelay, arg_wResolution, arg_lptc, arg_dwUser, arg_fuCallbackType);
					ss:dword_10003018 = eax;
				}
			}
		}

		//	loc_10001279
		lastMsgTime	= msg->time;
		break;	// jmp loc_10001288

	case WM_KEYUP:		// 0x0101	-> loc_10001283
	case WM_SYSKEYUP:	// 0x0105	-> loc_10001283
		//	loc_10001283
		sub_100012f0();
		break;
	}

	// loc_10001288
	HHOOK	arg_hhk		= lParam;
	int		arg_nCode	= nCode;
	WPARAM	arg_wParam	= wParam;
	LPARAM	arg_lParam	= lParam;

	return	CallNextHookEx(arg_hhk, arg_nCode, arg_wParam, arg_lParam);
}

// 空きを探す
int sub_100012d0() {
	int		eax = 0;
	char*	ecx = (char*) &dword_10003018;

	for(;;) {
		int edx = (int*) ecx;
		if(edx == 0) {
			break;
		}
		eax += 1;

		ecx += 0x24;
		if(ecx >= &dword_10003e28) {
			// 100 回ループしたら終わる
			eax = -1;
			break;
		}
	}

	return eax;
}



///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CALLBACK fptc(UINT uTimerID, UINT uMsg, DWORD_PTR dwUser, DWORD_PTR dw1, DWORD_PTR dw2) {
	DWORD arg_8	= dwUser;

	eax	= arg_8;
	if(eax < 100) {	
		eax	*= 4;
		ecx = dword_10003004;
		esi	= eax * 9;
		* (DWORD*) (dword_10003018 + esi) = 0;

		if(ecx == 0) {
			dword_10003004 = 1;
			eax = GetForegroundWindow();
			if(eax != 0) {
				HWND	arg_hWnd			= eax;
				LPDWORD	arg_lpdwProcessId	= 0;
				eax = GetWindowThreadProcessId(arg_hWnd, arg_lpdwProcessId);
				if(eax == *(DWORD*)(esi+dword_10003038)) {		//	threadId
					eax = *(DWORD*)(esi+dword_10003024);		//	wParam
					eax = GetAsyncKeyState(eax);
					if((short)eax < 0) {
						eax = GetQueueStatus(QS_KEY);	// QS_KEY = 1
						if(eax == 0) {
							HWND	arg_hWnd	= *(DWORD*)(esi+dword_1000301c);
							UINT	arg_uMsg	= *(DWORD*)(esi+dword_10003020);
							WPARAM	arg_wParam	= *(DWORD*)(esi+dword_10003024);
							LPDWORD	arg_lParam	= *(DWORD*)(esi+dword_10003028);
							PostMessage(arg_hWnd, arg_uMsg, arg_wParam, arg_lParam);
						}
					}
				}
			}
			// loc_100013ae
			dword_10003004 = 0;
		}
	}

	//	loc_100013b8
}
