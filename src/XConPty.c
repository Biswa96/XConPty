#include <Windows.h>
#include <assert.h>
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

DWORD WINAPI PipeListener(HANDLE hPipeIn)
{
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
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

HRESULT WINAPI XConPty(PWSTR szCommand)
{
    BOOL bRes;

    HANDLE hPipeIn = NULL, hPipeOut;
    HANDLE hPipePTYIn, hPipePTYOut;
    HPCON_INTERNAL hpCon;

#ifdef FUN_MODE
    /* Do not enable VT-100 control sequences to view raw buffers */
#else
    HANDLE hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD consoleMode;
    GetConsoleMode(hStdOut, &consoleMode);
    bRes = SetConsoleMode(hStdOut, consoleMode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
#endif

    /* Create the pipes to which the ConPTY will connect */
    if (CreatePipe(&hPipePTYIn, &hPipeOut, NULL, 0)
        && CreatePipe(&hPipeIn, &hPipePTYOut, NULL, 0))
    {
        /* Create the Pseudo Console attached to the PTY-end of the pipes */
        COORD consoleSize = { width, height };
        bRes = CreatePseudoConsole_mod(consoleSize, hPipePTYIn, hPipePTYOut, 0, &hpCon);
        assert(bRes == 0);

        CloseHandle(hPipePTYOut);
        CloseHandle(hPipePTYIn);
    }

    /* Create & start thread to write */
    HANDLE hThread = CreateThread(NULL, 0, PipeListener, hPipeIn, 0, NULL);
    assert(hThread != INVALID_HANDLE_VALUE);

    /* Initialize thread attribute */
    size_t AttrSize;
    LPPROC_THREAD_ATTRIBUTE_LIST AttrList = NULL;
    InitializeProcThreadAttributeList(NULL, 1, 0, &AttrSize);
    AttrList = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, AttrSize);
    InitializeProcThreadAttributeList(AttrList, 1, 0, &AttrSize);
    bRes = UpdateProcThreadAttribute(AttrList, 0, PROC_THREAD_ATTRIBUTE_PSEUDOCONSOLE, /* 0x20016u */
                                     &hpCon, sizeof(&hpCon), NULL, NULL);
    assert(bRes != 0);

    /* Initialize startup info struct */
    PROCESS_INFORMATION ProcInfo;
    memset(&ProcInfo, 0, sizeof ProcInfo);
    STARTUPINFOEXW SInfoEx;
    memset(&SInfoEx, 0, sizeof SInfoEx);
    SInfoEx.StartupInfo.cb = sizeof SInfoEx;
    SInfoEx.lpAttributeList = AttrList;

    bRes = CreateProcessAsUserW(
        NULL,
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
    assert(bRes != 0);

    if (bRes)
    {
#ifdef FUN_MODE
        WaitForSingleObject(ProcInfo.hThread, INFINITE);
#else
        WaitForSingleObject(ProcInfo.hThread, nTimes * mSeconds);
#endif
    }

    /* Cleanup */
    CloseHandle(ProcInfo.hThread);
    CloseHandle(ProcInfo.hProcess);
    HeapFree(GetProcessHeap(), 0, AttrList);
    ClosePseudoConsole_mod(&hpCon);
    CloseHandle(hPipeOut);
    CloseHandle(hPipeIn);

    return bRes;
}

#define STRINGIFY(s) L ## #s
#define XSTRINGIFY(s) STRINGIFY(s)
#define count XSTRINGIFY(nTimes)

#ifdef FUN_MODE
    wchar_t szCommand[] = L"cmd.exe";
#else
    wchar_t szCommand[] = L"ping localhost -n " count;
#endif

int WINAPI main(void)
{
    XConPty(szCommand);
    return 0;
}
