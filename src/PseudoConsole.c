#include <windows.h>
#include <winternl.h>
#include <assert.h>
#include <stdio.h>
#include "PseudoConsole.h"

#define NtCurrentProcess() ((HANDLE)(LONG_PTR)-1)

/* Get SystemRoot folder, imported from ntdll */
PCWSTR NTAPI RtlGetNtSystemRoot(void);

NTSTATUS
WINAPI
CreateHandle(
    PHANDLE FileHandle,
    PWSTR Buffer,
    ACCESS_MASK DesiredAccess,
    HANDLE RootDirectory,
    BOOLEAN IsInheritable,
    ULONG OpenOptions)
{
    IO_STATUS_BLOCK IoStatusBlock;
    ULONG Attributes = OBJ_CASE_INSENSITIVE;
    if (IsInheritable)
        Attributes |= OBJ_INHERIT;

    UNICODE_STRING ObjectName;
    memset(&ObjectName, 0, sizeof ObjectName);
    RtlInitUnicodeString(&ObjectName, Buffer);

    OBJECT_ATTRIBUTES ObjectAttributes;
    memset(&ObjectAttributes, 0, sizeof ObjectAttributes);
    ObjectAttributes.RootDirectory = RootDirectory;
    ObjectAttributes.Length = sizeof ObjectAttributes;
    ObjectAttributes.Attributes = Attributes;
    ObjectAttributes.ObjectName = &ObjectName;

    return NtOpenFile(
        FileHandle,
        DesiredAccess,
        &ObjectAttributes,
        &IoStatusBlock,
        FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
        OpenOptions);
}

HRESULT
WINAPI
CreatePseudoConsole_mod(
    COORD size,
    HANDLE hInput,
    HANDLE hOutput,
    DWORD dwFlags,
    HPCON *phPC)
{
    NTSTATUS Status;
    BOOL bRes;

    HANDLE InputHandle, OutputHandle;
    HANDLE hConServer, hConReference;
    HANDLE hProc = NtCurrentProcess();

    bRes = DuplicateHandle(hProc, hInput, hProc, &InputHandle,
                           0, TRUE, DUPLICATE_SAME_ACCESS);
    assert(bRes != 0);

    bRes = DuplicateHandle(hProc, hOutput, hProc, &OutputHandle,
                           0, TRUE, DUPLICATE_SAME_ACCESS);
    assert(bRes != 0);

    Status = CreateHandle(&hConServer, L"\\Device\\ConDrv\\Server",
                          GENERIC_ALL, NULL, TRUE, 0);
    assert(Status == 0);

    HANDLE ReadPipeHandle, WritePipeHandle;
    SECURITY_ATTRIBUTES PipeAttributes;
    memset(&PipeAttributes, 0, sizeof PipeAttributes);
    PipeAttributes.bInheritHandle = FALSE;
    PipeAttributes.lpSecurityDescriptor = NULL;
    PipeAttributes.nLength = sizeof PipeAttributes;

    bRes = CreatePipe(&ReadPipeHandle, &WritePipeHandle, &PipeAttributes, 0);
    assert(bRes != 0);

    bRes = SetHandleInformation(ReadPipeHandle, HANDLE_FLAG_INHERIT, HANDLE_FLAG_INHERIT);
    assert(bRes != 0);

    PWSTR InheritCursor = L"--inheritcursor "; /* Requires one space */
    if (!(dwFlags & PSEUDOCONSOLE_INHERIT_CURSOR))
        InheritCursor = L"";

    wchar_t ConHostCommand[MAX_PATH];

    /* ConHost command format strings */
#define COMMAND_FORMAT \
    L"\\\\?\\%s\\system32\\conhost.exe %s--width %hu --height %hu --signal 0x%x --server 0x%x"

    swprintf_s(
        ConHostCommand,
        MAX_PATH,
        COMMAND_FORMAT,
        RtlGetNtSystemRoot(),
        InheritCursor,
        size.X,
        size.Y,
        HandleToULong(ReadPipeHandle),
        HandleToULong(hConServer));

    /* Initialize thread attribute list */
    HANDLE Values[4] = { hConServer, InputHandle, OutputHandle, ReadPipeHandle };
    size_t AttrSize;
    LPPROC_THREAD_ATTRIBUTE_LIST AttrList = NULL;
    InitializeProcThreadAttributeList(NULL, 1, 0, &AttrSize);
    AttrList = HeapAlloc(GetProcessHeap(), 0, AttrSize);
    InitializeProcThreadAttributeList(AttrList, 1, 0, &AttrSize);
    bRes = UpdateProcThreadAttribute(AttrList, 0,
                                     PROC_THREAD_ATTRIBUTE_HANDLE_LIST, /* 0x20002u */
                                     Values, sizeof Values, NULL, NULL);
    assert(bRes != 0);

    /* Assign members of STARTUPINFOEXW */
    PROCESS_INFORMATION ProcInfo;
    memset(&ProcInfo, 0, sizeof ProcInfo);
    STARTUPINFOEXW SInfoEx;
    memset(&SInfoEx, 0, sizeof SInfoEx);
    SInfoEx.StartupInfo.cb = sizeof SInfoEx;
    SInfoEx.StartupInfo.dwFlags |= STARTF_USESTDHANDLES;
    SInfoEx.StartupInfo.hStdInput = InputHandle;
    SInfoEx.StartupInfo.hStdOutput = OutputHandle;
    SInfoEx.StartupInfo.hStdError = OutputHandle;
    SInfoEx.lpAttributeList = AttrList;

    bRes = CreateProcessW(
        NULL,
        ConHostCommand,
        NULL,
        NULL,
        TRUE, /* All handles are inheritable by ConHost PTY */
        EXTENDED_STARTUPINFO_PRESENT,
        NULL,
        NULL,
        &SInfoEx.StartupInfo,
        &ProcInfo);
    assert(bRes != 0);

    Status = CreateHandle(
        &hConReference,
        L"\\Reference",
        GENERIC_READ | GENERIC_WRITE | SYNCHRONIZE,
        hConServer,
        FALSE,
        FILE_SYNCHRONOUS_IO_NONALERT);
    assert(Status == 0);

    /* Return handles to caller */
    HPCON_INTERNAL * hpCon = (HPCON_INTERNAL *)HeapAlloc(GetProcessHeap(), 0, sizeof(*hpCon));
    hpCon->hWritePipe = WritePipeHandle;
    hpCon->hConDrvReference = hConReference;
    hpCon->hConHostProcess = ProcInfo.hProcess;

    /* Dereference HPCON pointer to HPCON_INTERNAL structure pointer */
    *phPC = hpCon;

    /* Cleanup */
    HeapFree(GetProcessHeap(), 0, AttrList);
    CloseHandle(InputHandle);
    CloseHandle(OutputHandle);
    CloseHandle(hConServer);
    CloseHandle(ProcInfo.hThread);
    CloseHandle(ProcInfo.hProcess);

    return 0;
}

HRESULT WINAPI ResizePseudoConsole_mod(HPCON hPC, COORD size)
{
    BOOL bRes;

    RESIZE_PSEUDO_CONSOLE_BUFFER ResizeBuffer;
    memset(&ResizeBuffer, 0, sizeof ResizeBuffer);
    ResizeBuffer.Flags = RESIZE_CONHOST_SIGNAL_BUFFER;
    ResizeBuffer.SizeX = size.X;
    ResizeBuffer.SizeY = size.Y;

    HPCON_INTERNAL *hpCon = (HPCON_INTERNAL *)hPC;

    bRes = WriteFile(
        hpCon->hWritePipe,
        &ResizeBuffer,
        sizeof ResizeBuffer,
        NULL,
        NULL);
    assert(bRes != 0);
    return 0;
}

void WINAPI ClosePseudoConsole_mod(HPCON hPC)
{
    DWORD ExitCode;

    HPCON_INTERNAL *hpCon = (HPCON_INTERNAL *)hPC;

    CloseHandle(hpCon->hWritePipe);

    if (GetExitCodeProcess(hpCon->hConHostProcess, &ExitCode)
        && ExitCode == STILL_ACTIVE)
    {
        WaitForSingleObject(hpCon->hConHostProcess, INFINITE);
    }

    TerminateProcess(hpCon->hConHostProcess, 0);

    CloseHandle(hpCon->hConDrvReference);

    HeapFree(GetProcessHeap(), 0, hpCon);
}
