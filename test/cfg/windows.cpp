
// Test library configuration for windows.cfg
//
// Usage:
// $ cppcheck --check-library --library=windows --enable=information --error-exitcode=1 --inline-suppr --suppress=missingIncludeSystem test/cfg/windows.cpp
// =>
// No warnings about bad library configuration, unmatched suppressions, etc. exitcode=0
//

#include <windows.h>

void validCode()
{
    DWORD dwordInit = 0;
    WORD wordInit = 0;
    BYTE byteInit = 0;

    // Valid Semaphore usage, no leaks, valid arguments
    HANDLE hSemaphore1;
    hSemaphore1 = CreateSemaphore(NULL, 0, 1, NULL);
    CloseHandle(hSemaphore1);
    HANDLE hSemaphore2;
    hSemaphore2 = CreateSemaphoreEx(NULL, 0, 1, NULL, 0, SEMAPHORE_ALL_ACCESS);
    CloseHandle(hSemaphore2);
    HANDLE hSemaphore3;
    hSemaphore3 = OpenSemaphore(SEMAPHORE_ALL_ACCESS, TRUE, "sem");
    CloseHandle(hSemaphore3);

    // Valid lstrcat usage, but with warning because it is deprecated
    char buf[30] = "hello world";
    // cppcheck-suppress lstrcatCalled
    lstrcat(buf, "test");

    // cppcheck-suppress strlwrCalled
    strlwr(buf);
    // cppcheck-suppress struprCalled
    strupr(buf);

    // Valid Mutex usage, no leaks, valid arguments
    HANDLE hMutex1;
    hMutex1 = CreateMutex(NULL, TRUE, NULL);
    if (hMutex1) {
        ReleaseMutex(hMutex);
    }
    CloseHandle(hMutex1);
    HANDLE hMutex2;
    hMutex2 = CreateMutexEx(NULL, NULL, 0, MUTEX_ALL_ACCESS);
    CloseHandle(hMutex2);
    HANDLE hMutex3;
    hMutex3 = OpenMutex(MUTEX_ALL_ACCESS, FALSE, "sem");
    CloseHandle(hMutex3);

    // Valid Module usage, no leaks, valid arguments
    HMODULE hModule = GetModuleHandle(L"My.dll");
    FreeLibrary(hModule);
    hModule = GetModuleHandle(TEXT("somedll"));
    FreeLibrary(hModule);
    hModule = GetModuleHandle(NULL);
    FreeLibrary(hModule);

    // Valid Event usage, no leaks, valid arguments
    HANDLE event;
    event = CreateEvent(NULL, FALSE, FALSE, NULL);
    if (NULL != event) {
        SetEvent(event);
        CloseHandle(event);
    }
    event = OpenEvent(EVENT_ALL_ACCESS, FALSE, L"testevent");
    if (NULL != event) {
        PulseEvent(event);
        SetEvent(event);
        CloseHandle(event);
    }
    event = CreateEventEx(NULL, L"testevent3", CREATE_EVENT_INITIAL_SET, EVENT_MODIFY_STATE);
    if (NULL != event) {
        ResetEvent(event);
        CloseHandle(event);
    }

    void *pMem1 = _malloca(1);
    _freea(pMem1);
    // Memory from _alloca must not be freed
    void *pMem2 = _alloca(10);
    memset(pMem2, 0, 10);

    SYSTEMTIME st;
    GetSystemTime(&st);

    DWORD lastError = GetLastError();
    SetLastError(lastError);

    PSID pEveryoneSID = NULL;
    SID_IDENTIFIER_AUTHORITY SIDAuthWorld = SECURITY_WORLD_SID_AUTHORITY;
    AllocateAndInitializeSid(&SIDAuthWorld, 1, SECURITY_WORLD_RID, 0, 0, 0, 0, 0, 0, 0, &pEveryoneSID)
    FreeSid(pEveryoneSID);

    LPVOID pMem = HeapAlloc(GetProcessHeap(), 0, 10);
    pMem = HeapReAlloc(GetProcessHeap(), 0, pMem, 0);
    HeapFree(GetProcessHeap(), 0, pMem);

    char bufC[50];
    sprintf_s(bufC, "Hello");
    printf("%s", bufC);
    sprintf_s(bufC, "%s", "test");
    printf("%s", bufC);
    sprintf_s(bufC, _countof(bufC), "%s", "test");
    printf("%s", bufC);
    wchar_t bufWC[50];
    swprintf_s(bufWC, L"Hello");
    wprintf(L"%s\n", bufWC);
    swprintf_s(bufWC, L"%s %d", L"swprintf_s", 3);
    wprintf(L"%s\n", bufWC);
    swprintf_s(bufWC, _countof(bufWC), L"%s %d", L"swprintf_s", 6);
    wprintf(L"%s\n", bufWC);
    TCHAR bufTC[50];
    _stprintf(bufTC, TEXT("Hello"));
    _tprintf(TEXT("%s"), bufTC);
    _stprintf(bufTC, TEXT("%d"), 1);
    _tprintf(TEXT("%s"), bufTC);
    _stprintf(bufTC, _countof(bufTC), TEXT("%d"), 2);
    _tprintf(TEXT("%s"), bufTC);

    GetUserName(NULL, &dwordInit);
    dwordInit = 10;
    GetUserName(bufTC, _countof(bufTC));

    WSADATA wsaData = {0};
    WSAStartup(2, &wsaData);
    SOCKET sock = socket(1, 2, 3);
    u_long ulongInit = 0;
    ioctlsocket(sock, FIONBIO, &ulongInit);
    if (sock != INVALID_SOCKET) {
        closesocket(sock);
    }
    WSACleanup();

    wordInit = MAKEWORD(1, 2);
    // cppcheck-suppress redundantAssignment
    dwordInit = MAKELONG(1, 2);
    // cppcheck-suppress redundantAssignment
    wordInit = LOWORD(dwordInit);
    byteInit = LOBYTE(wordInit);
    wordInit = HIWORD(dwordInit);
    // cppcheck-suppress redundantAssignment
    byteInit = HIBYTE(wordInit);
    if (byteInit) {}

    bool boolVar;
    uint8_t byteBuf[5] = {0};
    uint8_t byteBuf2[10] = {0};
    boolVar = RtlEqualMemory(byteBuf, byteBuf2, sizeof(byteBuf));
    if (boolVar) {}
    boolVar = RtlCompareMemory(byteBuf, byteBuf2, sizeof(byteBuf));
    if (boolVar) {}
    RtlMoveMemory(byteBuf, byteBuf2, sizeof(byteBuf));
    RtlCopyMemory(byteBuf, byteBuf2, sizeof(byteBuf));
    RtlZeroMemory(byteBuf, sizeof(byteBuf));
    ZeroMemory(byteBuf, sizeof(byteBuf));
    RtlSecureZeroMemory(byteBuf, sizeof(byteBuf));
    SecureZeroMemory(byteBuf, sizeof(byteBuf));
    RtlFillMemory(byteBuf, sizeof(byteBuf), 0xff);

    // cppcheck-suppress LocalAllocCalled
    HLOCAL pLocalAlloc = LocalAlloc(1, 2);
    LocalFree(pLocalAlloc);

    // cppcheck-suppress lstrlenCalled
    (void)lstrlen(bufTC);
    // cppcheck-suppress lstrlenCalled
    (void)lstrlen(NULL);

    // Intrinsics
    __noop();
    __noop(1, "test", NULL);
    __nop();

    // Valid Library usage, no leaks, valid arguments
    HINSTANCE hInstLib = LoadLibrary(L"My.dll");
    FreeLibrary(hInstLib);
    hInstLib = LoadLibraryA("My.dll");
    FreeLibrary(hInstLib);
    hInstLib = LoadLibraryEx(L"My.dll", NULL, 0);
    FreeLibrary(hInstLib);
    hInstLib = LoadLibraryExW(L"My.dll", NULL, 0);
    FreeLibrary(hInstLib);
    hInstLib = ::LoadLibrary(L"My.dll");
    FreeLibraryAndExitThread(hInstLib, 0); // Does not return! Must be at the end!
}

void bufferAccessOutOfBounds()
{
    wchar_t buf[10];
    // Verifying _countof macro configuration
    // Valid loop over array
    for (size_t i = 0; i < _countof(buf); ++i) {
        buf[i] = L'\0';
    }
    // Wrong loop over array accessing one element past the end
    for (size_t i = 0; i <= _countof(buf); ++i) {
        // cppcheck-suppress arrayIndexOutOfBounds
        buf[i] = L'\0';
    }

    uint8_t byteBuf[5] = {0};
    uint8_t byteBuf2[10] = {0};
    // TODO ticket #8412 cppcheck-suppress ignoredReturnValue
    // cppcheck-suppress bufferAccessOutOfBounds
    RtlEqualMemory(byteBuf, byteBuf2, 20);
    // cppcheck-suppress ignoredReturnValue
    // cppcheck-suppress bufferAccessOutOfBounds
    RtlCompareMemory(byteBuf, byteBuf2, 20);
    // cppcheck-suppress bufferAccessOutOfBounds
    RtlMoveMemory(byteBuf, byteBuf2, 20);
    // TODO cppcheck-suppress redundantCopy
    // cppcheck-suppress bufferAccessOutOfBounds
    MoveMemory(byteBuf, byteBuf2, 20);
    // TODO cppcheck-suppress redundantCopy
    // cppcheck-suppress bufferAccessOutOfBounds
    RtlCopyMemory(byteBuf, byteBuf2, 20);
    // TODO cppcheck-suppress redundantCopy
    // cppcheck-suppress bufferAccessOutOfBounds
    CopyMemory(byteBuf, byteBuf2, 20);
    // cppcheck-suppress bufferAccessOutOfBounds
    RtlZeroMemory(byteBuf, sizeof(byteBuf)+1);
    // cppcheck-suppress bufferAccessOutOfBounds
    ZeroMemory(byteBuf, sizeof(byteBuf)+1);
    // cppcheck-suppress bufferAccessOutOfBounds
    RtlSecureZeroMemory(byteBuf, sizeof(byteBuf)+1);
    // cppcheck-suppress bufferAccessOutOfBounds
    SecureZeroMemory(byteBuf, sizeof(byteBuf)+1);
    // cppcheck-suppress bufferAccessOutOfBounds
    RtlFillMemory(byteBuf, sizeof(byteBuf)+1, 0x01);
    // cppcheck-suppress bufferAccessOutOfBounds
    FillMemory(byteBuf, sizeof(byteBuf)+1, 0x01);
}

void nullPointer()
{
    HANDLE hSemaphore;
    // cppcheck-suppress nullPointer
    hSemaphore = OpenSemaphore(SEMAPHORE_ALL_ACCESS, FALSE, NULL);
    CloseHandle(hSemaphore);

    // cppcheck-suppress lstrcatCalled
    // cppcheck-suppress nullPointer
    lstrcat(NULL, "test");
    char buf[10] = "\0";
    // cppcheck-suppress lstrcatCalled
    // cppcheck-suppress nullPointer
    lstrcat(buf, NULL);

    HANDLE hMutex;
    // cppcheck-suppress nullPointer
    hMutex = OpenMutex(MUTEX_ALL_ACCESS, FALSE, NULL);
    CloseHandle(hMutex);

    //Incorrect: 1. parameter, must not be null
    // cppcheck-suppress nullPointer
    FARPROC pAddr = GetProcAddress(NULL, "name");
    HMODULE * phModule = NULL;
    // cppcheck-suppress nullPointer
    GetModuleHandleEx(0, NULL, phModule);

    // cppcheck-suppress leakReturnValNotUsed
    // cppcheck-suppress nullPointer
    OpenEvent(EVENT_ALL_ACCESS, FALSE, NULL);
    HANDLE hEvent = NULL;
    // cppcheck-suppress nullPointer
    PulseEvent(hEvent);
    // cppcheck-suppress nullPointer
    ResetEvent(hEvent);
    // cppcheck-suppress nullPointer
    SetEvent(hEvent);

    char *str = NULL;
    // cppcheck-suppress strlwrCalled
    // cppcheck-suppress nullPointer
    strlwr(str);
    // cppcheck-suppress struprCalled
    // cppcheck-suppress nullPointer
    strupr(str);

    // cppcheck-suppress nullPointer
    GetSystemTime(NULL);
    // cppcheck-suppress nullPointer
    GetLocalTime(NULL);

    // TODO: error message: arg1 must not be nullptr if variable pointed to by arg2 is not 0
    DWORD dwordInit = 10;
    GetUserName(NULL, &dwordInit);
    TCHAR bufTC[10];
    // cppcheck-suppress nullPointer
    GetUserName(bufTC, NULL);

    SOCKET socketInit = {0};
    sockaddr sockaddrUninit;
    int intInit = 0;
    int *pIntNull = NULL;
    char charArray[] = "test";
    // cppcheck-suppress nullPointer
    WSAStartup(1, NULL);
    // cppcheck-suppress nullPointer
    bind(socketInit, NULL, 5);
    // cppcheck-suppress nullPointer
    getpeername(socketInit, NULL, &intInit);
    // cppcheck-suppress nullPointer
    getpeername(socketInit, &sockaddrUninit, pIntNull);
    // cppcheck-suppress nullPointer
    getsockopt(sockInit, 1, 2, NULL, &intInit);
    // cppcheck-suppress nullPointer
    getsockopt(sockInit, 1, 2, charArray, pIntNull);
}

void memleak_malloca()
{
    // cppcheck-suppress unreadVariable
    void *pMem = _malloca(10);
    // cppcheck-suppress memleak
}

void memleak_AllocateAndInitializeSid()
{
    PSID pEveryoneSID = NULL;
    SID_IDENTIFIER_AUTHORITY SIDAuthWorld = SECURITY_WORLD_SID_AUTHORITY;
    AllocateAndInitializeSid(&SIDAuthWorld, 1, SECURITY_WORLD_RID, 0, 0, 0, 0, 0, 0, 0, &pEveryoneSID)
    // TODO: enable when #6994 is implemented cppcheck-suppress memleak
}

void memleak_HeapAlloc()
{
    LPVOID pMem;
    pMem = HeapAlloc(GetProcessHeap(), 0, 10);
    HeapValidate(GetProcessHeap(), 0, pMem);
    // cppcheck-suppress unreadVariable
    SIZE_T memSize = HeapSize(GetProcessHeap(), 0, pMem);
    // cppcheck-suppress memleak
}

void memleak_LocalAlloc()
{
    LPTSTR pszBuf;
    // cppcheck-suppress LocalAllocCalled
    pszBuf = (LPTSTR)LocalAlloc(LPTR, MAX_PATH*sizeof(TCHAR));
    (void)LocalSize(pszBuf);
    (void)LocalFlags(pszBuf);
    LocalLock(pszBuf);
    LocalUnlock(pszBuf);
    // cppcheck-suppress memleak
}

void resourceLeak_CreateSemaphoreA()
{
    HANDLE hSemaphore;
    // cppcheck-suppress unreadVariable
    hSemaphore = CreateSemaphoreA(NULL, 0, 1, "sem1");
    // cppcheck-suppress resourceLeak
}

void resourceLeak_CreateSemaphoreEx()
{
    HANDLE hSemaphore;
    // cppcheck-suppress unreadVariable
    hSemaphore = CreateSemaphoreEx(NULL, 0, 1, NULL, 0, SEMAPHORE_ALL_ACCESS);
    // cppcheck-suppress resourceLeak
}

void resourceLeak_OpenSemaphore()
{
    HANDLE hSemaphore;
    // cppcheck-suppress unreadVariable
    hSemaphore = OpenSemaphore(SEMAPHORE_ALL_ACCESS, TRUE, "sem");
    // cppcheck-suppress resourceLeak
}

void resourceLeak_CreateMutexA()
{
    HANDLE hMutex;
    // cppcheck-suppress unreadVariable
    hMutex = CreateMutexA(NULL, TRUE, "sem1");
    // cppcheck-suppress resourceLeak
}

void resourceLeak_CreateMutexEx()
{
    HANDLE hMutex;
    // cppcheck-suppress unreadVariable
    hMutex = CreateMutexEx(NULL, "sem", 0, MUTEX_ALL_ACCESS);
    // cppcheck-suppress resourceLeak
}

void resourceLeak_OpenMutex()
{
    HANDLE hMutex;
    // cppcheck-suppress unreadVariable
    hMutex = OpenMutex(MUTEX_ALL_ACCESS, TRUE, "sem");
    // cppcheck-suppress resourceLeak
}

void resourceLeak_LoadLibrary()
{
    HINSTANCE hInstLib;
    hInstLib = ::LoadLibrary(L"My.dll");
    typedef BOOL (WINAPI *fpFunc)();
    // cppcheck-suppress unreadVariable
    fpFunc pFunc = GetProcAddress(hInstLib, "name");
    // cppcheck-suppress resourceLeak
}

void resourceLeak_CreateEvent()
{
    HANDLE hEvent;
    hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
    SetEvent(hEvent);
    // cppcheck-suppress resourceLeak
}

void resourceLeak_CreateEventExA()
{
    HANDLE hEvent;
    // cppcheck-suppress unreadVariable
    hEvent = CreateEventExA(NULL, "test", CREATE_EVENT_INITIAL_SET, EVENT_MODIFY_STATE);
    // cppcheck-suppress resourceLeak
}

void resourceLeak_OpenEventW()
{
    HANDLE hEvent;
    // cppcheck-suppress unreadVariable
    hEvent = OpenEventW(EVENT_ALL_ACCESS, TRUE, L"testevent");
    // cppcheck-suppress resourceLeak
}

void resourceLeak_socket()
{
    SOCKET sock;
    // cppcheck-suppress unreadVariable
    sock = socket(1, 2, 3);
    // cppcheck-suppress resourceLeak
}

void ignoredReturnValue()
{
    // cppcheck-suppress leakReturnValNotUsed
    CreateSemaphoreW(NULL, 0, 1, NULL);
    // cppcheck-suppress leakReturnValNotUsed
    CreateSemaphoreExA(NULL, 0, 1, NULL, 0, SEMAPHORE_ALL_ACCESS);
    // cppcheck-suppress leakReturnValNotUsed
    OpenSemaphoreA(SEMAPHORE_ALL_ACCESS, TRUE, "sem");

    // cppcheck-suppress leakReturnValNotUsed
    CreateMutexW(NULL, FALSE, NULL);
    // cppcheck-suppress leakReturnValNotUsed
    CreateMutexExA(NULL, NULL, 1, MUTEX_ALL_ACCESS);
    // cppcheck-suppress leakReturnValNotUsed
    OpenMutexA(MUTEX_ALL_ACCESS, TRUE, "sem");

    // cppcheck-suppress leakReturnValNotUsed
    LoadLibrary(L"My.dll");
    // cppcheck-suppress leakReturnValNotUsed
    LoadLibraryEx(L"My.dll", NULL, 0);

    HINSTANCE hInstLib = LoadLibrary(L"My.dll");
    // cppcheck-suppress ignoredReturnValue
    GetProcAddress(hInstLib, "name");
    FreeLibrary(hInstLib);

    // cppcheck-suppress leakReturnValNotUsed
    CreateEvent(NULL, FALSE, FALSE, NULL);
    // cppcheck-suppress leakReturnValNotUsed
    OpenEvent(EVENT_ALL_ACCESS, FALSE, L"testevent");
    // cppcheck-suppress leakReturnValNotUsed
    CreateEventEx(NULL, L"test", CREATE_EVENT_INITIAL_SET, EVENT_MODIFY_STATE);

    // cppcheck-suppress leakReturnValNotUsed
    _malloca(10);
    // cppcheck-suppress ignoredReturnValue
    _alloca(5);

    // cppcheck-suppress ignoredReturnValue
    GetLastError();

    // cppcheck-suppress ignoredReturnValue
    GetProcessHeap()
    // cppcheck-suppress leakReturnValNotUsed
    HeapAlloc(GetProcessHeap(), 0, 10);
    // cppcheck-suppress leakReturnValNotUsed
    HeapReAlloc(GetProcessHeap(), 0, 1, 0);

    // cppcheck-suppress leakReturnValNotUsed
    socket(1, 2, 3);

    // cppcheck-suppress ignoredReturnValue
    _fileno(stdio);

    // cppcheck-suppress lstrlenCalled
    // cppcheck-suppress ignoredReturnValue
    lstrlen(TEXT("test"));
}

void invalidFunctionArg()
{
    HANDLE hSemaphore;
    // cppcheck-suppress invalidFunctionArg
    hSemaphore = CreateSemaphore(NULL, 0, 0, NULL);
    CloseHandle(hSemaphore);
    // cppcheck-suppress invalidFunctionArgBool
    hSemaphore = CreateSemaphore(NULL, 0, 1, true);
    CloseHandle(hSemaphore);
    // cppcheck-suppress invalidFunctionArg
    hSemaphore = CreateSemaphoreEx(NULL, 0, 0, NULL, 0, SEMAPHORE_ALL_ACCESS);
    CloseHandle(hSemaphore);
    // cppcheck-suppress invalidFunctionArg
    hSemaphore = CreateSemaphoreEx(NULL, 0, 1, NULL, 1, SEMAPHORE_ALL_ACCESS);
    CloseHandle(hSemaphore);

    HANDLE hMutex;
    // cppcheck-suppress invalidFunctionArgBool
    hMutex = CreateMutex(NULL, TRUE, false);
    CloseHandle(hMutex);
    // cppcheck-suppress invalidFunctionArgBool
    hMutex = CreateMutex(NULL, FALSE, true);
    CloseHandle(hMutex);
    // cppcheck-suppress invalidFunctionArg
    hMutex = CreateMutexEx(NULL, NULL, 3, MUTEX_ALL_ACCESS);
    CloseHandle(hMutex);

    //Incorrect: 2. parameter to LoadLibraryEx() must be NULL
    // cppcheck-suppress invalidFunctionArg
    HINSTANCE hInstLib = LoadLibraryEx(L"My.dll", 1, 0);
    FreeLibrary(hInstLib);

    // cppcheck-suppress invalidFunctionArg
    void *pMem = _malloca(-1);
    _freea(pMem);
    // cppcheck-suppress unreadVariable
    // cppcheck-suppress invalidFunctionArg
    pMem = _alloca(-5);
}

void uninitvar()
{
    HANDLE hSemaphore;
    // cppcheck-suppress uninitvar
    CloseHandle(hSemaphore);

    char buf[10];
    // cppcheck-suppress lstrcatCalled
    // cppcheck-suppress uninitvar
    lstrcat(buf, "test");
    buf[0] = '\0';
    char buf2[2];
    // cppcheck-suppress lstrcatCalled
    // cppcheck-suppress uninitvar
    lstrcat(buf, buf2);

    HANDLE hMutex;
    // cppcheck-suppress uninitvar
    ReleaseMutex(hMutex);
    // cppcheck-suppress uninitvar
    CloseHandle(hMutex);

    HANDLE hEvent;
    // cppcheck-suppress uninitvar
    PulseEvent(hEvent);
    // cppcheck-suppress uninitvar
    ResetEvent(hEvent);
    // cppcheck-suppress uninitvar
    SetEvent(hEvent);
    // cppcheck-suppress uninitvar
    CloseHandle(hEvent);

    char buf_uninit[10];
    // cppcheck-suppress strlwrCalled
    // cppcheck-suppress uninitvar
    strlwr(buf_uninit);
    // cppcheck-suppress struprCalled
    // cppcheck-suppress uninitvar
    strupr(buf_uninit);

    DWORD dwordUninit;
    // cppcheck-suppress uninitvar
    SetLastError(dwordUninit);

    DWORD dwordUninit;
    // cppcheck-suppress uninitvar
    GetUserName(NULL, &dwordUninit);

    FILE *pFileUninit;
    // cppcheck-suppress uninitvar
    // cppcheck-suppress ignoredReturnValue
    _fileno(pFileUninit);
}

void errorPrintf()
{
    char bufC[50];
    // cppcheck-suppress wrongPrintfScanfArgNum
    sprintf_s(bufC, _countof(bufC), "%s %d", "sprintf_s");
    printf("%s\n", bufC);
    // cppcheck-suppress wrongPrintfScanfArgNum
    sprintf_s(bufC, "%s %d", "sprintf_s");
    printf("%s\n", bufC);
    // cppcheck-suppress wrongPrintfScanfArgNum
    sprintf_s(bufC, _countof(bufC), "test", 0);
    printf("%s\n", bufC);
    // cppcheck-suppress wrongPrintfScanfArgNum
    sprintf_s(bufC, "test", "sprintf_s");
    printf("%s\n", bufC);
    // cppcheck-suppress invalidPrintfArgType_s
    sprintf_s(bufC, _countof(bufC), "%s", 1);
    printf("%s\n", bufC);
    // cppcheck-suppress invalidPrintfArgType_s
    sprintf_s(bufC, "%s", 1);
    printf("%s\n", bufC);

    wchar_t bufWC[50];
    // cppcheck-suppress wrongPrintfScanfArgNum
    swprintf_s(bufWC, _countof(bufWC), L"%s %d", L"swprintf_s");
    wprintf(L"%s\n", bufWC);
    // cppcheck-suppress wrongPrintfScanfArgNum
    swprintf_s(bufWC, L"%s %d", L"swprintf_s");
    wprintf(L"%s\n", bufWC);
    // cppcheck-suppress wrongPrintfScanfArgNum
    swprintf_s(bufWC, _countof(bufWC), L"test", 0);
    wprintf(L"%s\n", bufWC);
    // cppcheck-suppress wrongPrintfScanfArgNum
    swprintf_s(bufWC, L"test", L"swprintf_s");
    wprintf(L"%s\n", bufWC);
    // cppcheck-suppress invalidPrintfArgType_s
    swprintf_s(bufWC, _countof(bufWC), L"%s", 1);
    wprintf(L"%s\n", bufWC);
    // cppcheck-suppress invalidPrintfArgType_s
    swprintf_s(bufWC, L"%s", 1);
    wprintf(L"%s\n", bufWC);

    TCHAR bufTC[50];
    // cppcheck-suppress wrongPrintfScanfArgNum
    _stprintf_s(bufTC, _countof(bufTC), TEXT("%s %d"), TEXT("_stprintf_s"));
    _tprintf(L"%s\n", bufTC);
    // cppcheck-suppress wrongPrintfScanfArgNum
    _stprintf_s(bufTC, TEXT("%s %d"), TEXT("_stprintf_s"));
    _tprintf(TEXT("%s\n"), bufTC);
    // cppcheck-suppress wrongPrintfScanfArgNum
    _stprintf_s(bufTC, _countof(bufTC), TEXT("test"), 0);
    _tprintf(TEXT("%s\n"), bufTC);
    // cppcheck-suppress wrongPrintfScanfArgNum
    _stprintf_s(bufTC, TEXT("test"), TEXT("_stprintf_s"));
    _tprintf(TEXT("%s\n"), bufTC);
    // cppcheck-suppress invalidPrintfArgType_s
    _stprintf_s(bufTC, _countof(bufTC), TEXT("%s"), 1);
    _tprintf(TEXT("%s\n"), bufTC);
    // cppcheck-suppress invalidPrintfArgType_s
    _stprintf_s(bufTC, TEXT("%s"), 1);
    _tprintf(TEXT("%s\n"), bufTC);
}

void allocDealloc_GetModuleHandleEx()
{
    // For GetModuleHandleEx it depends on the first argument if FreeLibrary
    // must be called or is not allowed to be called.
    // If the first argument is GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT
    // (0x00000002), a call to FreeLibrary is not allowed, otherwise it is
    // necessary.
    // Since this is not possible to configure at the moment cppcheck should
    // accept not calling FreeLibrary and also calling it for the handle.
    // TODO: Enhance cppcheck to conditionally check for alloc/dealloc issues.

    // No warning because of correct FreeLibrary on 'hModule' should be issued.
    HMODULE hModule1;
    if (GetModuleHandleEx(0, NULL, &hModule1)) {
        FreeLibrary(hModule1);
    }

    //This is a false negative, but it is not detected to avoid false positives.
    HMODULE hModule2;
    GetModuleHandleEx(0, NULL, &hModule2);
}

void uninitvar_tolower(_locale_t l)
{
    int c;
    // cppcheck-suppress uninitvar
    (void)_tolower(c);
    // cppcheck-suppress uninitvar
    (void)_tolower_l(c, l);
}

void uninitvar_toupper(_locale_t l)
{
    int c;
    // cppcheck-suppress uninitvar
    (void)_toupper(c);
    // cppcheck-suppress uninitvar
    (void)_toupper_l(c, l);
}

void uninitvar_towlower(_locale_t l)
{
    wint_t i;
    // cppcheck-suppress uninitvar
    (void)_towlower_l(i, l);
}

void uninitvar_towupper(_locale_t l)
{
    wint_t i;
    // cppcheck-suppress uninitvar
    (void)_towupper_l(i, l);
}

void oppositeInnerCondition_SUCCEEDED_FAILED(HRESULT hr)
{
    if (SUCCEEDED(hr)) {
        // TODO ticket #8596 cppcheck-suppress oppositeInnerCondition
        if (FAILED(hr)) {
        }
    }
}

/*HANDLE WINAPI CreateThread(
  _In_opt_  LPSECURITY_ATTRIBUTES  lpThreadAttributes,
  _In_      SIZE_T                 dwStackSize,
  _In_      LPTHREAD_START_ROUTINE lpStartAddress,
  _In_opt_  LPVOID                 lpParameter,
  _In_      DWORD                  dwCreationFlags,
  _Out_opt_ LPDWORD                lpThreadId
);*/
HANDLE test_CreateThread(LPSECURITY_ATTRIBUTES  lpThreadAttributes,
                         SIZE_T                 dwStackSize,
                         LPTHREAD_START_ROUTINE lpStartAddress,
                         LPVOID                 lpParameter,
                         DWORD                  dwCreationFlags,
                         LPDWORD                lpThreadId)
{
    // Create uninitialized variables
    LPSECURITY_ATTRIBUTES  uninit_lpThreadAttributes;
    SIZE_T                 uninit_dwStackSize;
    LPTHREAD_START_ROUTINE uninit_lpStartAddress;
    LPVOID                 uninit_lpParameter;
    DWORD                  uninit_dwCreationFlags;

    // cppcheck-suppress leakReturnValNotUsed
    // cppcheck-suppress uninitvar
    (void) CreateThread(lpThreadAttributes, dwStackSize, lpStartAddress, lpParameter, uninit_dwCreationFlags, lpThreadId);
    // cppcheck-suppress leakReturnValNotUsed
    // cppcheck-suppress uninitvar
    (void) CreateThread(lpThreadAttributes, dwStackSize, lpStartAddress, uninit_lpParameter, dwCreationFlags, lpThreadId);
    // @todo uninitvar shall be reported
    // cppcheck-suppress leakReturnValNotUsed
    (void) CreateThread(lpThreadAttributes, dwStackSize, uninit_lpStartAddress, lpParameter, dwCreationFlags, lpThreadId);
    // cppcheck-suppress leakReturnValNotUsed
    // cppcheck-suppress uninitvar
    (void) CreateThread(lpThreadAttributes, uninit_dwStackSize, lpStartAddress, lpParameter, dwCreationFlags, lpThreadId);
    // @todo uninitvar shall be reported
    // cppcheck-suppress leakReturnValNotUsed
    (void) CreateThread(uninit_lpThreadAttributes, dwStackSize, lpStartAddress, lpParameter, dwCreationFlags, lpThreadId);

    // cppcheck-suppress leakReturnValNotUsed
    // cppcheck-suppress nullPointer
    (void) CreateThread(lpThreadAttributes, dwStackSize, 0, lpParameter, dwCreationFlags, lpThreadId);

    // cppcheck-suppress leakReturnValNotUsed
    // cppcheck-suppress invalidFunctionArg
    (void) CreateThread(lpThreadAttributes, -1, lpStartAddress, lpParameter, dwCreationFlags, lpThreadId);

    // no warning shall be shown for
    return CreateThread(lpThreadAttributes, dwStackSize, lpStartAddress, lpParameter, dwCreationFlags, lpThreadId);
}

// unsigned char *_mbscat(unsigned char *strDestination, const unsigned char *strSource);
unsigned char * uninitvar_mbscat(unsigned char *strDestination, const unsigned char *strSource)
{
    unsigned char *uninit_deststr;
    unsigned char *uninit_srcstr;
    // cppcheck-suppress uninitvar
    (void)_mbscat(uninit_deststr,uninit_srcstr);
    // cppcheck-suppress uninitvar
    (void)_mbscat(strDestination,uninit_srcstr);
    // cppcheck-suppress uninitvar
    (void)_mbscat(uninit_deststr,uninit_deststr);

    // no warning shall be shown for
    return _mbscat(strDestination,strSource);
}

// unsigned char *_mbscat(unsigned char *strDestination, const unsigned char *strSource);
unsigned char * nullPointer_mbscat(unsigned char *strDestination, const unsigned char *strSource)
{
    // cppcheck-suppress nullPointer
    (void)_mbscat(0,strSource);
    // cppcheck-suppress nullPointer
    (void)_mbscat(strDestination,0);

    // no warning shall be shown for
    return _mbscat(strDestination,strSource);
}

// errno_t _mbscat_s(unsigned char *strDestination, size_t numberOfElements, const unsigned char *strSource );
error_t uninitvar_mbscat_s(unsigned char *strDestination, size_t numberOfElements, const unsigned char *strSource)
{
    unsigned char *uninit_strDestination;
    size_t uninit_numberOfElements;
    unsigned char *uninit_strSource;

    // cppcheck-suppress uninitvar
    (void)_mbscat_s(uninit_strDestination, numberOfElements, strSource);
    // cppcheck-suppress uninitvar
    (void)_mbscat_s(strDestination, uninit_numberOfElements, strSource);
    // cppcheck-suppress uninitvar
    (void)_mbscat_s(strDestination, numberOfElements, uninit_strSource);

    // no warning shall be shown for
    return _mbscat_s(strDestination, numberOfElements, strSource);
}

// errno_t _mbscat_s(unsigned char *strDestination, size_t numberOfElements, const unsigned char *strSource );
error_t nullPointer_mbscat_s(unsigned char *strDestination, size_t numberOfElements, const unsigned char *strSource)
{
    // cppcheck-suppress nullPointer
    (void)_mbscat_s(0, numberOfElements, strSource);
    // cppcheck-suppress nullPointer
    (void)_mbscat_s(strDestination, numberOfElements, 0);

    // no warning shall be shown for
    return _mbscat_s(strDestination, numberOfElements, strSource);
}

// errno_t _strncpy_s_l(char *strDest, size_t numberOfElements, const char *strSource, size_t count, _locale_t locale);
error_t uninitvar__strncpy_s_l(char *strDest, size_t numberOfElements, const char *strSource, size_t count, _locale_t locale)
{
    size_t uninit_numberOfElements;
    const char *uninit_strSource;
    size_t uninit_count;
    _locale_t uninit_locale;

    // cppcheck-suppress uninitvar
    (void)_strncpy_s_l(strDest, uninit_numberOfElements, strSource, count, locale);
    // cppcheck-suppress uninitvar
    (void)_strncpy_s_l(strDest, numberOfElements, uninit_strSource, count, locale);
    // cppcheck-suppress uninitvar
    (void)_strncpy_s_l(strDest, numberOfElements, strSource, uninit_count, locale);
    // cppcheck-suppress uninitvar
    (void)_strncpy_s_l(strDest, numberOfElements, strSource, count, uninit_locale);

    // no warning shall be shown for
    return _strncpy_s_l(strDest, numberOfElements, strSource, count, locale);
}

// errno_t _strncpy_s_l(char *strDest, size_t numberOfElements, const char *strSource, size_t count, _locale_t locale);
error_t nullPointer__strncpy_s_l(char *strDest, size_t numberOfElements, const char *strSource, size_t count, _locale_t locale)
{
    // cppcheck-suppress nullPointer
    (void)_strncpy_s_l(0, numberOfElements, strSource, count, locale);
    // cppcheck-suppress nullPointer
    (void)_strncpy_s_l(strDest, numberOfElements, 0, count, locale);

    // no warning shall be shown for
    return _strncpy_s_l(strDest, numberOfElements, strSource, count, locale);
}
