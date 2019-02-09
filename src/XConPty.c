#include "WinInternal.h"
#include "KernelBase.h"
#include "PseudoConsole.h"

#define nTimes 10
#define mSeconds 1000
#define BUFF_SIZE 0x200

#define width 120
#define height 30
#ifndef PROC_THREAD_ATTRIBUTE_PSEUDOCONSOLE
#define ProcThreadAttributePseudoConsole 22
#define PROC_THREAD_ATTRIBUTE_PSEUDOCONSOLE \
    ProcThreadAttributeValue (ProcThreadAttributePseudoConsole, FALSE, TRUE, FALSE)
#endif

DWORD
WINAPI
PipeListener(HANDLE hPipeIn)
{
    HANDLE hConsole = X_GetStdHandle(STD_OUTPUT_HANDLE);
    char szBuffer[BUFF_SIZE];
    DWORD dwBytesWritten, dwBytesRead;

    while (ReadFile(hPipeIn, szBuffer, BUFF_SIZE, &dwBytesRead, NULL))
    {
        if (dwBytesRead == 0)
            break;
        WriteFile(hConsole, szBuffer, dwBytesRead, &dwBytesWritten, NULL);
    }
    return TRUE;
}

HRESULT
WINAPI
XConPty(PWSTR szCommand)
{
    BOOL bRes = 0;
    HRESULT hRes = 0;

    HANDLE hPipeIn = NULL, hPipeOut = NULL;
    X_HPCON hpCon;
    HANDLE hPipePTYIn, hPipePTYOut;
    HANDLE hToken = NULL; // Modify this if necessary

    PROCESS_INFORMATION ProcInfo;
    STARTUPINFOEXW SInfoEx = { 0 };
    LPPROC_THREAD_ATTRIBUTE_LIST AttrList = NULL;

#ifdef FUN_MODE
    // Do not enable VT-100 control sequences to view raw buffers
#else
    HANDLE hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD consoleMode;
    GetConsoleMode(hStdOut, &consoleMode);
    bRes = SetConsoleMode(hStdOut, consoleMode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
#endif

    // Create the pipes to which the ConPTY will connect
    if (X_CreatePipe(&hPipePTYIn, &hPipeOut, NULL, 0) &&
        X_CreatePipe(&hPipeIn, &hPipePTYOut, NULL, 0))
    {
        // Create the Pseudo Console attached to the PTY-end of the pipes
        COORD consoleSize = { width, height };
        hRes = X_CreatePseudoConsole(consoleSize, hPipePTYIn, hPipePTYOut, 0, &hpCon);
        LogResult(hRes, L"CreatePseudoConsole");

        NtClose(hPipePTYOut);
        NtClose(hPipePTYIn);
    }

    // Create & start thread to write
    HANDLE hThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)PipeListener, hPipeIn, 0, NULL);
    if (hThread == INVALID_HANDLE_VALUE)
        LogResult(X_GetLastError(), L"CreateThread");

    // Initialize thread attribute
    size_t AttrSize;
    X_InitializeProcThreadAttributeList(NULL, 1, 0, &AttrSize);
    AttrList = RtlAllocateHeap(X_GetProcessHeap(), HEAP_ZERO_MEMORY, AttrSize);
    X_InitializeProcThreadAttributeList(AttrList, 1, 0, &AttrSize);
    bRes = X_UpdateProcThreadAttribute(AttrList,
                                       0,
                                       PROC_THREAD_ATTRIBUTE_PSEUDOCONSOLE, //0x20016u
                                       &hpCon,
                                       sizeof(PVOID),
                                       NULL,
                                       NULL);
    if (!bRes)
        LogResult(X_GetLastError(), L"UpdateProcThreadAttribute");

    // Initialize startup info struct
    SInfoEx.StartupInfo.cb = sizeof SInfoEx;
    SInfoEx.lpAttributeList = AttrList;

    bRes = CreateProcessAsUserW(hToken,
                                NULL,
                                szCommand,
                                NULL,
                                NULL,
                                FALSE,
                                EXTENDED_STARTUPINFO_PRESENT,
                                NULL,
                                NULL,
                                &SInfoEx.StartupInfo,
                                &ProcInfo);

    // X_ResizePseudoConsole(&hpCon, (COORD){ 100, 100 });

    if (bRes)
    {
#ifdef FUN_MODE
        WaitForSingleObject(ProcInfo.hThread, INFINITE);
#else
        WaitForSingleObject(ProcInfo.hThread, nTimes * mSeconds);
#endif
    }
    else
        LogResult(X_GetLastError(), L"CreateProcessAsUserW");

    // Cleanup
    NtClose(ProcInfo.hThread);
    NtClose(ProcInfo.hProcess);
    RtlFreeHeap(X_GetProcessHeap(), 0, AttrList);
    X_ClosePseudoConsole(&hpCon);
    NtClose(hPipeOut);
    NtClose(hPipeIn);

    return hRes;
}

#define STRINGIFY(s) L ## #s
#define XSTRINGIFY(s) STRINGIFY(s)
#define count XSTRINGIFY(nTimes)

#ifdef FUN_MODE
    wchar_t szCommand[] = L"cmd.exe";
#else
    wchar_t szCommand[] = L"ping localhost -n " count;
#endif

int
WINAPI
main(void)
{
    XConPty(szCommand);
}
