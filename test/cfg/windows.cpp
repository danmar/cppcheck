
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
}

void memleak_malloca()
{
    // cppcheck-suppress unreadVariable
    void *pMem = _malloca(10);
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
