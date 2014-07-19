// http://blogs.msdn.com/b/oldnewthing/archive/2007/10/08/5351207.aspx
// http://icodeanswer.appspot.com/post/210504
// http://stackoverflow.com/questions/210504/enumerate-windows-like-alt-tab-does

#define WIN32_LEAN_AND_MEAN
#define _WIN32_WINNT 0x0601
#include <windows.h>
#include <tchar.h>
#include <vector>
#include <locale.h>


static HWND getLastVisibleActivePopUpOfWindow(HWND hwnd) {
    for(;;) {
        const HWND h = GetLastActivePopup(hwnd);
        if(IsWindowVisible(h)) {
            return h;
        } else if (h == hwnd) {
            return NULL;
        }
        hwnd = h;
    }
}


static bool isAltTabWindow(HWND hwnd) {
    if(GetShellWindow() == hwnd) {
        return false;
    }

    const LONG ex = GetWindowLong(hwnd, GWL_EXSTYLE);
    if(ex & WS_EX_TOOLWINDOW) {
        return false;
    }

    const HWND hRoot = GetAncestor(hwnd, GA_ROOTOWNER);
    const HWND hLast = getLastVisibleActivePopUpOfWindow(hRoot);
    if(hLast != hwnd) {
        return false;
    }

    return true;
}


using Hwnds = std::vector<HWND>;

// NOTE : Order of Hwnds are same as ALT-TAB list.
// NOTE : Also, Ascending Z order of Hwnds are same as ALT-TAB list.
static Hwnds enumAltTabWindows() {
    Hwnds hwnds;

    EnumWindows([](HWND h, LPARAM lParam) -> BOOL {
        Hwnds& hs = *reinterpret_cast<Hwnds*>(lParam);
        if(isAltTabWindow(h)) {
            hs.push_back(h);
        }
        return TRUE;
    }, reinterpret_cast<LPARAM>(&hwnds));

    return hwnds;
}


static int getZorder(HWND hwnd) {
    int z = 0;
    for(auto h = hwnd; h != NULL; h = GetWindow(h, GW_HWNDPREV)) {
        ++z;
    }
    return z;
}


int main() {
    _tsetlocale(LC_ALL, _T(""));

    const Hwnds hwnds = enumAltTabWindows();
    for(const HWND hwnd : hwnds) {
        TCHAR className[256] {};
        GetClassName(hwnd, className, _countof(className));

        TCHAR text[256] {};
        GetWindowText(hwnd, text, _countof(text));

        int zorder = getZorder(hwnd);

        _tprintf(_T("HWND=%p, z=%5d, class=[%s], caption=[%s]\n"), hwnd, zorder, className, text);
    }

    return 0;
}
