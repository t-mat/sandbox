#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <tchar.h>
#include <stdio.h>
#include <locale.h>


template<class> struct FindFilesData          {};
template<>      struct FindFilesData<char>    { using Type = WIN32_FIND_DATAA; };
template<>      struct FindFilesData<wchar_t> { using Type = WIN32_FIND_DATAW; };


template<class Char, class FindData>
class FindFilesIterator {
public:
    struct Body {
        FindData    findData {};
        HANDLE      handle   { INVALID_HANDLE_VALUE };
    };

public:
    FindFilesIterator()
        : pBody {}
    {}

    FindFilesIterator(const Char* filename, Body* pBody)
        : pBody { pBody }
    {
        setHandle(open(filename, getFindDataPtr()));
    }

    ~FindFilesIterator() {
        close();
    }

    FindData operator *() const {
        const auto* p = getFindDataPtr();
        if(p) {
            return *p;
        } else {
            return FindData {};
        }
    }

    const FindFilesIterator& operator ++ () {
        if(good()) {
            next();
        }
        return *this;
    }

    bool operator == (const FindFilesIterator& v) const {
        if(good() || v.good()) {
            return false;
        }
        return true;
    }

    bool operator != (const FindFilesIterator& v) const {
        return ! (*this == v);
    }

protected:
    static const HANDLE badHandle() {
        return INVALID_HANDLE_VALUE;
    }

    bool good() const {
        return badHandle() != getHandle();
    }

    void setHandle(HANDLE handle) {
        if(pBody) {
            pBody->handle = handle;
        }
    }

    HANDLE getHandle() const {
        if(pBody) {
            return pBody->handle;
        } else {
            return badHandle();
        }
    }

    FindData* getFindDataPtr() const {
        if(pBody) {
            return &pBody->findData;
        } else {
            return nullptr;
        }
    }

    static HANDLE open(const wchar_t* filename, WIN32_FIND_DATAW* pFindData) {
        return FindFirstFileW(filename, pFindData);
    }

    static HANDLE open(const char* filename, WIN32_FIND_DATAA* pFindData) {
        return FindFirstFileA(filename, pFindData);
    }

    static BOOL findNextFile(HANDLE hFindFile, WIN32_FIND_DATAW* pFindData) {
        return FindNextFileW(hFindFile, pFindData);
    }

    static BOOL findNextFile(HANDLE hFindFile, WIN32_FIND_DATAA* pFindData) {
        return FindNextFileA(hFindFile, pFindData);
    }

    void close() {
        if(good()) {
            FindClose(getHandle());
        }
        setHandle(badHandle());
    }

    void next() {
        if(good()) {
            if(! findNextFile(getHandle(), getFindDataPtr())) {
                close();
            }
        }
    }

protected:
    Body*       pBody;
};


template<
      class Char
    , class FindData = FindFilesData<Char>::Type
    , class Iterator = FindFilesIterator<Char, FindData>
    , class IteratorBody = Iterator::Body
>
class FindFilesT {
public:
    using iterator  = Iterator;

public:
    FindFilesT(const Char* filename)
        : body {}
        , itBegin { filename, &body }
    {}

    iterator begin() const { return itBegin; }
    iterator end() const { return iterator {}; }

private:
    IteratorBody    body;
    iterator        itBegin;
};


using FindFilesA = FindFilesT<char>;
using FindFilesW = FindFilesT<wchar_t>;

#if defined(_UNICODE)
using FindFiles = FindFilesW;
#else
using FindFiles = FindFilesA;
#endif


inline FindFilesA makeFindFiles(const char* str) {
    return FindFilesA {str};
}

inline FindFilesW makeFindFiles(const wchar_t* str) {
    return FindFilesW {str};
}


int main() {
    _tsetlocale(LC_ALL, _T(""));

    // TCHAR
    {
        const TCHAR* filename = _T("*.*");
        for(auto& wfd : makeFindFiles(filename)) {
            _tprintf(_T("[%s]\n"), wfd.cFileName);
        }
    }

    // char
    {
        const char* filename = "*.*";
        for(auto& wfd : makeFindFiles(filename)) {
            printf("[%s]\n", wfd.cFileName);
        }
    }

    // wchar_t
    {
        const wchar_t* filename = L"*.*";
        for(auto& wfd : makeFindFiles(filename)) {
            wprintf(L"[%s]\n", wfd.cFileName);
        }
    }
}
