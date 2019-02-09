#include "WinInternal.h"
#include "KernelBase.h"
#include "PseudoConsole.h"
#include <stdio.h>

#ifndef PSEUDOCONSOLE_INHERIT_CURSOR
#define PSEUDOCONSOLE_INHERIT_CURSOR 1
#endif

HRESULT
WINAPI
X_CreatePseudoConsoleAsUser(HANDLE TokenHandle,
                            COORD ConsoleSize,
                            HANDLE hInput,
                            HANDLE hOutput,
                            DWORD dwFlags,
                            PX_HPCON hpCon)
{
    NTSTATUS Status;
    BOOL bRes;
    ULONG LastError = X_GetLastError();

    HANDLE InputHandle = NULL, OutputHandle = NULL;
    HANDLE hConServer = NULL, hConReference;
    HANDLE hProc = NtCurrentProcess();
    HANDLE ReadPipeHandle = NULL, WritePipeHandle = NULL;
    HANDLE HeapHandle = X_GetProcessHeap();

    wchar_t ConHostCommand[MAX_PATH];
    size_t AttrSize;
    LPPROC_THREAD_ATTRIBUTE_LIST AttrList = NULL;
    STARTUPINFOEXW SInfoEx = { 0 };
    PROCESS_INFORMATION ProcInfo = { 0 };

    if (X_DuplicateHandle(hProc, hInput, hProc, &InputHandle, 0, TRUE, DUPLICATE_SAME_ACCESS) &&
        X_DuplicateHandle(hProc, hOutput, hProc, &OutputHandle, 0, TRUE, DUPLICATE_SAME_ACCESS))
    {
        Status = X_CreateHandle(&hConServer,
                                L"\\Device\\ConDrv\\Server",
                                GENERIC_ALL,
                                NULL,
                                TRUE,
                                0);

        if (NT_SUCCESS(Status))
        {
            SECURITY_ATTRIBUTES PipeAttributes = { 0 };
            PipeAttributes.bInheritHandle = FALSE;
            PipeAttributes.lpSecurityDescriptor = NULL;
            PipeAttributes.nLength = sizeof PipeAttributes;

            if (X_CreatePipe(&ReadPipeHandle, &WritePipeHandle, &PipeAttributes, 0) &&
                X_SetHandleInformation(ReadPipeHandle, HANDLE_FLAG_INHERIT, HANDLE_FLAG_INHERIT))
            {
                PWSTR InheritCursor = L"--inheritcursor "; // Requires one space
                if (!(dwFlags & PSEUDOCONSOLE_INHERIT_CURSOR))
                    InheritCursor = L"";

#ifdef FUN_MODE
                UNREFERENCED_PARAMETER(ConsoleSize);
                PCWSTR Format = L"\\\\?\\%s\\system32\\conhost.exe %s--signal 0x%x --server 0x%x";
                _snwprintf_s(ConHostCommand,
                             MAX_PATH,
                             MAX_PATH,
                             Format,
                             RtlGetNtSystemRoot(),
                             InheritCursor,
                             ToULong(ReadPipeHandle),
                             ToULong(hConServer));
#else
                PCWSTR Format = L"\\\\?\\%s\\system32\\conhost.exe --headless %s--width %hu --height %hu --signal 0x%x --server 0x%x";
                _snwprintf_s(ConHostCommand,
                             MAX_PATH,
                             MAX_PATH,
                             Format,
                             RtlGetNtSystemRoot(),
                             InheritCursor,
                             ConsoleSize.X,
                             ConsoleSize.Y,
                             ToULong(ReadPipeHandle),
                             ToULong(hConServer));
#endif // FUN_MODE

                // Initialize thread attribute list
                HANDLE Values[4] = { NULL };
                Values[0] = hConServer;
                Values[1] = InputHandle;
                Values[2] = OutputHandle;
                Values[3] = ReadPipeHandle;

                X_InitializeProcThreadAttributeList(NULL, 1, 0, &AttrSize);
                AttrList = RtlAllocateHeap(HeapHandle, HEAP_ZERO_MEMORY, AttrSize);
                X_InitializeProcThreadAttributeList(AttrList, 1, 0, &AttrSize);
                bRes = X_UpdateProcThreadAttribute(AttrList,
                                                        0,
                                                        PROC_THREAD_ATTRIBUTE_HANDLE_LIST, //0x20002u
                                                        Values,
                                                        sizeof Values,
                                                        NULL,
                                                        NULL);

                // Assign members of STARTUPINFOEXW
                SInfoEx.StartupInfo.cb = sizeof SInfoEx;
                SInfoEx.StartupInfo.dwFlags |= STARTF_USESTDHANDLES;
                SInfoEx.StartupInfo.hStdInput = InputHandle;
                SInfoEx.StartupInfo.hStdOutput = OutputHandle;
                SInfoEx.StartupInfo.hStdError = OutputHandle;
                SInfoEx.lpAttributeList = AttrList;

                bRes = CreateProcessAsUserW(TokenHandle,
                                            NULL,
                                            ConHostCommand,
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
                    Status = X_CreateHandle(&hConReference,
                                            L"\\Reference",
                                            GENERIC_READ | GENERIC_WRITE | SYNCHRONIZE,
                                            hConServer,
                                            FALSE,
                                            FILE_SYNCHRONOUS_IO_NONALERT);

                    if (NT_SUCCESS(Status))
                    {
                        hpCon->hWritePipe = WritePipeHandle;
                        hpCon->hConDrvReference = hConReference;
                        hpCon->hConHostProcess = ProcInfo.hProcess;
                    }
                }
                else
                    LogResult(LastError, L"CreateProcessAsUserW");
            }
            else
                LogResult(LastError, L"CreatePipe");
        }
        else
            LogResult(LastError, L"CreateHandle");
    }
    else
        LogResult(LastError, L"DuplicateHandle");

    // Cleanup
    RtlFreeHeap(HeapHandle, 0, AttrList);
    NtClose(InputHandle);
    NtClose(OutputHandle);
    NtClose(hConServer);
    NtClose(ProcInfo.hThread);
    NtClose(ProcInfo.hProcess);
    // NtClose(ReadPipeHandle);
    // NtClose(WritePipeHandle);

    return S_OK;
}

HRESULT
WINAPI
X_CreatePseudoConsole(COORD ConsoleSize,
                      HANDLE hInput,
                      HANDLE hOutput,
                      DWORD dwFlags,
                      PX_HPCON hpCon)
{
    HANDLE TokenHandle = NULL;
    return X_CreatePseudoConsoleAsUser(TokenHandle,
                                       ConsoleSize,
                                       hInput,
                                       hOutput,
                                       dwFlags,
                                       hpCon);
}

HRESULT 
WINAPI
X_ResizePseudoConsole(PX_HPCON hPC,
                      COORD ConsoleSize)
{
    HRESULT hRes = 0;
    NTSTATUS Status;
    IO_STATUS_BLOCK IoStatusBlock;

    RESIZE_PSEUDO_CONSOLE_BUFFER ResizeBuffer = { 0 };
    ResizeBuffer.Flags = RESIZE_CONHOST_SIGNAL_BUFFER;
    ResizeBuffer.SizeX = ConsoleSize.X;
    ResizeBuffer.SizeY = ConsoleSize.Y;

    Status = NtWriteFile(hPC->hWritePipe,
                         NULL,
                         NULL,
                         NULL,
                         &IoStatusBlock,
                         &ResizeBuffer,
                         sizeof ResizeBuffer,
                         NULL,
                         NULL);

    if (!NT_SUCCESS(Status))
    {
        LogStatus(Status, L"NtWriteFile");
        DWORD ErrCode = X_GetLastError();
        if (ErrCode > 0)
            hRes = ErrCode | 0x80070000;
        else
            hRes = ErrCode;
    }
    return hRes;
}

VOID
WINAPI
X_ClosePseudoConsole(PX_HPCON hpCon)
{
    X_TerminateProcess(hpCon->hConHostProcess, 0);
    NtClose(hpCon->hWritePipe);
    NtClose(hpCon->hConDrvReference);
}
