#include "ntdll.h"
#include "KernelBase.h"
#include <stdio.h>

#ifndef PSEUDOCONSOLE_INHERIT_CURSOR
#define PSEUDOCONSOLE_INHERIT_CURSOR 0x1
#endif

HRESULT X_CreatePseudoConsole(
    COORD size,
    HANDLE hInput,
    HANDLE hOutput,
    DWORD dwFlags,
    X_HPCON* hpCon)
{
    NTSTATUS Status;
    BOOL bRes;

    HANDLE InputHandle = NULL, OutputHandle = NULL;
    HANDLE hConServer = NULL, hConReference;
    HANDLE hProc = X_GetCurrentProcess();
    HANDLE ReadPipeHandle, WritePipeHandle;
    HANDLE hToken = NULL; // Modify this if necessary

    SECURITY_ATTRIBUTES PipeAttributes;
    wchar_t ConHost[MAX_PATH];
    size_t AttrSize;
    LPPROC_THREAD_ATTRIBUTE_LIST AttrList = NULL;
    STARTUPINFOEXW SInfoEx = { 0 };
    PROCESS_INFORMATION ProcInfo;

    if (X_DuplicateHandle(hProc, hInput, hProc, &InputHandle, 0, TRUE, DUPLICATE_SAME_ACCESS) &&
        X_DuplicateHandle(hProc, hOutput, hProc, &OutputHandle, 0, TRUE, DUPLICATE_SAME_ACCESS))
    {
        Status = X_CreateHandle(&hConServer, L"\\Device\\ConDrv\\Server", GENERIC_ALL, NULL, TRUE, 0);

        if (Status >= 0)
        {
            PipeAttributes.bInheritHandle = FALSE;
            PipeAttributes.lpSecurityDescriptor = NULL;
            PipeAttributes.nLength = sizeof(SECURITY_ATTRIBUTES);

            if (X_CreatePipe(&ReadPipeHandle, &WritePipeHandle, &PipeAttributes, 0) &&
                X_SetHandleInformation(ReadPipeHandle, HANDLE_FLAG_INHERIT, HANDLE_FLAG_INHERIT))
            {
                PCWSTR InheritCursor = L"--inheritcursor "; // Requires one space
                if (!(dwFlags & PSEUDOCONSOLE_INHERIT_CURSOR))
                    InheritCursor = L"";

                swprintf_s(
                    ConHost,
                    MAX_PATH,
                    L"\\\\?\\%s\\system32\\conhost.exe --headless %s--width %hu --height %hu --signal 0x%x --server 0x%x",
                    RtlGetNtSystemRoot(),
                    InheritCursor,
                    size.X,
                    size.Y,
                    HandleToULong(ReadPipeHandle),
                    HandleToULong(hConServer));

                // Initialize thread attribute list
                HANDLE Values[4] = { hConServer, InputHandle, OutputHandle, ReadPipeHandle };
                X_InitializeProcThreadAttributeList(NULL, 1, 0, &AttrSize);
                AttrList = (LPPROC_THREAD_ATTRIBUTE_LIST)malloc(AttrSize);
                X_InitializeProcThreadAttributeList(AttrList, 1, 0, &AttrSize);
                bRes = X_UpdateProcThreadAttribute(
                    AttrList,
                    0,
                    PROC_THREAD_ATTRIBUTE_HANDLE_LIST, //0x20002u
                    Values,
                    sizeof(Values),
                    NULL,
                    NULL);

                // Assign members of STARTUPINFOEXW
                SInfoEx.StartupInfo.cb = sizeof(STARTUPINFOEXW);
                SInfoEx.StartupInfo.dwFlags = STARTF_USESTDHANDLES;
                SInfoEx.StartupInfo.hStdInput = InputHandle;
                SInfoEx.StartupInfo.hStdOutput = OutputHandle;
                SInfoEx.StartupInfo.hStdError = OutputHandle;
                SInfoEx.lpAttributeList = AttrList;

                bRes = CreateProcessAsUserW(
                    hToken,
                    NULL,
                    ConHost,
                    NULL,
                    NULL,
                    TRUE,
                    EXTENDED_STARTUPINFO_PRESENT,
                    NULL,
                    NULL,
                    &SInfoEx.StartupInfo,
                    &ProcInfo);

                if (bRes)
                {
                    Status = X_CreateHandle(
                        &hConReference,
                        L"\\Reference",
                        GENERIC_READ | GENERIC_WRITE | SYNCHRONIZE,
                        hConServer,
                        FALSE,
                        FILE_SYNCHRONOUS_IO_NONALERT);

                    if (Status >= 0)
                    {
                        hpCon->hWritePipe = WritePipeHandle;
                        hpCon->hConDrvReference = hConReference;
                        hpCon->hConHostProcess = ProcInfo.hProcess;
                    }
                }
                else
                    Log(X_GetLastError(), L"CreateProcessW");
            }
            else
                Log(X_GetLastError(), L"CreatePipe");
        }
        else
            Log(X_GetLastError(), L"CreateHandle");
    }
    else
        Log(X_GetLastError(), L"DuplicateHandle");

    // Cleanup
    free(AttrList);
    NtClose(InputHandle);
    NtClose(OutputHandle);
    NtClose(hConServer);
    //NtClose(ProcInfo.hThread);
    //NtClose(ProcInfo.hProcess);
    //NtClose(ReadPipeHandle);
    //NtClose(WritePipeHandle);

    return S_OK;
}

void X_ClosePseudoConsole(X_HPCON hpCon)
{
    X_TerminateProcess(hpCon.hConHostProcess, 0);
    NtClose(hpCon.hWritePipe);
    NtClose(hpCon.hConDrvReference);
}
