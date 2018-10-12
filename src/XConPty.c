#include "KernelBase.h"
#include <winternl.h>

#define nTimes 10
#define mSeconds 1000
#define BUFF_SIZE 0x200

void PipeListener(HANDLE hPipeIn)
{
    HANDLE hConsole = X_GetStdHandle(STD_OUTPUT_HANDLE);
    char szBuffer[BUFF_SIZE];
    DWORD dwBytesWritten, dwBytesRead;
    BOOL fRead;

    do
    {
        fRead = ReadFile(hPipeIn, szBuffer, BUFF_SIZE, &dwBytesRead, NULL);
        WriteFile(hConsole, szBuffer, dwBytesRead, &dwBytesWritten, NULL);
    } while (fRead && dwBytesRead >= 0);
}

#define width 120
#define height 30
#ifndef PROC_THREAD_ATTRIBUTE_PSEUDOCONSOLE
#define PROC_THREAD_ATTRIBUTE_PSEUDOCONSOLE \
    ProcThreadAttributeValue (22, FALSE, TRUE, FALSE)
#endif

HRESULT XConPty(PWSTR szCommand)
{
    BOOL bRes;
    HRESULT hRes = 0;

    HANDLE hPipeIn = NULL, hPipeOut = NULL;
    X_HPCON hpCon;
    HANDLE hPipePTYIn, hPipePTYOut;
    HANDLE hToken = NULL; // Modify this if necessary

    PROCESS_INFORMATION ProcInfo;
    STARTUPINFOEXW SInfoEx = { 0 };
    LPPROC_THREAD_ATTRIBUTE_LIST AttrList = NULL;

    HANDLE hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD consoleMode;
    GetConsoleMode(hStdOut, &consoleMode);
    hRes = SetConsoleMode(hStdOut, consoleMode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);

    // Create the pipes to which the ConPTY will connect
    if (X_CreatePipe(&hPipePTYIn, &hPipeOut, NULL, 0) &&
        X_CreatePipe(&hPipeIn, &hPipePTYOut, NULL, 0))
    {
        // Create the Pseudo Console attached to the PTY-end of the pipes
        COORD consoleSize = { width, height };
        hRes = X_CreatePseudoConsole(consoleSize, hPipePTYIn, hPipePTYOut, 0, &hpCon);
        if (hRes != 0)
            Log(hRes, L"CreatePseudoConsole");

        NtClose(hPipePTYOut);
        NtClose(hPipePTYIn);
    }

    // Create & start thread to write
    HANDLE hThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)PipeListener, hPipeIn, 0, NULL);
    if (hThread == INVALID_HANDLE_VALUE)
        Log(X_GetLastError(), L"CreateThread");

    // Initialize thread attribute
    size_t AttrSize;
    X_InitializeProcThreadAttributeList(NULL, 1, 0, &AttrSize);
    AttrList = (LPPROC_THREAD_ATTRIBUTE_LIST)malloc(AttrSize);
    X_InitializeProcThreadAttributeList(AttrList, 1, 0, &AttrSize);
    bRes = X_UpdateProcThreadAttribute(
        AttrList,
        0,
        PROC_THREAD_ATTRIBUTE_PSEUDOCONSOLE, //0x20016u
        &hpCon,
        sizeof(PVOID),
        NULL,
        NULL);
    if (!bRes)
        Log(X_GetLastError(), L"UpdateProcThreadAttribute");

    // Initialize startup info struct        
    SInfoEx.StartupInfo.cb = sizeof(STARTUPINFOEXW);
    SInfoEx.lpAttributeList = AttrList;

    bRes = CreateProcessAsUserW(
        hToken,
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

    if (bRes)
        WaitForSingleObject(ProcInfo.hThread, nTimes * mSeconds);
    else
        Log(X_GetLastError(), L"CreateProcessW");

    // Cleanup
    NtClose(ProcInfo.hThread);
    NtClose(ProcInfo.hProcess);
    free(AttrList);
    X_ClosePseudoConsole(hpCon);
    NtClose(hPipeOut);
    NtClose(hPipeIn);

    return hRes;
}

#define STRINGIFY(s) L#s
#define XSTRINGIFY(s) STRINGIFY(s)
#define count XSTRINGIFY(nTimes)

int main(void)
{
    wchar_t szCommand[] = L"ping localhost -n " count ;
    XConPty(szCommand);
}
