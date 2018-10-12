#include "ntdll.h"
#include <stdio.h>

void Log(ULONG Result, PWSTR Function)
{
    PWSTR MsgBuffer = NULL;
    FormatMessageW(
        (FORMAT_MESSAGE_ALLOCATE_BUFFER |
            FORMAT_MESSAGE_FROM_SYSTEM |
            FORMAT_MESSAGE_IGNORE_INSERTS),
        NULL, Result, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (PWSTR)&MsgBuffer, 0, NULL);
    wprintf(L"%ls Error: %ld\n%ls\n", Function, (Result & 0xFFFF), MsgBuffer);
    LocalFree(MsgBuffer);
}

HANDLE X_GetCurrentProcess(void)
{
    return (HANDLE)-1;
}

DWORD X_GetLastError(void)
{
    return (DWORD)NtCurrentTeb()->Reserved2[0];
}

ULONG BaseSetLastNTError(NTSTATUS Status)
{
    ULONG Result;

    Result = RtlNtStatusToDosError(Status);
    RtlSetLastWin32Error(Result);
    return Result;
}

BOOL X_InitializeProcThreadAttributeList(
    LPPROC_THREAD_ATTRIBUTE_LIST lpAttributeList,
    DWORD dwAttributeCount,
    DWORD dwFlags,
    PSIZE_T lpSize)
{
    if (dwFlags) {
        // STATUS_INVALID_PARAMETER_3
    }
    if (dwAttributeCount > sizeof(PROC_THREAD_ATTRIBUTE_ENTRY))
    {
        // STATUS_INVALID_PARAMETER_2
    }

    BOOL Result = TRUE;
    SIZE_T Size = sizeof(PROC_THREAD_ATTRIBUTE_ENTRY) * (dwAttributeCount + 1);
    if (lpAttributeList && *lpSize >= Size)
    {
        lpAttributeList->dwFlags = 0;
        lpAttributeList->Unknown = 0;
        lpAttributeList->Size = dwAttributeCount;
        lpAttributeList->Count = 0;
    }
    else
    {
        Result = FALSE;
        RtlSetLastWin32Error(ERROR_INSUFFICIENT_BUFFER);
    }

    *lpSize = Size;
    return Result;
}

BOOL X_UpdateProcThreadAttribute(
    LPPROC_THREAD_ATTRIBUTE_LIST lpAttributeList,
    DWORD dwFlags,
    DWORD_PTR Attribute,
    PVOID lpValue,
    SIZE_T cbSize,
    PVOID lpPreviousValue,
    PSIZE_T lpReturnSize)
{
    DWORD Flags = 1 << Attribute;

    if (!(Attribute & PROC_THREAD_ATTRIBUTE_INPUT)) {
        if (lpAttributeList->Count == lpAttributeList->Size) {
            // STATUS_UNSUCCESSFUL
        }
        if (Flags & lpAttributeList->dwFlags) {
            // STATUS_OBJECT_NAME_EXISTS
        }
        if (lpPreviousValue) {
            // STATUS_INVALID_PARAMETER_6
        }
        if (dwFlags & 1) {
            // STATUS_INVALID_PARAMETER_2
        }
    }

    if (Attribute & (PROC_THREAD_ATTRIBUTE_THREAD | PROC_THREAD_ATTRIBUTE_INPUT)
        && lpReturnSize) {
        // STATUS_INVALID_PARAMETER_7
    }

    if (Flags)
    {
        ++lpAttributeList->Count;
        lpAttributeList->dwFlags |= Flags;
        lpAttributeList->Entries[0].Attribute = Attribute;
        lpAttributeList->Entries[0].cbSize = cbSize;
        lpAttributeList->Entries[0].lpValue = lpValue;
        // lpAttributeList->Size previously set
        // lpAttributeList->Unknown unchanged
    }

    return TRUE;
}

BOOL X_CreatePipe(
    PHANDLE hReadPipe,
    PHANDLE hWritePipe,
    PSECURITY_ATTRIBUTES lpPipeAttributes,
    DWORD nSize)
{
    UNICODE_STRING PipeName = { 0 };
    OBJECT_ATTRIBUTES ObjectAttributes = { 0 };
    IO_STATUS_BLOCK IoStatusBlock;

    PVOID lpSecurityDescriptor = NULL;
    ULONG Attributes = OBJ_CASE_INSENSITIVE; // Always case insensitive
    HANDLE DeviceHandle, ReadPipeHandle, WritePipeHandle;
    NTSTATUS Status;
    LARGE_INTEGER DefaultTimeOut;
    DefaultTimeOut.QuadPart = -1200000000LL;
    DWORD Size = 0x1000uL;
    if (nSize)
        Size = nSize;

    // Open NamedPipe device
    wchar_t DeviceName[] = L"\\Device\\NamedPipe\\";
    PipeName.Buffer = DeviceName;
    PipeName.Length = (USHORT)(sizeof(DeviceName) - sizeof(wchar_t));
    ObjectAttributes.Length = sizeof(OBJECT_ATTRIBUTES);
    ObjectAttributes.ObjectName = &PipeName;

    Status = NtOpenFile(
        &DeviceHandle,
        GENERIC_READ | SYNCHRONIZE,
        &ObjectAttributes,
        &IoStatusBlock,
        FILE_SHARE_READ | FILE_SHARE_WRITE,
        FILE_SYNCHRONOUS_IO_NONALERT);
    if (Status < 0)
    {
        Log(Status, L"NtOpenFile");
        BaseSetLastNTError(Status);
        return FALSE;
    }

    PipeName.Buffer = NULL;
    PipeName.Length = 0;

    // Set security Attributes
    if (lpPipeAttributes)
    {
        lpSecurityDescriptor = lpPipeAttributes->lpSecurityDescriptor;
        if (lpPipeAttributes->bInheritHandle)
            Attributes |= OBJ_INHERIT;
    }

    // Create Named Pipe
    ObjectAttributes.RootDirectory = DeviceHandle;
    ObjectAttributes.Length = sizeof(OBJECT_ATTRIBUTES);
    ObjectAttributes.ObjectName = &PipeName;
    ObjectAttributes.Attributes = Attributes;
    ObjectAttributes.SecurityDescriptor = lpSecurityDescriptor;
    ObjectAttributes.SecurityQualityOfService = NULL;

    Status = NtCreateNamedPipeFile(
        &ReadPipeHandle,
        GENERIC_READ | SYNCHRONIZE | FILE_WRITE_ATTRIBUTES,
        &ObjectAttributes,
        &IoStatusBlock,
        FILE_SHARE_READ | FILE_SHARE_WRITE,
        FILE_CREATE,
        FILE_SYNCHRONOUS_IO_NONALERT,
        FILE_PIPE_BYTE_STREAM_TYPE,
        FILE_PIPE_BYTE_STREAM_MODE,
        FILE_PIPE_QUEUE_OPERATION,
        1,
        Size,
        Size,
        &DefaultTimeOut);
    if (Status < 0)
    {
        Log(Status, L"NtCreateNamedPipeFile");
        BaseSetLastNTError(Status);
        return FALSE;
    }

    // Open write handle for the pipe
    ObjectAttributes.RootDirectory = ReadPipeHandle;
    ObjectAttributes.ObjectName = &PipeName;
    ObjectAttributes.Length = sizeof(OBJECT_ATTRIBUTES);
    ObjectAttributes.Attributes = Attributes;
    ObjectAttributes.SecurityDescriptor = lpSecurityDescriptor;
    ObjectAttributes.SecurityQualityOfService = NULL;

    Status = NtOpenFile(
        &WritePipeHandle,
        GENERIC_WRITE | SYNCHRONIZE | FILE_READ_ATTRIBUTES,
        &ObjectAttributes,
        &IoStatusBlock,
        FILE_SHARE_READ | FILE_SHARE_WRITE,
        FILE_SYNCHRONOUS_IO_NONALERT | FILE_NON_DIRECTORY_FILE);
    if (Status < 0)
    {
        Log(Status, L"NtOpenFile");
        BaseSetLastNTError(Status);
        return FALSE;
    }

    // Return pipe handles
    *hReadPipe = ReadPipeHandle;
    *hWritePipe = WritePipeHandle;
    return TRUE;
}

NTSTATUS X_CreateHandle(
    PHANDLE FileHandle,
    PWSTR Buffer,
    ACCESS_MASK DesiredAccess,
    HANDLE RootDirectory,
    BOOLEAN IsInheritable,
    ULONG OpenOptions)
{
    UNICODE_STRING ObjectName = { 0 };
    OBJECT_ATTRIBUTES ObjectAttributes = { 0 };
    IO_STATUS_BLOCK IoStatusBlock;
    ULONG Attributes = OBJ_CASE_INSENSITIVE;

    ObjectName.Buffer = Buffer;
    ObjectName.Length = (USHORT)(sizeof(wchar_t) * wcslen(Buffer));
    ObjectName.MaximumLength = (USHORT)(sizeof(wchar_t) * (wcslen(Buffer) + 1));

    ObjectAttributes.RootDirectory = RootDirectory;
    ObjectAttributes.Length = sizeof(OBJECT_ATTRIBUTES);
    if (IsInheritable)
        Attributes |= OBJ_INHERIT;
    ObjectAttributes.Attributes = Attributes;
    ObjectAttributes.ObjectName = &ObjectName;
    ObjectAttributes.SecurityDescriptor = NULL;
    return NtOpenFile(
        FileHandle,
        DesiredAccess,
        &ObjectAttributes,
        &IoStatusBlock,
        FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
        OpenOptions);
}

BOOL X_DuplicateHandle(
    HANDLE hSourceProcessHandle,
    HANDLE hSourceHandle,
    HANDLE hTargetProcessHandle,
    LPHANDLE lpTargetHandle,
    DWORD dwDesiredAccess,
    BOOL bInheritHandle,
    DWORD dwOptions)
{
    NTSTATUS Status;

    switch (HandleToULong(hSourceHandle))
    {
    case STD_ERROR_HANDLE:
        hSourceHandle = NtCurrentTeb()->ProcessEnvironmentBlock->
            ProcessParameters->Reserved2[4];
        break;
    case STD_OUTPUT_HANDLE:
        hSourceHandle = NtCurrentTeb()->ProcessEnvironmentBlock->
            ProcessParameters->Reserved2[3];
        break;
    case STD_INPUT_HANDLE:
        hSourceHandle = NtCurrentTeb()->ProcessEnvironmentBlock->
            ProcessParameters->Reserved2[2];
        break;
    }

    Status = NtDuplicateObject(
        hSourceProcessHandle,
        hSourceHandle,
        hTargetProcessHandle,
        lpTargetHandle,
        dwDesiredAccess,
        bInheritHandle != 0 ? OBJ_INHERIT : 0,
        dwOptions);
    if (Status < 0)
    {
        Log(Status, L"NtDuplicateObject");
        BaseSetLastNTError(Status);
        return FALSE;
    }
    return TRUE;
}

HANDLE X_GetStdHandle(DWORD nStdHandle)
{
    HANDLE hSourceHandle;

    switch (nStdHandle)
    {
    case STD_ERROR_HANDLE:
        hSourceHandle = NtCurrentTeb()->ProcessEnvironmentBlock->
            ProcessParameters->Reserved2[4];
        break;
    case STD_OUTPUT_HANDLE:
        hSourceHandle = NtCurrentTeb()->ProcessEnvironmentBlock->
            ProcessParameters->Reserved2[3];
        break;
    case STD_INPUT_HANDLE:
        hSourceHandle = NtCurrentTeb()->ProcessEnvironmentBlock->
            ProcessParameters->Reserved2[2];
        break;
    default:
        hSourceHandle = (HANDLE)-1;
    }
    if (hSourceHandle == (HANDLE)-1)
        BaseSetLastNTError(STATUS_INVALID_HANDLE);
    return hSourceHandle;
}

BOOL X_SetHandleInformation(
    HANDLE hObject,
    DWORD dwMask,
    DWORD dwFlags)
{
    OBJECT_HANDLE_FLAG_INFORMATION ObjectInformation;
    NTSTATUS Status;

    switch (HandleToULong(hObject))
    {
    case STD_ERROR_HANDLE:
        hObject = NtCurrentTeb()->ProcessEnvironmentBlock->
            ProcessParameters->Reserved2[4];
        break;
    case STD_OUTPUT_HANDLE:
        hObject = NtCurrentTeb()->ProcessEnvironmentBlock->
            ProcessParameters->Reserved2[3];
        break;
    case STD_INPUT_HANDLE:
        hObject = NtCurrentTeb()->ProcessEnvironmentBlock->
            ProcessParameters->Reserved2[2];
        break;
    }
    
    Status = NtQueryObject(
        hObject,
        (OBJECT_INFORMATION_CLASS)4, //ObjectHandleFlagInformation
        &ObjectInformation,
        sizeof(OBJECT_HANDLE_FLAG_INFORMATION),
        NULL);
    if (Status < 0)
    {
        Log(Status, L"NtQueryObject");
        BaseSetLastNTError(Status);
        return FALSE;
    }

    if (dwMask & HANDLE_FLAG_INHERIT)
        ObjectInformation.Inherit = dwFlags & 1;
    if (dwMask & HANDLE_FLAG_PROTECT_FROM_CLOSE)
        ObjectInformation.ProtectFromClose = (dwFlags >> 1) & 1;

    Status = NtSetInformationObject(
        hObject,
        (OBJECT_INFORMATION_CLASS)4,
        &ObjectInformation,
        sizeof(OBJECT_HANDLE_FLAG_INFORMATION));
    if (Status < 0)
    {
        Log(Status, L"NtSetInformationObject");
        BaseSetLastNTError(Status);
        return FALSE;
    }

    return TRUE;
}

BOOL X_TerminateProcess(HANDLE hProcess, UINT uExitCode)
{
    if (!hProcess)
    {
        RtlSetLastWin32Error(ERROR_INVALID_HANDLE);
        return FALSE;
    }

    // RtlReportSilentProcessExit(); goes to SendMessageToWERService();
    // which opens WerFault.exe aka. User Data Collector

    NTSTATUS Status = NtTerminateProcess(
        hProcess,
        uExitCode);

    if (Status < 0)
    {
        BaseSetLastNTError(Status);
        return FALSE;
    }
    return TRUE;
}
