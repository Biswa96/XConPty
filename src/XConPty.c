#include <windows.h>
#include <assert.h>
#include "PseudoConsole.h"

#define BUFF_SIZE 0x200

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

int WINAPI main(void)
{
    BOOL bRes;

    HANDLE hPipeIn = NULL, hPipeOut;
    HANDLE hPipePTYIn, hPipePTYOut;
    HPCON hpCon = NULL;

    /* Create the pipes to which the ConPTY will connect */
    if (CreatePipe(&hPipePTYIn, &hPipeOut, NULL, 0)
        && CreatePipe(&hPipeIn, &hPipePTYOut, NULL, 0))
    {
        /* Create the Pseudo Console attached to the PTY-end of the pipes */
        COORD consoleSize = { 0 };
        CONSOLE_SCREEN_BUFFER_INFO csbi = { 0 };
        if (GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi))
        {
            consoleSize.X = csbi.srWindow.Right - csbi.srWindow.Left + 1;
            consoleSize.Y = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;
        }
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
    bRes = UpdateProcThreadAttribute(
        AttrList, 0, PROC_THREAD_ATTRIBUTE_PSEUDOCONSOLE, /* 0x20016u */
        hpCon, sizeof hpCon, NULL, NULL);
    assert(bRes != 0);

    /* Initialize startup info struct */
    PROCESS_INFORMATION ProcInfo;
    memset(&ProcInfo, 0, sizeof ProcInfo);
    STARTUPINFOEXW SInfoEx;
    memset(&SInfoEx, 0, sizeof SInfoEx);
    SInfoEx.StartupInfo.cb = sizeof SInfoEx;
    SInfoEx.lpAttributeList = AttrList;

    wchar_t Command[] = L"cmd.exe";

    bRes = CreateProcessW(
        NULL,
        Command,
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
        WaitForSingleObject(ProcInfo.hThread, INFINITE);

    /* Cleanup */
    CloseHandle(ProcInfo.hThread);
    CloseHandle(ProcInfo.hProcess);
    HeapFree(GetProcessHeap(), 0, AttrList);
    ClosePseudoConsole_mod(hpCon);
    CloseHandle(hPipeOut);
    CloseHandle(hPipeIn);

    return bRes;
}
