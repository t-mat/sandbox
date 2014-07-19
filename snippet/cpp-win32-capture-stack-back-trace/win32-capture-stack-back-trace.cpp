// CaptureStackBackTrace
// http://msdn.microsoft.com/en-us/library/windows/desktop/bb204633(v=vs.85).aspx
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>

//////////////////////////////////////////////////////////////
void capture() {
    const ULONG framesToSkip = 0;
    const ULONG framesToCapture = 64;
    void* backTrace[framesToCapture] {};
    ULONG backTraceHash = 0;

    const USHORT nFrame = CaptureStackBackTrace(
          framesToSkip
        , framesToCapture
        , backTrace
        , &backTraceHash
    );

    for(USHORT iFrame = 0; iFrame < nFrame; ++iFrame) {
        printf("[%3d] = %p\n", iFrame, backTrace[iFrame]);
    }
    printf("backTraceHash = %08x\n", backTraceHash);
}

//////////////////////////////////////////////////////////////
int f(int x);
int g(int x);
const int xmax = 8;

int f(int x) {
    if(x >= xmax) {
        capture();
        return x;
    }
    return g(x + 1);
}

int g(int x) {
    return f(x + 1);
}

int main() {
    f(0);
}
